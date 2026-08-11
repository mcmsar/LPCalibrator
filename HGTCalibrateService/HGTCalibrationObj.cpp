/*********************************************************************
*	              Copyright (c) 2016 by EMS Technologies, Inc.,
*										All rights reserved
*	This program is unpublished software and contains the trade secrets
*	and confidential information of EMS Technologies, Inc.  It may not be 
* reproduced in whole or in part, in any form or by any means whatsoever 
* without the express written permission of EMS Technologies, Inc.
*
********************************************************************/

/*******************************************************************

  Revision Record

	$Log:
	$
********************************************************************/

//#include "HGTCalibrationObj.h"
#include "HGTCalibrationObjectsContainer.h"
#include "ccbootstrap.h"
#include "calib406msg.h"
#include "satelliteproperties.h"			// CEMSSatelliteProperties
#include "lutreceiverproperties.h"			// CEMSLutReceiverProperties
#include "emsrange.h"						// CEMSRangeRate
#include "EMSBeacon.h"
#include "CBeaconMessage.h"
#include "CBcnMsgDecodeBase.h"
#include "LogHelper.h"
#include "logmsgparam.h"

#define INITGUID
#include <initguid.h>
#include "ConfigManagerProxyClsid.h"

#include <iostream>
#include "convutility.h"
#include "HGTFileLogger.h"
#include "TSiDebugTrace.h"
#include "DBFPlateConfig.h"

//#include <math.h>
//#include "emsvectr.h"

// Moving Beacon
#include "emsvectr.h"

const double c_dDefaultCNRFilterThreshold = 35.0;
const double c_dDefaultAlarmThreshold = 10.0;
const int    c_iDefaultAlarmCount = 2;

const double c_dOldOrbitInDays = 2.0; //In days
const double c_dSatPositionDiffInKm = 10.0; //In Kilometer
const double c_dSatVelDiffInKmPerSec = 1e-2; //In Kilometer/sec

const ULONG	 c_ulConvergenceTestCount = 10;

//For DBF
//const double c_dFreqDBFoffset  = 11177.0 ;
const double c_dFreqDBFoffset  = 0.0 ;
const double c_dMinFoaResidual = 200.0;
//const double c_dMinFoaResidual = 5.0;
// Restored from 0.0 (set 16th Nov 2021) - near-horizon candidates are exactly
// where neighboring orbital planes' Doppler curves are most likely to cross,
// which is when _GetDBFSatellite is most prone to picking the wrong
// satellite (see c_dFoaAmbiguityMarginHz below).
const double c_dMinElevation = 10.0;
const double c_dSAthreshold  = 7.0; // Minimum allowed satellite separation angle

// _GetDBFSatellite picks whichever candidate satellite has the smallest FOA
// residual out of a ~36-satellite sweep, with no check on how much better it
// is than the runner-up. When two candidates are within this margin of each
// other, treat the pick as ambiguous rather than trusting whichever one is
// fractionally smaller on this one noisy sample - real captured 200kHz/1s
// buffer data (satellites 405/418 on 2026-07-27, 409/418 on 2026-07-28) show
// the same beacon's detections genuinely flipping between two satellite IDs,
// including several at the exact same timestamp. Starting value is a guess;
// tune from the "AMBIGUOUS"/"HYSTERESIS" trace lines this produces.
const double c_dFoaAmbiguityMarginHz = 20.0;

// A candidate satellite must get the arrival TIME this close before it is
// allowed to compete on frequency residual. Tightened from 0.02 (which the
// commented-out 0.005 above it suggests was already suspected of being loose).
//
// 0.02 s is ~6000 km of range error - so wide it discriminates nothing. In the
// 2026-08-05 capture, satellite 433 was selected for two populations that split
// cleanly by beacon: the correctly-identified one had a median |TOA residual|
// of 0.18 ms, the misidentified one 8.8 ms - a 50x separation that 0.02 s let
// straight through, admitting detections with 6.5 kHz frequency residuals.
// 0.002 s (~600 km) sits in the gap between the two populations.
const double c_dMaxToaResidualSec = 0.002;

// Above this, a reference-beacon FOA residual is treated as evidence that the
// detection was attributed to the wrong satellite, and it is excluded from the
// bias update entirely rather than clamped into it. See _CalculateOffsets.
// Sits well above the largest genuine bias error observed (the ~1478 Hz
// accumulated in lscalibdata.csv was itself the product of a broken loop, and
// real per-detection residuals now run at ~0.3 Hz) and well below the 4-10 kHz
// misidentification population.
const double c_dMaxBiasUpdateResidualHz = 1000.0;

CHGTCalibrationObj::CHGTCalibrationObj() :  CApiObjBase( TEXT("CHGTCalibrationObj") ),m_lpOrbit(NULL)
											
{
	m_ulSARRNumber = 0;
	m_bSpSarrUpdated = false;
	m_bToaFoaSarrUpdated = false;
	memset( m_aDataSourceStatus, 0, sizeof(m_aDataSourceStatus) );
	memset( &m_lsCalibData, 0, sizeof( HGTLSCALIBDATA ) );
	


	_CreateObjects();

	//For DBF
	memset(&m_ulArrBeamId, 0, sizeof(m_ulArrBeamId));
	//m_SAtimeLast.intTime = 0;
}

CHGTCalibrationObj::CHGTCalibrationObj( CHGTCalibrationObj& Obj) : CApiObjBase( TEXT("CHGTCalibrationObj") ),m_lpOrbit(NULL)
{
	m_ulSARRNumber = Obj.m_ulSARRNumber;
	m_bSpSarrUpdated = Obj.m_bSpSarrUpdated;
	m_bToaFoaSarrUpdated = Obj.m_bToaFoaSarrUpdated;
	memset( m_aDataSourceStatus, 0, sizeof(m_aDataSourceStatus) );
	memset( &m_lsCalibData, 0, sizeof( HGTLSCALIBDATA ) );
	memcpy( &m_lsCalibData, Obj.GetLsCalibData(), sizeof( HGTLSCALIBDATA ) );

	_CreateObjects();

	//For DBF
	memset(&m_ulArrBeamId, 0, sizeof(m_ulArrBeamId));
	//m_SAtimeLast.intTime = 0;
}

CHGTCalibrationObj::CHGTCalibrationObj( const HGTLSCALIBDATA& lsCalibData, IEMSOrbit4 *lpOrbit): CApiObjBase( TEXT("CHGTCalibrationObj") ),m_lpOrbit(NULL)
{
	m_ulSARRNumber = 0;
	m_bSpSarrUpdated = false;
	m_bToaFoaSarrUpdated = false;
	memset( m_aDataSourceStatus, 0, sizeof(m_aDataSourceStatus) );
	memcpy(&m_lsCalibData, &lsCalibData, sizeof(HGTLSCALIBDATA));

	_CreateObjects();

	//For DBF
	memset(&m_ulArrBeamId, 0, sizeof(m_ulArrBeamId));
	//m_SAtimeLast.intTime = 0;

	if(m_lpOrbit)
	{
		m_lpOrbit->Release();
		m_lpOrbit = NULL;
	}

	m_lpOrbit = lpOrbit;
}

CHGTCalibrationObj::~CHGTCalibrationObj()
{
	_ReleaseObjects();
}

void CHGTCalibrationObj::_ReleaseObjects()
{

	//if ( m_lpOrbit )
	//{
	//	m_lpOrbit->Release();
	//	m_lpOrbit = 0;
	//}
}

//EMS_RESULT
//CHGTCalibrationObj::_InitOrbitObj()
//{
//	EMS_RESULT hr = EMS_OK;
//
//	hr = CoCreateInstance( CLSID_EMSOrbit, 0, CLSCTX_ALL, IID_IEMSOrbit4, 
//							(LPVOID*)&m_lpOrbit );
//
//	if ( hr != EMS_OK )
//	  	throw CEMSException( EMS_CALIB406_ORBIT_ERROR, hr );
//	
//	return hr;
//}

EMS_RESULT
CHGTCalibrationObj::_CreateObjects( void )
{
	EMS_RESULT hr = EMS_OK;

	//hr = _InitOrbitObj();
	
	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::Init()
{
	EMS_RESULT hr = EMS_OK;

	return hr;
}


EMS_RESULT 
CHGTCalibrationObj::_SetInitialFlags( CEMSRawLpCalibObj *pRawLpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj )
	{

		DWORD dwFlags = 0;

		//Update process flag - 3rd June 2021
		dwFlags |= EMS_C406PF_PROCESS_MASK;

		// only care if they are SARR Records
		//
		if ( (EMSCALIBTYPE_406_SARR == pRawLpCalibObj->GetType()) ||
			 (INTERFERER_TYPE == pRawLpCalibObj->GetType()) ||
			 (MISCODED_TYPE == pRawLpCalibObj->GetType()) ) 
		{
			DWORD ulSatID = pRawLpCalibObj->GetSatId();
			DWORD ulLutID = pRawLpCalibObj->GetLutId();


//			if ( EMS_C406PF_LUT_FIXED_BITS == (EMS_C406PF_LUT_FIXED_BITS &
//				pRawLpCalibObj->GetProcessFlags() ) )
//			{
//				dwFlags |= pRawLpCalibObj->GetProcessFlags();
//			}
//			else
			{
				dwFlags |= EMS_C406PF_SAT_REMOVE_DLINKTIME_FLAG;

				if ( CEMSSatelliteProperties::RequiresDDCorrection( ulSatID ) )
				{
					dwFlags |= EMS_C406PF_SAT_REMOVE_DLINKFREQ_FLAG;
				}

				if ( CEMSSatelliteProperties::RequiresSpectrumInversion( ulSatID ) )
				{
					dwFlags |= EMS_C406PF_SAT_INVERTED_SPECTRUM_FLAG;
				}


				if ( CEMSLutReceiverProperties::RequiresSpectrumInversion( ulLutID ) )
				{
					dwFlags |= EMS_C406PF_LUT_INVERTED_SPECTRUM_FLAG;
				}
			
			}

			if ( lpRefBeaconData != NULL )
			{
				EMSLOCATION locnBeacon;
				CEMSCoordSystem	CS;

				dwFlags |= EMS_C406PF_REF_BEACON_FLAG;
			
				locnBeacon.degLatitude = lpRefBeaconData->data.dLatitude;

				locnBeacon.degLongitude = lpRefBeaconData->data.dLongitude;

				locnBeacon.fAltitude = (float)lpRefBeaconData->data.dAltitude;

				pRawLpCalibObj->SetBcnVector( CS.ToEarthFixedDouble( locnBeacon ) ) ;

				pRawLpCalibObj->SetTxFrequency( lpRefBeaconData->data.dFrequency);
				
				if ( EMS_BEACON_TCAL & lpRefBeaconData->data.uFlag )
				{
					dwFlags |= EMS_C406PF_REF_TIME_FLAG;
				}

				if ( EMS_BEACON_USEREFFREQ  & lpRefBeaconData->data.uFlag )
				{
			 		 dwFlags |= EMS_C406PF_REF_FREQ_FLAG;
				}

				// Gated on lpRefBeaconData != NULL (ref-beacon hits only) to keep volume low.
				// Pins down whether the beacon record's uFlag actually carries the TCAL/
				// USEREFFREQ bits that _ComputeResidual requires before it will call
				// _CalculateOffsets (the function that updates FOA/TOA bias).
				CTSiDebugTrace::LogFmtAlways(
					"_SetInitialFlags: BcnId=%016I64X uFlag=0x%08lX TCAL=%s USEREFFREQ=%s -> dwFlags=0x%08lX REF_TIME=%s REF_FREQ=%s",
					pRawLpCalibObj->GetBcnId(),
					(unsigned long)lpRefBeaconData->data.uFlag,
					(EMS_BEACON_TCAL & lpRefBeaconData->data.uFlag) ? "YES" : "NO",
					(EMS_BEACON_USEREFFREQ & lpRefBeaconData->data.uFlag) ? "YES" : "NO",
					(unsigned long)dwFlags,
					(EMS_C406PF_REF_TIME_FLAG & dwFlags) ? "YES" : "NO",
					(EMS_C406PF_REF_FREQ_FLAG & dwFlags) ? "YES" : "NO");
			}
		}

		pRawLpCalibObj->SetProcessFlags(dwFlags);
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}


CEMSRawLpCalibObj* 
CHGTCalibrationObj::_CreateRawLPCalibObject(CEMSRawSpCalibObj*  pRawSpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData)
{
	EMS_RESULT hr = EMS_OK;

	if(!pRawSpCalibObj)
		return NULL;

	CEMSRawLpCalibObj *pRawLpCalibObj = new CEMSRawLpCalibObj();

	pRawLpCalibObj->SetRawSpCalib(*pRawSpCalibObj->GetRawData());
	pRawLpCalibObj->ResetStatusFlags();
	pRawLpCalibObj->UpdatePassId(); // for traceability

	_SetInitialFlags( pRawLpCalibObj, lpRefBeaconData);

	return pRawLpCalibObj;
}
		
EMS_RESULT
CHGTCalibrationObj::_OrbitUpdate(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;

	if(pRawLpCalibObj)
	{
		if(m_lpOrbit)
		{
			EMSTIMECOORDD timeCoord;
			EMSTLEDATA tleData1;
			ULONG ulCount = 0;
			ULONG ulOrbitNumber = 0;
			double dTimeDiff1 = 0.0;

			memset( &timeCoord, 0, sizeof( timeCoord ) );
			EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();
			EMSTIME timeMsg1 = pRawLpCalibObj->GetTimeMsg();
			ULONG ulSatId = pRawLpCalibObj->GetSatId();

			// Get the current orbit vector
			hr = m_lpOrbit->GetSatelliteTLE( ulSatId, timeMsg, &tleData1 );

			if ( EMS_OK == hr )
			{
				dTimeDiff1 = (tleData1.timeEpoch.intTime - timeMsg.intTime)*1e-9;
				dTimeDiff1 = abs(dTimeDiff1/86400.0);  //in Days
				pRawLpCalibObj->SetReserve0(DWORD(dTimeDiff1 * 24.0));

				CTSiDebugTrace::LogFmt(
					"ORBIT IN GetSatelliteTLE: SatId=%3lu -> hr=0x%08X TLEepoch=%I64d MsgTime=%I64d AgeInDays=%.3f "
					"OrbitNum=%d Incl=%.4f RAAN=%.4f Ecc=%.7f ArgPerigee=%.4f MeanAnomaly=%.4f MeanMotion=%.8f "
					"Ndot=%.9g Nddot=%.9g BStar=%.9g",
					ulSatId, (unsigned long)hr,
					tleData1.timeEpoch.intTime, timeMsg.intTime, dTimeDiff1,
					tleData1.nOrbitNumber,
					tleData1.elem.var.fInclination, tleData1.elem.var.fRightAscNode,
					tleData1.elem.var.fEccentricity, tleData1.elem.var.fArgPerigee,
					tleData1.elem.var.fMeanAnomaly, tleData1.elem.var.fMeanMotion,
					tleData1.fXndt2o, tleData1.fXndd6o, tleData1.fBStar);

				hr = m_lpOrbit->EarthFixedOrbitDouble( ulSatId, &timeMsg1, 1, &timeCoord, &ulCount );

				if( EMS_OK == hr )
				{
					CTSiDebugTrace::LogFmt(
						"ORBIT IN EarthFixedOrbit: SatId=%3lu -> hr=0x%08X count=%lu "
						"Pos(km) X=%10.3f Y=%10.3f Z=%10.3f  "
						"Vel(km/s) X=%8.5f Y=%8.5f Z=%8.5f",
						ulSatId, (unsigned long)hr, ulCount,
						timeCoord.coord.radius.dX,   timeCoord.coord.radius.dY,   timeCoord.coord.radius.dZ,
						timeCoord.coord.velocity.dX, timeCoord.coord.velocity.dY, timeCoord.coord.velocity.dZ);
				}
				else
				{
					CTSiDebugTrace::LogFmt(
						"ORBIT IN EarthFixedOrbit: SatId=%3lu -> *** hr=0x%08X (FAILED) ***",
						ulSatId, (unsigned long)hr);
				}
			}
			else
			{
				printf("Sat %d unavailable (hr = %d\n", ulSatId, hr);
				CTSiDebugTrace::LogFmt(
					"ORBIT IN GetSatelliteTLE: SatId=%3lu -> *** hr=0x%08X (FAILED - TLE unavailable) ***",
					ulSatId, (unsigned long)hr);
			}

			hr = m_lpOrbit->GetOrbitNumber( ulSatId, timeMsg, &ulOrbitNumber );
			CTSiDebugTrace::LogFmt(
				"ORBIT IN GetOrbitNumber:  SatId=%3lu -> hr=0x%08X OrbitNumber=%lu",
				ulSatId, (unsigned long)hr, ulOrbitNumber);

			if ( ( EMS_OK == hr ) && ( 0 <= ulOrbitNumber ) )
			{
				pRawLpCalibObj->SetPassId(ulOrbitNumber);
			}

			EMSTIMECOORDD timeCoordOrigonal;
			if ( ( EMS_OK == hr ) && ( 1 <= ulCount ) && (dTimeDiff1 < c_dOldOrbitInDays) ) //Orbit less than 2days
			{
				timeCoordOrigonal = pRawLpCalibObj->GetSatPVC();
				double rx = timeCoordOrigonal.coord.radius.dX - timeCoord.coord.radius.dX; //in Killometers
				double ry = timeCoordOrigonal.coord.radius.dY - timeCoord.coord.radius.dY;
				double rz = timeCoordOrigonal.coord.radius.dZ - timeCoord.coord.radius.dZ;

				double vx = timeCoordOrigonal.coord.velocity.dX - timeCoord.coord.velocity.dX; //in Killometers/sec
				double vy = timeCoordOrigonal.coord.velocity.dY - timeCoord.coord.velocity.dY;
				double vz = timeCoordOrigonal.coord.velocity.dZ - timeCoord.coord.velocity.dZ;

				if( (abs(rx) > c_dSatPositionDiffInKm) || (abs(ry) > c_dSatPositionDiffInKm) || (abs(rz) > c_dSatPositionDiffInKm) ||
					(abs(vx) > c_dSatVelDiffInKmPerSec) || (abs(vy) > c_dSatVelDiffInKmPerSec) || (abs(vz) > c_dSatVelDiffInKmPerSec))
				{
					CTSiDebugTrace::LogFmt(
						"ORBIT IN EarthFixedOrbit: SatId=%3lu PVC updated (diff: dR=%.3f,%.3f,%.3f dV=%.5f,%.5f,%.5f km)",
						ulSatId, rx, ry, rz, vx, vy, vz);
					pRawLpCalibObj->SetSatPVC( timeCoord);
				}
			}

			memset(&timeCoordOrigonal, 0, sizeof(EMSTIMECOORDD));
			timeCoordOrigonal = pRawLpCalibObj->GetSatPVC();

			if(timeCoordOrigonal.coord.radius.dX == 0.0 &&
				timeCoordOrigonal.coord.radius.dY == 0.0 &&
				timeCoordOrigonal.coord.radius.dZ == 0.0)
			{
				CTSiDebugTrace::LogFmt(
					"ORBIT IN _OrbitUpdate: SatId=%3lu *** zero position vector - returning EMS_FALSE ***", ulSatId);
				hr  = EMS_FALSE;
			}
		}
	}
	else
	{
		hr = EMS_BAD_PARAM;
	}

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_SetGSAzimuthElevation(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;
	if ( pRawLpCalibObj )
	{
	    EMSTIMECOORDD	vCoord;
		CEMSCoordSystem CS;
		EMSAZELVECTORD  vAzEl;
		EMSVECTORD      vDiff;
		EMSVECTORD      vObserver;
		EMSENUVECTORD   vENU;

		memset( &vCoord,0,sizeof(EMSTIMECOORDD ) );

		vCoord = pRawLpCalibObj->GetSatPVC();

		vObserver = CS.ToEarthFixedDouble( m_lsCalibData.siteLocation );

		vDiff.dX = vCoord.coord.radius.dX - vObserver.dX;
		vDiff.dY = vCoord.coord.radius.dY - vObserver.dY;
		vDiff.dZ = vCoord.coord.radius.dZ - vObserver.dZ;

		double dSinLatitude = sin( m_lsCalibData.siteLocation.degLatitude * c_dDegToRad );
		double dCosLatitude = cos( m_lsCalibData.siteLocation.degLatitude * c_dDegToRad );
		double dSinLongitude = sin( m_lsCalibData.siteLocation.degLongitude * c_dDegToRad );
		double dCosLongitude = cos( m_lsCalibData.siteLocation.degLongitude * c_dDegToRad );

		vENU.dNorth = -(( dSinLatitude * dCosLongitude * vDiff.dX ) +
						( dSinLatitude * dSinLongitude * vDiff.dY ) -
						( dCosLatitude * vDiff.dZ ) );

		vENU.dEast  =  (( -dSinLongitude * vDiff.dX ) +
						( dCosLongitude * vDiff.dY ) );

		vENU.dUp    =  (( dCosLatitude * dCosLongitude * vDiff.dX ) +
						( dCosLatitude * dSinLongitude * vDiff.dY ) +
						( dSinLatitude * vDiff.dZ ) );


		vAzEl = CS.ToAzimuthAndElevation( vENU );

		if ( vAzEl.azel.degAzimuth < 0.0 ) vAzEl.azel.degAzimuth += 360.0;
		pRawLpCalibObj->SetAzimuth(vAzEl.azel.degAzimuth);
		pRawLpCalibObj->SetElevation(vAzEl.azel.degElevation);
	}

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_RestoreMeasuredSignal( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;
	if ( pRawLpCalibObj )
	{
		double dFrequency  = pRawLpCalibObj->GetFrequency();
		dFrequency += pRawLpCalibObj->GetFreqOffset();
		pRawLpCalibObj->SetFrequency(dFrequency);
		pRawLpCalibObj->SetFreqOffset(0);

		EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();
		timeMsg.intTime += pRawLpCalibObj->GetTimeOffset();
		pRawLpCalibObj->SetTimeMsg(timeMsg);
		pRawLpCalibObj->SetTimeOffset(0);
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_RemoveDownlinkPropogationTime( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj )
	{
		INT64  i64DeltaTime = 0;
		EMSRANGERATED	rangeRateDL;
		EMSVECTORD		LUTVector;
		EMSTIMECOORDD	vCoord;
		CEMSCoordSystem CS;


		memset( &LUTVector,0,sizeof(EMSVECTORD ) );

		memset( &vCoord,0,sizeof(EMSTIMECOORDD ) );

		vCoord = pRawLpCalibObj->GetSatPVC();

		LUTVector  = CS.ToEarthFixedDouble( m_lsCalibData.siteLocation );

		hr = CEMSRangeRate::Calculate( LUTVector, &vCoord, 1, &rangeRateDL );

		i64DeltaTime = (INT64)(((double)rangeRateDL.dRange / c_dVelocityOfLight ) * (INT64)1000000000);

		//!
		//! update Time
		//! Calib406 Extended Record
		//!

		EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();

		timeMsg.intTime -= i64DeltaTime;
		pRawLpCalibObj->SetTimeMsg(timeMsg);


		INT64 i64TimeOffset = pRawLpCalibObj->GetTimeOffset();
		i64TimeOffset += i64DeltaTime;
		pRawLpCalibObj->SetTimeOffset(i64TimeOffset);

		// Update orbit vector to new TOA (time of arrival at satellite)

		hr = _OrbitUpdate( pRawLpCalibObj );			// capture the appropriate orbit vector
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

 // RR - maybe combine both downlink calculations into one.
EMS_RESULT 
CHGTCalibrationObj::_RemoveDownlinkDoppler( CEMSRawLpCalibObj* pRawLpCalibObj ) 
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj )
	{
		m_dDopplerShiftDL = 0.0;
		INT64  i64DeltaTime = 0;
		ULONG ulCount = 0;

		double dFreqDL = CEMSSatelliteProperties::GetDownlinkFrequency(pRawLpCalibObj->GetSatId() );

		double dFreqDLoffset = pRawLpCalibObj->GetFrequency() - 406050000.0;

		// Verbose tier (needs the .debug marker) - fires once per candidate per
		// DBF scan, same volume as the "ORBIT IN ..." lines. Lets us compare the
		// GetDownlinkFrequency() value actually used per SatId (e.g. 4xx/GAL vs
		// 6xx/BDS) against the raw received frequency, to check whether this
		// external per-satellite table is where a constellation-family-specific
		// mixing/downlink frequency should be (but isn't) accounted for.
		CTSiDebugTrace::LogFmt(
			"DOWNLINK IN _RemoveDownlinkDoppler: SatId=%3lu GetDownlinkFrequency=%.3f RxFreq=%.3f dFreqDLoffset=%.3f",
			pRawLpCalibObj->GetSatId(), dFreqDL, pRawLpCalibObj->GetFrequency(), dFreqDLoffset);

		EMSRANGERATED	rangeRateDL;
		EMSVECTORD		LUTVector;
		EMSTIMECOORDD	vCoord;
		CEMSCoordSystem CS;

		memset( &LUTVector,0,sizeof(EMSVECTORD ) );

		memset( &vCoord,0,sizeof(EMSTIMECOORDD ) );

		vCoord = pRawLpCalibObj->GetSatPVC();


		LUTVector  = CS.ToEarthFixedDouble( m_lsCalibData.siteLocation );

		hr = CEMSRangeRate::Calculate( LUTVector, &vCoord, 1, &rangeRateDL );


		double dV = rangeRateDL.dRangeRate / c_dVelocityOfLight;

		m_dDopplerShiftDL = (1.0 - sqrt( (1.0-dV)/(1.0+dV) )) * (-dFreqDL-dFreqDLoffset) ;
					
		// Check if LUT receiver performs spectrum inversion
		// Assume true only if satellite spectrum is inverted.
		if ( EMS_C406PF_SAT_INVERTED_SPECTRUM_FLAG == ( EMS_C406PF_SAT_INVERTED_SPECTRUM_FLAG & pRawLpCalibObj->GetProcessFlags() ) )
		{
		//	if ( EMS_C406PF_LUT_INVERTED_SPECTRUM_FLAG == 
		//		( EMS_C406PF_LUT_INVERTED_SPECTRUM_FLAG & pRawLpCalibObj->GetProcessFlags() ) )
			{
				m_dDopplerShiftDL = (1.0 - sqrt( (1.0-dV)/(1.0+dV) )) * (dFreqDL-dFreqDLoffset);
			}
		}
		//!
		//! update Frequency 
		//! Calib406 Extended Record
		//!

		double dFrequency = pRawLpCalibObj->GetFrequency();
		dFrequency -= m_dDopplerShiftDL;
		pRawLpCalibObj->SetFrequency(dFrequency);

		double dFreqOffset = pRawLpCalibObj->GetFreqOffset();
		dFreqOffset += (float)m_dDopplerShiftDL;
		pRawLpCalibObj->SetFreqOffset(dFreqOffset);

		//Set the downlink
		//pRawLpCalibObj->SetResolutionFlags(DWORD(m_dDopplerShiftDL*10));
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_DoSpectrumInversion( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj )
	{
		double dDeltaF = 0.0;

		dDeltaF = pRawLpCalibObj->GetFrequency() - 406000000.0;

		dDeltaF = 100000.0 - dDeltaF;

		pRawLpCalibObj->SetFrequency(406000000.0 + dDeltaF);
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_CorrectDownLink(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;

	if ( EMS_C406PF_SAT_REMOVE_DLINKTIME_FLAG == 
								( EMS_C406PF_SAT_REMOVE_DLINKTIME_FLAG & pRawLpCalibObj->GetProcessFlags() ) )
		hr = _RemoveDownlinkPropogationTime( pRawLpCalibObj );

	if ( EMS_C406PF_SAT_REMOVE_DLINKFREQ_FLAG == 
								( EMS_C406PF_SAT_REMOVE_DLINKFREQ_FLAG & pRawLpCalibObj->GetProcessFlags() ) )
		hr = _RemoveDownlinkDoppler( pRawLpCalibObj );


	return hr;
}


EMS_RESULT
CHGTCalibrationObj::_CalcTimeResidual( EMSRANGERATED rangeRateUL, CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj )
	{
		double dResidualTime = 0.0; //in seconds
		INT64  i64ResidualTime = 0;
		INT64  i64DeltaTimeUL = 0;
		INT64  i64PredictedTime = 0;
		

		i64DeltaTimeUL  = (INT64)(((double)rangeRateUL.dRange / c_dVelocityOfLight ) * (INT64)1000000000);
		
		//EMSTIME timeTransmit = pRawLpCalibObj->GetTransmitTime();
		//if ( timeTransmit.intTime > 0 )
		//{

		//	i64PredictedTime = timeTransmit.intTime + i64DeltaTimeUL; 

		//	EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();
		//	i64ResidualTime	 = timeMsg.intTime - i64PredictedTime;

		//	dResidualTime = (double)(i64ResidualTime *1e-9);

		//}
		//else
		{
			EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();
			EMSTIME timeTx;
			timeTx.intTime = timeMsg.intTime - i64DeltaTimeUL;
			pRawLpCalibObj->SetTransmitTime(timeTx);

			dResidualTime = (double) (timeTx.intTime * 1e-9);
			
			dResidualTime -= (double) ((int)dResidualTime);

		}

		//Residual should be in between -0.5 to +0.5 - Change 27th May 2021
		if(dResidualTime > 0.5)
			dResidualTime -= 1.0;
		else if(dResidualTime < -0.5)
			dResidualTime += 1.0;

		pRawLpCalibObj->SetResidualTime( dResidualTime );

	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

EMS_RESULT
CHGTCalibrationObj::_CalcFreqResidual( EMSRANGERATED rangeRateUL, CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;

	if(pRawLpCalibObj)
	{
		m_dDopplerShiftUL = 0.0;
		double dPredictedFreqUL = 0.0;
		double dResidualFreq = 0.0;
		double dTxFrequency = pRawLpCalibObj->GetTxFrequency();
		double dFreqScaleUL = 0.0;

		double dFrequency = 0.0;

		
		dFreqScaleUL =  dTxFrequency / c_dVelocityOfLight;

		m_dDopplerShiftUL = rangeRateUL.dRangeRate * (-dFreqScaleUL);

		if ( dTxFrequency > 406.0e6 )
		{
			dFrequency = pRawLpCalibObj->GetFrequency();
			
			dPredictedFreqUL = dTxFrequency + m_dDopplerShiftUL;

			dResidualFreq   = dFrequency;

			dResidualFreq  -= dPredictedFreqUL;

			pRawLpCalibObj->SetResidualFrequency(dResidualFreq);
		}

		//pRawLpCalibObj->SetDataFlags(DWORD(m_dDopplerShiftUL*10));
	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}


EMS_RESULT 
CHGTCalibrationObj::_ComputeTOAFOAResidual(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;

	if(pRawLpCalibObj)
	{
		EMSRANGERATED rangeRateUL;

		EMSTIMECOORDD vSatellite;
		vSatellite.coordSystem = COORDSYS_EarthFixed;
		vSatellite.vTime = pRawLpCalibObj->GetTimeMsg();
		vSatellite.coord = pRawLpCalibObj->GetSatPVC().coord;

		pRawLpCalibObj->SetSatPVC(vSatellite);


		hr = CEMSRangeRate::Calculate( pRawLpCalibObj->GetBcnVector(), 
												&vSatellite, 1, &rangeRateUL );

		hr = _CalcTimeResidual( rangeRateUL, pRawLpCalibObj );

		hr = _CalcFreqResidual( rangeRateUL, pRawLpCalibObj );

		//Statistics
		if(pRawLpCalibObj->GetProcessFlags() & ( EMS_C406PF_REF_FREQ_FLAG | EMS_C406PF_REF_TIME_FLAG ) )
		{
			hr = _CalculateToaFoaOffsets( pRawLpCalibObj ); 
		}
	}
	else
		hr = EMS_BAD_PARAM;

	
	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_ComputeResidual(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;

	if(pRawLpCalibObj)
	{

		EMSRANGERATED rangeRateUL;
		hr = CEMSRangeRate::Calculate( pRawLpCalibObj->GetBcnVector(), 
										&pRawLpCalibObj->GetSatPVC(), 1, &rangeRateUL );

		hr = _CalcTimeResidual( rangeRateUL, pRawLpCalibObj );

		hr = _CalcFreqResidual( rangeRateUL, pRawLpCalibObj );

		//Statistics

		if(pRawLpCalibObj->GetProcessFlags() & ( EMS_C406PF_REF_FREQ_FLAG | EMS_C406PF_REF_TIME_FLAG ) )
		{
			hr = _CalculateOffsets( pRawLpCalibObj );
		}
	}
	else
		hr = EMS_BAD_PARAM;

	
	return hr;
}

BOOL 
CHGTCalibrationObj::IsRLS( const BYTE *msg) 
{
	if( !msg )
		return false;
	BYTE prot = 0;

	CBeaconMsg bcn;
	bcn.Set( msg );
	bcn.IsNational( &prot );

	if( prot == CBeaconMsgBase::BP_NAT_RLS )
	{
		return true;
	}
	return false;	
}

BOOL 
CHGTCalibrationObj::IsELT_DT( const BYTE *msg) 
{
	if( !msg )
		return false;
	BYTE prot = 0;

	CBeaconMsg bcn;
	bcn.Set( msg );
	bcn.IsNational( &prot );

	if( prot == CBeaconMsgBase::BP_ELT_DT )
	{
		return true;
	}
	return false;	
}

void
CHGTCalibrationObj::_CheckPDF2( CEMSRawLpCalibObj* pRawLpCalibObj )  //Looking for fixed bits
{

	BYTE* pBeaconMsg = pRawLpCalibObj->GetBeaconMsg();

	if ( pBeaconMsg[3] & 0x80 && !IsRLS(pBeaconMsg ) && !IsELT_DT(pBeaconMsg )) // Long message test
	{
		if ( pBeaconMsg[17] == 0x00 && pBeaconMsg[16] == 0x00
		  && pBeaconMsg[15] == 0x00 && pBeaconMsg[14] == 0x00 )
		{
		  pBeaconMsg[14] = 0xFF;
		  pBeaconMsg[15] = 0xFF;
		  pBeaconMsg[16] = 0xFF;
		  pBeaconMsg[17] = 0xFF;

		  pRawLpCalibObj->SetBeaconMsg(pBeaconMsg);
		  pRawLpCalibObj->SetBitErrorField2(5);
		}
	}

	return;
}

void
CHGTCalibrationObj::_CheckProtocol( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	CBeaconMsg oBcn;
	bool bSetPDF2 = false;

	BYTE* pBeaconMsg = pRawLpCalibObj->GetBeaconMsg();
	oBcn.Set(pBeaconMsg);

	if( !oBcn.getProtocolFlag() && oBcn.getFormatFlag() )
	{
		ULONG ulProt = oBcn.getProtocolCode();
		if ( ulProt == CBeaconMsgBase::BP_NAT_RLS || ulProt == CBeaconMsgBase::BP_ELT_DT )
		//if ( ulProt == 13 || ulProt == 9 )
		{
			bSetPDF2 = false;
		}
		else if( oBcn.IsNational() )
		{
			// Check fixed bits 107 to 109 set to '110'
			if ( ! ( oBcn.getBit(107) && oBcn.getBit(108) && !oBcn.getBit(109) ) )
				bSetPDF2 = true;
		}
		else if( oBcn.IsStandard() )
		{
			// Check fixed bits 107 to 110 set to '1101'
			if ( ! ( oBcn.getBit(107) && oBcn.getBit(108) && !oBcn.getBit(109) && oBcn.getBit(110) ) )
				bSetPDF2 = true;
		}

		if ( bSetPDF2 )
		{
			pBeaconMsg[14] = 0xFF;
			pBeaconMsg[15] = 0xFF;
			pBeaconMsg[16] = 0xFF;
			pBeaconMsg[17] = 0xFF;

			pRawLpCalibObj->SetBeaconMsg(pBeaconMsg);
			pRawLpCalibObj->SetBitErrorField2(4);
		}
	}

	// Set protocol and Country Code
	WORD wProtocol = pBeaconMsg[4] & 0x0F;
	pRawLpCalibObj->SetDuplicateMsgCount(wProtocol);
	pRawLpCalibObj->SetResolutionFlags(oBcn.getCountryCode());

	return;
}

EMS_RESULT 
CHGTCalibrationObj::_SetOffsets()
{
	EMS_RESULT hr = EMS_OK;

	m_sarrCalibData.SetSarrNumber(m_ulSARRNumber);
	m_sarrCalibData.SetSatId(m_lsCalibData.ulConstellation);
	m_sarrCalibData.SetSource(m_lsCalibData.wAntId);
	m_sarrCalibData.SetFreqOffset((float)m_lsCalibData.dFoaBias);
	m_sarrCalibData.SetFreqDrift(0);
	m_sarrCalibData.SetTimeOffset((INT64)(m_lsCalibData.dToaBias * 1e9) );
	m_sarrCalibData.SetTimeEffective(m_lsCalibData.timeLastUpdate);

	return hr;
}

bool
CHGTCalibrationObj::_IsHarmonic( double dFreqDiff )
{
	bool bOk = false;
	double dHarmonic  = 800.0;
	double dThreshold = 5.0;

	if( fabs( fabs( dFreqDiff ) -   dHarmonic ) < dThreshold ) bOk = true;
	if( fabs( fabs( dFreqDiff ) - 2*dHarmonic ) < dThreshold ) bOk = true;
	if( fabs( fabs( dFreqDiff ) - 4*dHarmonic ) < dThreshold ) bOk = true;
	if( fabs( fabs( dFreqDiff ) - 6*dHarmonic ) < dThreshold ) bOk = true;

	return bOk;
}

void
CHGTCalibrationObj::_UpdateAlarmStatus( double dValue )
{
	int iIndex = -1;

	for( int i = 0; i < c_iMaxDataSource; i++ )
	{
		if( (m_aDataSourceStatus[i].ulSatID == m_lsCalibData.ulConstellation) &&
			(m_aDataSourceStatus[i].ulLUTID == m_lsCalibData.ulLutId) )
		{
			iIndex = i;
			break;
		}
	}

	if( -1 == iIndex )
	{
		if( fabs(dValue) > c_dDefaultAlarmThreshold )
		{
			for( int i = 0; i < c_iMaxDataSource; i++ )
			{
				if( (0 == m_aDataSourceStatus[i].ulSatID) &&
					(0 == m_aDataSourceStatus[i].ulLUTID) )
				{
					m_aDataSourceStatus[i].ulSatID = m_lsCalibData.ulConstellation;
					m_aDataSourceStatus[i].ulLUTID = m_lsCalibData.ulLutId;
					m_aDataSourceStatus[i].ulAntID = (ULONG)m_lsCalibData.wAntId;
					m_aDataSourceStatus[i].eStatus = EMS_CALIB_SOURCE_OK;
					m_aDataSourceStatus[i].iAlarmCount = 0;
					iIndex = i;
					break;
				}
			}
		}
	}

	if( 0 <= iIndex )
	{
		if( fabs(dValue) > c_dDefaultAlarmThreshold )
		{
			m_aDataSourceStatus[iIndex].iAlarmCount++;
			if( m_aDataSourceStatus[iIndex].iAlarmCount > c_iDefaultAlarmCount )
			{
				if( EMS_CALIB_SOURCE_OK == m_aDataSourceStatus[iIndex].eStatus )
				{
					// fire an alarm;
					CEMSLogMsgParam	param(5);

					param.Add( m_lsCalibData.ulConstellation );
					param.Add( m_lsCalibData.ulLutId );
					param.Add( m_lsCalibData.wAntId );
					param.Add( m_aDataSourceStatus[iIndex].iAlarmCount );
					param.AddDouble( dValue );

					CEMSLogHelper::LogInfoMsg( EMS_CALIB406_DATA_SOURCE_PROBLEM, param );

					m_aDataSourceStatus[iIndex].eStatus = EMS_CALIB_SOURCE_ALARM;
				}

				m_aDataSourceStatus[iIndex].iAlarmCount = 0;				
			}
		}
		else
		{
			m_aDataSourceStatus[iIndex].iAlarmCount = 0;
			if( EMS_CALIB_SOURCE_ALARM == m_aDataSourceStatus[iIndex].eStatus )
			{
				// clear alarm, or inform the operator that it is ok
				m_aDataSourceStatus[iIndex].eStatus = EMS_CALIB_SOURCE_OK;
				CEMSLogMsgParam	param(5);

				param.Add( m_lsCalibData.ulConstellation );
				param.Add( m_lsCalibData.ulLutId );
				param.Add( m_lsCalibData.wAntId );
				param.Add( m_aDataSourceStatus[iIndex].iAlarmCount );
				param.AddDouble( dValue );

				CEMSLogHelper::LogInfoMsg( EMS_CALIB406_DATA_SOURCE_PROBLEM_CLEARED, param );
			}

			memset( &m_aDataSourceStatus[iIndex], 0, sizeof( EMSDataSourceStatus ) );
		}
	}
}

bool
CHGTCalibrationObj::_CheckDataSourceStatus()
{
	bool bOk = true;
	int iIndex = -1;

	for( int i = 0; i < c_iMaxDataSource; i++ )
	{
		if( (m_aDataSourceStatus[i].ulSatID == m_lsCalibData.ulConstellation) &&
			(m_aDataSourceStatus[i].ulLUTID == m_lsCalibData.ulLutId) )
		{
			iIndex = i;
			break;
		}
	}

	if( 0 <= iIndex )
	{
		bOk = (EMS_CALIB_SOURCE_OK == m_aDataSourceStatus[iIndex].eStatus);
	}

	return bOk;
}

bool
CHGTCalibrationObj::_CheckDataSourceTime()
{
	bool bOk = false;
	int iIndex = -1;

	for( int i = 0; i < c_iMaxDataSource; i++ )
	{
		if( (m_aDataSourceStatus[i].ulSatID == m_lsCalibData.ulConstellation) &&
			(m_aDataSourceStatus[i].ulLUTID == m_lsCalibData.ulLutId) )
		{
			iIndex = i;
			break;
		}
	}

	if( 0 <= iIndex )
	{
		INT64 tdiff = m_lsCalibData.timeLastUpdate.intTime - m_aDataSourceStatus[iIndex].tLastUpdate.intTime;
		if( tdiff < 3600 * c_iNanoSecsPerSec ) bOk = true;
		m_aDataSourceStatus[iIndex].tLastUpdate = m_lsCalibData.timeLastUpdate;
	}

	return bOk;
}

EMS_RESULT 
CHGTCalibrationObj::_CalculateToaFoaOffsets( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_FALSE;
	
	if ( pRawLpCalibObj )
	{
		// Check if a strong signal and it is not a harmonic of a reference beacon
		if ( !_IsHarmonic(pRawLpCalibObj->GetResidualFrequency() )
			&& (pRawLpCalibObj->GetCarrierPower() > c_dDefaultCNRFilterThreshold) )
		{
				
			bool bTimeOK   = _CheckDataSourceTime();
			bool bStatusOK = _CheckDataSourceStatus();
	
			if ( EMS_C406PF_REF_FREQ_FLAG & pRawLpCalibObj->GetProcessFlags() ) 
			{
				double dAlphaFreq	= m_lsCalibData.dFoaGain;
				double dFreqDiff	= 0.0;

				dFreqDiff = pRawLpCalibObj->GetResidualFrequency();

				dAlphaFreq = m_lsCalibData.dFoaGain;

				// RR Sept 30, 2021 - Do not allow restart but assume calibration values are predetermined approximately
				//if( !bTimeOK || !bStatusOK || m_lsCalibData.ulFoaCount > c_ulConvergenceTestCount)
				//{
					if ( dFreqDiff >  m_lsCalibData.dFoaThreshold ) dFreqDiff =  m_lsCalibData.dFoaThreshold;
					if ( dFreqDiff < -m_lsCalibData.dFoaThreshold ) dFreqDiff = -m_lsCalibData.dFoaThreshold;
				//}
				//else
				//{
				//	dAlphaFreq = 1.0;
				//}
			

				m_lsCalibData.dFoaBias += dAlphaFreq * dFreqDiff;
				m_lsCalibData.timeLastUpdate = pRawLpCalibObj->GetTimeMsg();

				//pRawLpCalibObj->SetFreqOffset(m_lsCalibData.dFoaBias);

				m_lsCalibData.ulFoaCount++;

				hr = EMS_OK;

				CTSiDebugTrace::LogFmtAlways(
					"LP BIAS UPDATE FOA: LutId=%lu AntId=%u Const=%lu "
					"freqDiff=%.3f alpha=%.5f newFOABias=%.6f count=%lu",
					m_lsCalibData.ulLutId, (unsigned)m_lsCalibData.wAntId,
					m_lsCalibData.ulConstellation,
					dFreqDiff, dAlphaFreq,
					m_lsCalibData.dFoaBias, m_lsCalibData.ulFoaCount);
			}

			if ( EMS_C406PF_REF_TIME_FLAG & pRawLpCalibObj->GetProcessFlags() )
			{
				double dAlphaTime	= m_lsCalibData.dToaGain;
				double dTimeDiff	= 0.0;

				dTimeDiff = pRawLpCalibObj->GetResidualTime();
 
				//Assumes reference beacon 24th bit occurs at the 220 millisecs mark.
				if(dTimeDiff > 0.2 && dTimeDiff < 0.24)
					dTimeDiff -= 0.22;
				if(dTimeDiff > 0.9)
					dTimeDiff -= 1.0;
				// RR Sept 30, 2021 - Do not allow restart but assume calibration values are predetermined approximately
				//if( !bTimeOK || !bStatusOK || m_lsCalibData.ulToaCount > c_ulConvergenceTestCount )
				{
					// dTimeDiff (GetResidualTime()) is in seconds - see the 220-millisec
					// comment above - but dToaThreshold is configured/stored in
					// milliseconds (lscalibdata.csv "TOA Threshold (msec)"), so it must be
					// converted before being compared against dTimeDiff.
					double dToaThresholdSec = m_lsCalibData.dToaThreshold / 1000.0;
					if ( dTimeDiff >  dToaThresholdSec ) dTimeDiff =  dToaThresholdSec;
					if ( dTimeDiff < -dToaThresholdSec ) dTimeDiff = -dToaThresholdSec;
				}
				//else
				//{
				//	dAlphaTime = 1.0;
				//}
				

				m_lsCalibData.dToaBias += (dAlphaTime * dTimeDiff);

				m_lsCalibData.timeLastUpdate = pRawLpCalibObj->GetTimeMsg();

				//pRawLpCalibObj->SetTimeOffset((INT64)(m_lsCalibData.dToaBias * 1e9));

				m_lsCalibData.ulToaCount++;

				hr = EMS_OK;

				CTSiDebugTrace::LogFmtAlways(
					"LP BIAS UPDATE TOA: LutId=%lu AntId=%u Const=%lu "
					"timeDiff=%.6f alpha=%.6f newTOABias=%.9f count=%lu",
					m_lsCalibData.ulLutId, (unsigned)m_lsCalibData.wAntId,
					m_lsCalibData.ulConstellation,
					dTimeDiff, dAlphaTime,
					m_lsCalibData.dToaBias, m_lsCalibData.ulToaCount);

			}

			if ( EMS_OK == hr )
			{

				m_ulSARRNumber++;
					
				hr = _SetOffsets();

				m_sarrCalibData.SetLutId(pRawLpCalibObj->GetLutId());
				m_sarrCalibData.SetTime(pRawLpCalibObj->GetTime());
				m_sarrCalibData.SetSatId(pRawLpCalibObj->GetSatId()); //Output SatId
				m_sarrCalibData.SetPassId(pRawLpCalibObj->GetPassId());

				m_bToaFoaSarrUpdated = true;

			}

		}
	}


	return hr;
}


EMS_RESULT 
CHGTCalibrationObj::_CalculateOffsets( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_FALSE;
	
	if ( pRawLpCalibObj )
	{
		// Check if a strong signal and it is not a harmonic of a reference beacon
		//if ( !_IsHarmonic(pRawLpCalibObj->GetResidualFrequency() )
		//	&& (pRawLpCalibObj->GetCarrierPower() > c_dDefaultCNRFilterThreshold) ) 
		if ( pRawLpCalibObj->GetCarrierPower() > c_dDefaultCNRFilterThreshold )  
		{

			m_lsCalibData.timeLastUpdate = pRawLpCalibObj->GetTimeMsg(); //will be used in _CheckDataSourceTime

	
			bool bTimeOK   = _CheckDataSourceTime();
			bool bStatusOK = _CheckDataSourceStatus();

			if ( EMS_C406PF_REF_FREQ_FLAG & pRawLpCalibObj->GetProcessFlags() ) 
			{
				double dAlphaFreq	= m_lsCalibData.dFoaGain;
				double dFreqDiff	= 0.0;

				dFreqDiff = pRawLpCalibObj->GetResidualFrequency();

				_UpdateAlarmStatus( dFreqDiff );

				dAlphaFreq = m_lsCalibData.dFoaGain;

				// Plausibility bound, applied BEFORE the threshold clamp below.
				//
				// The clamp limits how far one detection can move the bias, but it
				// does not question whether the detection should move it at all -
				// it turns any outlier into a full-magnitude push, and a
				// systematically misidentified satellite then walks the bias
				// steadily in one direction. A residual of this size is not
				// evidence that the stored bias drifted; it is evidence that this
				// detection belongs to a different satellite.
				//
				// In the 2026-08-06 capture, 30 accepted detections carried
				// residuals of 4-10 kHz (sat 408 clustered at -9999.4..-9999.97,
				// sat 403 at -7300..-8588, sat 436 at -4036..-4173). All were
				// discarded downstream by the LP output threshold, but each would
				// have contributed a clamped 10 Hz push here first - roughly 300 Hz
				// against a signal whose median residual is now 0.3 Hz.
				if ( fabs(dFreqDiff) >= c_dMaxBiasUpdateResidualHz )
				{
					CTSiDebugTrace::LogFmtAlways(
						"SP BIAS SKIPPED FOA: LutId=%lu AntId=%u Const=%lu freqDiff=%.3f "
						"exceeds plausibility bound %.1f Hz - treating as misidentification, "
						"not bias drift; bias left at %.6f count=%lu",
						m_lsCalibData.ulLutId, (unsigned)m_lsCalibData.wAntId,
						m_lsCalibData.ulConstellation, dFreqDiff,
						c_dMaxBiasUpdateResidualHz,
						m_lsCalibData.dFoaBias, m_lsCalibData.ulFoaCount);
				}
				else
				{
					// Check if alarm condition or initialization
					//if ( !bTimeOK || !bStatusOK || (m_lsCalibData.ulFoaCount < c_ulConvergenceTestCount ) )
					//{
						//dAlphaFreq = 1.0;
					//}

					// Threshold test
					//if(m_lsCalibData.ulFoaCount > c_ulConvergenceTestCount )
					{
						if ( dFreqDiff >  m_lsCalibData.dFoaThreshold ) dFreqDiff =  m_lsCalibData.dFoaThreshold;
						if ( dFreqDiff < -m_lsCalibData.dFoaThreshold ) dFreqDiff = -m_lsCalibData.dFoaThreshold;
					}

					m_lsCalibData.dFoaBias += dAlphaFreq * dFreqDiff;
					m_lsCalibData.timeLastUpdate = pRawLpCalibObj->GetTimeMsg();

					//pRawLpCalibObj->SetFreqOffset(m_lsCalibData.dFoaBias);

					m_lsCalibData.ulFoaCount++;

					CTSiDebugTrace::LogFmtAlways(
						"SP BIAS UPDATE FOA: LutId=%lu AntId=%u Const=%lu "
						"freqDiff=%.3f alpha=%.5f newFOABias=%.6f count=%lu",
						m_lsCalibData.ulLutId, (unsigned)m_lsCalibData.wAntId,
						m_lsCalibData.ulConstellation,
						dFreqDiff, dAlphaFreq,
						m_lsCalibData.dFoaBias, m_lsCalibData.ulFoaCount);

					hr = EMS_OK;
				}
			}

			if ( EMS_C406PF_REF_TIME_FLAG & pRawLpCalibObj->GetProcessFlags() )
			{
				double dAlphaTime	= m_lsCalibData.dToaGain;
				double dTimeDiff	= 0.0;

				dTimeDiff = pRawLpCalibObj->GetResidualTime();

				//Assumes reference beacon 24th bit occurs at the 220 millisecs mark.
				if(dTimeDiff > 0.2 && dTimeDiff < 0.24)
					dTimeDiff -= 0.220;

				// RR Jan 19, 2022 Assume TOA bias is established and only small changes are permitted.
				//if ( !bTimeOK || !bStatusOK || m_lsCalibData.ulToaCount > c_ulConvergenceTestCount )
				{
					// dTimeDiff (GetResidualTime()) is in seconds - see the 220-millisec
					// comment above - but dToaThreshold is configured/stored in
					// milliseconds (lscalibdata.csv "TOA Threshold (msec)"), so it must be
					// converted before being compared against dTimeDiff.
					double dToaThresholdSec = m_lsCalibData.dToaThreshold / 1000.0;
					if ( dTimeDiff >  dToaThresholdSec ) dTimeDiff =  dToaThresholdSec;
					if ( dTimeDiff < -dToaThresholdSec ) dTimeDiff = -dToaThresholdSec;
				}
				//else
				//{
				//	dAlphaTime = 1.0;
				//}

				m_lsCalibData.dToaBias += (dAlphaTime * dTimeDiff);

				m_lsCalibData.timeLastUpdate = pRawLpCalibObj->GetTimeMsg();

				//pRawLpCalibObj->SetTimeOffset((INT64)(m_lsCalibData.dToaBias * 1e9));

				m_lsCalibData.ulToaCount++;

				CTSiDebugTrace::LogFmtAlways(
					"SP BIAS UPDATE TOA: LutId=%lu AntId=%u Const=%lu "
					"timeDiff=%.6f alpha=%.6f newTOABias=%.9f count=%lu",
					m_lsCalibData.ulLutId, (unsigned)m_lsCalibData.wAntId,
					m_lsCalibData.ulConstellation,
					dTimeDiff, dAlphaTime,
					m_lsCalibData.dToaBias, m_lsCalibData.ulToaCount);

				hr = EMS_OK;

			}



			if ( EMS_OK == hr )
			{

				m_ulSARRNumber++;
					
				hr = _SetOffsets();

				m_sarrCalibData.SetLutId(pRawLpCalibObj->GetLutId());
				m_sarrCalibData.SetTime(pRawLpCalibObj->GetTime());
				m_sarrCalibData.SetSatId(pRawLpCalibObj->GetSatId()); //output SatId
				m_sarrCalibData.SetPassId(pRawLpCalibObj->GetPassId());

				m_bSpSarrUpdated = true;

			}

		}
	}


	return hr;
}

std::string 
CHGTCalibrationObj::_FormatInputTime( EMSTIME time )
{
    std::string ret;
	CEMSTime Time( time );
    EMSTIMEFIELDS timef;

	if ( time.intTime < 0 )
	{
		timef.nYear = 1970;
		timef.nJulianDay = 0;
		timef.nHour = 0;
		timef.nMinute = 0;
		timef.nSecond = 0;
		timef.lNanosecond = 0;
	}
	else
	{
		Time.GetTime( &timef );
	}

	char cTimeBuffer[64];
	memset( cTimeBuffer, 0, 64 );

	_snprintf( cTimeBuffer, sizeof(cTimeBuffer)-1, "%04d/%03d %02d:%02d:%02d.%06d",
		timef.nYear, timef.nJulianDay, timef.nHour, timef.nMinute, timef.nSecond, timef.lNanosecond / 1000); 

	ret = cTimeBuffer;

	return ret;
}

EMS_RESULT 
CHGTCalibrationObj::_AdjustSarrFreqAndTime( CEMSRawLpCalibObj* pRawLpCalibObj )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj  )
	{

		// correct Freq & Time

		double dFrequency = pRawLpCalibObj->GetFrequency();
		dFrequency -= m_lsCalibData.dFoaBias;
		pRawLpCalibObj->SetFrequency(dFrequency);

		double dFreqOffset = pRawLpCalibObj->GetFreqOffset();
		dFreqOffset += m_lsCalibData.dFoaBias;
		pRawLpCalibObj->SetFreqOffset(dFreqOffset); //freq offset is added


		EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();

		timeMsg.intTime -= (INT64)(m_lsCalibData.dToaBias * 1e9);
		pRawLpCalibObj->SetTimeMsg(timeMsg);

		INT64 i64TimeOffset = pRawLpCalibObj->GetTimeOffset();
		i64TimeOffset += (INT64)(m_lsCalibData.dToaBias * 1e9);
		pRawLpCalibObj->SetTimeOffset(i64TimeOffset);

		// Add calibration frequency to data record
		double dReserve3 = pRawLpCalibObj->GetReserve3();
		dReserve3 = m_lsCalibData.dFoaBias;
		pRawLpCalibObj->SetReserve3(dReserve3);
	}
	else
		hr = EMS_BAD_PARAM;


	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_ApplyTOAFOAAdjustments(CEMSRawLpCalibObj* pRawLpCalibObj) 
{
	EMS_RESULT hr = EMS_OK;

	if(pRawLpCalibObj)
	{

		// Special test for zero fill PDF2 fields
		_CheckPDF2( pRawLpCalibObj );

		// Special test for fixed bits in National and Standard Location Protocol
		_CheckProtocol( pRawLpCalibObj );

		hr = _AdjustSarrFreqAndTime( pRawLpCalibObj );

		switch ( pRawLpCalibObj->GetType() )
		{

			case EMSCALIBTYPE_406_SARR:
			case INTERFERER_TYPE:
			case MISCODED_TYPE:
				// This forces the time resolution flag to highest level for SARR records
				//(Note: It is needed because data from other MEOLUTs using the XML, 722 formats,
				// does not contain this flag)
				{
					DWORD dwResolutionFlags = pRawLpCalibObj->GetResolutionFlags();
					dwResolutionFlags |= EMS_C406RF_TIMERES_MASK;
					pRawLpCalibObj->SetResolutionFlags(dwResolutionFlags);
				}

				break;
			
			default:			
				break;

		}


	}

	return hr;

}

EMS_RESULT 
CHGTCalibrationObj::_DataValidation(CEMSRawLpCalibObj* pRawLpCalibObj)
{
	EMS_RESULT hr = EMS_OK;
	if(pRawLpCalibObj)
	{


		switch ( pRawLpCalibObj->GetType() )
		{

			case EMSCALIBTYPE_406_SARR:
			case INTERFERER_TYPE:
			case MISCODED_TYPE:
				// This forces the time resolution flag to highest level for SARR records
				//(Note: It is needed because data from other MEOLUTs using the XML, 722 formats,
				// does not contain this flag)
				{

	/*				BYTE *cBeaconMsg = pRawLpCalibObj->GetBeaconMsg();
					if ( cBeaconMsg[2] == (ULONG)0xD0 )
					{
						pRawLpCalibObj->SetReserve0(0x0000ffff);
					}*/

					DWORD dwResolutionFlags = pRawLpCalibObj->GetResolutionFlags();
					dwResolutionFlags |= EMS_C406RF_TIMERES_MASK;
					pRawLpCalibObj->SetResolutionFlags(dwResolutionFlags);

					DWORD dwPocessFlags = pRawLpCalibObj->GetProcessFlags();
					dwPocessFlags |= EMS_C406PF_PROCESS_MASK;
					pRawLpCalibObj->SetProcessFlags(dwPocessFlags);
					
				}

				break;
			
			default:			
				break;

		}

	}
	return hr;

}

EMS_RESULT 
CHGTCalibrationObj::_AdjustDBFSarrFreqAndTime( CEMSRawLpCalibObj* pRawLpCalibObj, HGTLSCALIBDATA& lsCalibData, ULONG ulSat )
{
	EMS_RESULT hr = EMS_OK;

	if ( pRawLpCalibObj  )
	{

		// correct Freq & Time

		double dFrequency = pRawLpCalibObj->GetFrequency();
		dFrequency -= lsCalibData.dFoaBias;
		pRawLpCalibObj->SetFrequency(dFrequency);

		double dFreqOffset = pRawLpCalibObj->GetFreqOffset();
		dFreqOffset += lsCalibData.dFoaBias;
		pRawLpCalibObj->SetFreqOffset(dFreqOffset); //freq offset is added


		EMSTIME timeMsg = pRawLpCalibObj->GetTimeMsg();

		timeMsg.intTime -= (INT64)(lsCalibData.dToaBias * 1e9);
		pRawLpCalibObj->SetTimeMsg(timeMsg);

		INT64 i64TimeOffset = pRawLpCalibObj->GetTimeOffset();
		i64TimeOffset += (INT64)(lsCalibData.dToaBias * 1e9);
		pRawLpCalibObj->SetTimeOffset(i64TimeOffset);

		// Add calibration frequency to data record - Do we need the below one ?????
		double dReserve3 = pRawLpCalibObj->GetReserve3();
		dReserve3 = lsCalibData.dFoaBias;
		pRawLpCalibObj->SetReserve3(dReserve3);


		////Logging
		//std::string x  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( pRawLpCalibObj->GetTimeMsg() ); 
		//std::string y  = CEMSConversionUtil::ConvertToHexStringA( pRawLpCalibObj->GetBcnId() );

		//ULONG ulRecId = 0;

		//FILE* lpTraceSatFile = CHGTFileLogger::GetInstance()->GetTraceFile();

		//fprintf(lpTraceSatFile, "%d, %s, %s, %d, %d, %d, %6.2f, %6.2f\n", ulRecId, x.c_str(), y.c_str(),
		//	pRawLpCalibObj->GetLutId(), pRawLpCalibObj->GetAntennaId(), ulSat, lsCalibData.dFoaBias, lsCalibData.dToaBias );
		//fflush(lpTraceSatFile);


	}
	else
		hr = EMS_BAD_PARAM;

	return hr;
}

EMS_RESULT 
CHGTCalibrationObj::_GetDBFSatellite(CEMSRawLpCalibObj* pRawLpCalibObj, ULONG ulMinSat, ULONG ulMaxSat, HGTSASATData &strSaSatData)
{
	EMS_RESULT hr = EMS_OK;

	CEMSRawLpCalibObj pInitialLpCalibObj;
	pInitialLpCalibObj.Set(*pRawLpCalibObj);

	ULONG ulFirstSatId    = pRawLpCalibObj->GetSatId();
	ULONG ulBestSatellite = ulFirstSatId;
	ULONG ulBestSatellite2 = ulFirstSatId;

	double dFOAinitial = pRawLpCalibObj->GetFrequency();

	double dFreqOffsetInitial = pRawLpCalibObj->GetFreqOffset();
	EMSTIME TOAinital = pRawLpCalibObj->GetTimeMsg();
	INT64 i64TimeOffsetInitial = pRawLpCalibObj->GetTimeOffset();
	double dCNR = pRawLpCalibObj->GetCarrierPower();


	double dTestFrqResidual = 0.0;
	double dTestTimeResidual = 0.0;
	double dMinToaResidual = 0.0;
	double dMinFoaResidual = 10000.0;
	double dMinFoaResidual2 = 10000.0;
	double dElevation = 0.0;
	double dBestSatElevation = 0.0;
	double dBestSatAzimuth = 0.0;
	double dDopplerUL = 0.0;
	double dDopplerDL = 0.0;
	double dFOAbias = 0.0;
	double dTOAbias = 0.0;
	EMSLOCATION	site = m_lsCalibData.siteLocation;


	WORD wAntID = pRawLpCalibObj->GetAntennaId();
	//WORD wBeamID = wAntID % 10;
	ULONG ulBeamID = (ULONG) wAntID;

	HGTLSCALIBDATA lsCalibData;

	// Hysteresis: the satellite this beam was already locked to, so a later
	// ambiguous pick can prefer staying put over chattering to whichever
	// candidate is fractionally smaller on this one sample. 0 means "never
	// locked yet" (see GetDBFBestSatellite's non-ref-beacon branch, which
	// treats 0 the same way).
	ULONG  ulIncumbentSatellite  = m_ulArrBeamId[ulBeamID];
	bool   bIncumbentEvaluated   = false;
	double dIncumbentFoaResidual = 0.0;
	double dIncumbentToaResidual = 0.0;

	// Best candidate by FOA residual alone, ignoring the |TOA| < 0.02 gate that
	// the main selection below also requires. When no candidate clears that TOA
	// gate, dMinFoaResidual stays at its 10000.0 sentinel and ulBestSatellite
	// stays at the incoming SatId, which used to mean an unconditional reject
	// of the whole detection - 3184 of them in the 2026-07-29..08-04 capture,
	// ~23% of every rejection. Falling back to this pair keeps such a detection
	// usable for bias correction instead of discarding it outright.
	ULONG  ulBestSatelliteFoaOnly  = ulFirstSatId;
	double dBestFoaResidualFoaOnly = 0.0;
	double dBestToaResidualFoaOnly = 0.0;
	double dBestAzimuthFoaOnly     = 0.0;
	double dBestElevationFoaOnly   = 0.0;
	bool   bToaGatePassed          = false;

	// Whether the winning candidate needed the 0.22 s bit-phase correction to
	// clear the TOA gate. See the bit-phase guard in the accept/reject decision.
	bool   bBestUsedBitPhase       = false;

	// How many candidates actually produced a residual to judge. One means the
	// "best" was never compared against anything - see the single-survivor guard
	// in the accept/reject decision below.
	ULONG  ulCandidatesEvaluated   = 0;


	for(ULONG ulSat = ulMinSat; ulSat <= ulMaxSat ; ulSat++)
	{
		// Which stage rejected this candidate, for the REJECTED log line below -
		// added specifically so "did the restored elevation floor actually
		// change anything" is a directly observable count instead of an
		// inference from an aggregate rejection ratio.
		const char* pszRejectReason = "unknown";

		// Reset original field data
		pRawLpCalibObj->SetFrequency(dFOAinitial);
		pRawLpCalibObj->SetFreqOffset(dFreqOffsetInitial);
		pRawLpCalibObj->SetTimeMsg(TOAinital);
		pRawLpCalibObj->SetTimeOffset(i64TimeOffsetInitial);
		pRawLpCalibObj->SetSatId(ulSat);

		hr = _OrbitUpdate(pRawLpCalibObj);
		if( hr != EMS_OK )
			pszRejectReason = "orbit update";

		dElevation = 0.0;
		if(hr == EMS_OK )
		{
			hr = _SetGSAzimuthElevation(pRawLpCalibObj);
			if( hr != EMS_OK )
				pszRejectReason = "azimuth/elevation calc";
			dElevation = pRawLpCalibObj->GetElevation();

			// Elevation floor. c_dMinElevation (10 deg) is the calibration code's
			// own number and is unrelated to this plate; when a plate config is
			// loaded its configured minimum is used instead, which matches the
			// plate system and stops real low-elevation traffic being clipped.
			const CDBFPlateConfig* pPlate = CDBFPlateConfig::instance();
			double dMinElevation = pPlate->GetEffectiveMinElevation( c_dMinElevation );

			if(dElevation < dMinElevation)
			{
				hr = EMS_FALSE;
				pszRejectReason = "below elevation floor";
			}
			else if( !pPlate->IsWithinFieldOfView( pRawLpCalibObj->GetAzimuth() ) )
			{
				// The candidate is above the horizon but outside the azimuth sector
				// the plate can actually see, so it cannot be the source of this
				// detection - prune it from the sweep rather than letting it compete
				// on FOA residual alone. Removing physically impossible candidates
				// also takes pressure off the ambiguity check downstream, which then
				// only has to arbitrate between satellites the plate could hear.
				hr = EMS_FALSE;
				pszRejectReason = "outside plate azimuth sector";
			}
		}

		if(hr == EMS_OK )
		{
			hr = _CorrectDownLink(pRawLpCalibObj);
			if( hr != EMS_OK )
				pszRejectReason = "downlink correction";
		}

		if(hr == EMS_OK )
		{
			memset(&lsCalibData, 0, sizeof(HGTLSCALIBDATA));
			CHGTCalibrationObjectsContainer::instance()->GetCalibrationParameters(
																				pRawLpCalibObj->GetLutId(),
																				pRawLpCalibObj->GetAntennaId(),
																				ulSat, &lsCalibData);
			hr = _AdjustDBFSarrFreqAndTime( pRawLpCalibObj, lsCalibData, ulSat );
			if( hr != EMS_OK )
				pszRejectReason = "DBF SARR freq/time adjust";
		}

		if( hr == EMS_OK )
		{
			EMSRANGERATED rangeRateUL;
			hr = CEMSRangeRate::Calculate( pRawLpCalibObj->GetBcnVector(),
											&pRawLpCalibObj->GetSatPVC(), 1, &rangeRateUL );
			if( hr != EMS_OK )
				pszRejectReason = "range-rate calculation";

			hr = _CalcTimeResidual( rangeRateUL, pRawLpCalibObj );
			if( hr != EMS_OK )
				pszRejectReason = "time residual calc";

			hr = _CalcFreqResidual( rangeRateUL, pRawLpCalibObj );
			if( hr != EMS_OK )
				pszRejectReason = "freq residual calc";

			dTestFrqResidual = pRawLpCalibObj->GetResidualFrequency();

			dTestTimeResidual = pRawLpCalibObj->GetResidualTime();

			if( (ulSat == ulIncumbentSatellite) && (ulIncumbentSatellite != 0) )
			{
				dIncumbentFoaResidual = dTestFrqResidual;
				dIncumbentToaResidual = dTestTimeResidual;
				bIncumbentEvaluated = true;
			}

			// Reference-beacon bit-phase correction: the 24th bit is assumed to
			// fall at the 220 ms mark, so a residual sitting near 0.22 s is read
			// as bit-phase rather than range error. Note it only ever fires on the
			// positive side.
			//
			// Remember when it fires. Once the TOA gate tightened to 2 ms this
			// became the only way a grossly wrong candidate could still look
			// on-time: subtracting 0.22 turns a 220 ms residual (~66000 km of
			// range error) into ~0.0001 s, which clears the gate outright. In the
			// 2026-08-06 capture all four surviving bad detections had exactly
			// this signature - TOA 0.2192..0.2215 with FOA residuals of 1099 to
			// 6005 Hz - while the four legitimate near-0.22 detections were on the
			// negative side, never corrected, and carried FOA residuals under
			// 100 Hz. The correction itself is the discriminator.
			bool bBitPhaseApplied = false;
			if ( (dTestTimeResidual > 0.2) &&  (dTestTimeResidual < 0.24) )
			{
				dTestTimeResidual -= 0.22;
				bBitPhaseApplied = true;
			}

			// Tracked independently of the TOA gate below, so there is always a
			// best-by-FOA candidate to fall back on. See the declaration.
			if( (0 == ulCandidatesEvaluated) || (fabs(dTestFrqResidual) < fabs(dBestFoaResidualFoaOnly)) )
			{
				ulBestSatelliteFoaOnly  = ulSat;
				dBestFoaResidualFoaOnly = dTestFrqResidual;
				dBestToaResidualFoaOnly = pRawLpCalibObj->GetResidualTime();
				dBestAzimuthFoaOnly     = pRawLpCalibObj->GetAzimuth();
				dBestElevationFoaOnly   = dElevation;
			}

			ulCandidatesEvaluated++;

			//if( (fabs(dTestFrqResidual) < fabs(dMinFoaResidual)) && (fabs(dTestTimeResidual) < 0.005 ) )
			if( (fabs(dTestFrqResidual) < fabs(dMinFoaResidual)) && (fabs(dTestTimeResidual) < c_dMaxToaResidualSec ) )
			{
				bToaGatePassed = true;
				bBestUsedBitPhase = bBitPhaseApplied;
				dMinFoaResidual2 = dMinFoaResidual;
				ulBestSatellite2 = ulBestSatellite;
				dMinFoaResidual = dTestFrqResidual;
				dMinToaResidual = pRawLpCalibObj->GetResidualTime();
				ulBestSatellite = ulSat;
				dDopplerUL = m_dDopplerShiftUL;
				dDopplerDL = m_dDopplerShiftDL;
				dBestSatElevation = dElevation;
				dBestSatAzimuth = pRawLpCalibObj->GetAzimuth();
				site = m_lsCalibData.siteLocation;
				dFOAbias = lsCalibData.dFoaBias;
				dTOAbias = lsCalibData.dToaBias;
			}
			else if( fabs(dTestFrqResidual) < fabs(dMinFoaResidual2) )
			{
				dMinFoaResidual2 = dTestFrqResidual;
				ulBestSatellite2 = ulSat;
			}
		}

		if( hr == EMS_OK )
		{
			std::string x  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( pRawLpCalibObj->GetTimeMsg() ); 
			std::string y  = CEMSConversionUtil::ConvertToHexStringA( pRawLpCalibObj->GetBcnId() );
			std::string z  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( strSaSatData.timeLastUpdate );

			ULONG ulRecId = 1;

			FILE* lpTraceSatFile = CHGTFileLogger::GetInstance()->GetTraceFile();

			double dSAangle = 0.0;

			for( ULONG i = 0; i < strSaSatData.ulSAcount; i++ )
			{
				if ( strSaSatData.ulSAsat1[i] == ulBestSatellite )
				{
					dSAangle = strSaSatData.dSAangle[i];
				}
			}

			fprintf(lpTraceSatFile, "%d, %s, %s, %d, %d, %d, %d, %5.2f, %6.2f, %10.3f, %7.2f, %7.1f, %10.6f, %7.2f, %10.6f, %7.2f, %s\n",
				ulRecId, x.c_str(), y.c_str(),pRawLpCalibObj->GetLutId(), wAntID, ulBeamID, ulSat, dCNR, dElevation,
				dFOAinitial, dTestFrqResidual, dMinFoaResidual, dMinToaResidual, dFOAbias, dTOAbias, dSAangle, z.c_str());

			fflush(lpTraceSatFile);
		}
		else
		{
			// Candidate satellite rejected somewhere in the chain above -
			// pszRejectReason names the specific stage so a bad candidate can
			// be told apart from a bad site/config, and so the elevation
			// floor's actual effect (c_dMinElevation) is directly countable
			// via reason="below elevation floor" instead of inferred from an
			// aggregate rejection count.
			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: candidate SatId=%lu REJECTED hr=0x%08lX reason=%s",
				ulSat, (unsigned long)hr, pszRejectReason);
		}
	}

	// Reestabish initial values
	pRawLpCalibObj->Set(pInitialLpCalibObj);  //Smruti - 3rd NOV

	// No candidate cleared the |TOA| < 0.02 gate, so the selection above never
	// ran and its outputs are still at their sentinels. Use the best-by-FOA
	// candidate rather than rejecting the detection: a poor TOA residual is a
	// reason to distrust the *time* calibration, not a reason to throw away the
	// frequency information as well - and _CalculateOffsets clamps whatever it
	// receives to +/- the configured threshold before applying gain, so a bad
	// value can only ever move the bias by threshold*gain.
	bool bToaGateFellBack = false;
	if( !bToaGatePassed && (ulCandidatesEvaluated > 0) )
	{
		bToaGateFellBack = true;
		ulBestSatellite  = ulBestSatelliteFoaOnly;
		dMinFoaResidual  = dBestFoaResidualFoaOnly;
		dMinToaResidual  = dBestToaResidualFoaOnly;

		// Also carry the geometry across. Without this, dBestSatAzimuth /
		// dBestSatElevation stay at 0,0 on the fallback path and the AzOffset
		// reported below is measured from due north at the horizon rather than
		// from the satellite actually chosen - which made 194 of 815 RESULT
		// lines in the 2026-08-05 capture look far off-sector when they were not.
		dBestSatAzimuth   = dBestAzimuthFoaOnly;
		dBestSatElevation = dBestElevationFoaOnly;

		CTSiDebugTrace::LogFmtAlways(
			"_GetDBFSatellite: TOA-GATE FALLBACK BcnId=%016I64X AntId=%u - no candidate had "
			"|TOA residual| < 0.02s; using best-by-FOA SatId=%lu (%.3f Hz, TOA %.6f s)",
			pRawLpCalibObj->GetBcnId(), (unsigned)wAntID,
			ulBestSatelliteFoaOnly, dBestFoaResidualFoaOnly, dBestToaResidualFoaOnly);
	}

	// Ambiguity/hysteresis resolution: the sweep above picks whichever
	// candidate has the smallest FOA residual with no regard for how much
	// better it is than the runner-up. When the top two are within
	// c_dFoaAmbiguityMarginHz of each other, treat this as genuinely
	// ambiguous - noise on this one sample could easily have picked the
	// other one. Prefer staying with the beam's already-locked satellite if
	// it's still in contention; otherwise reject the detection outright
	// rather than risk crediting the wrong satellite's orbit with this
	// beacon's residual (see the 2026-07-27/28 200kHz/1s buffer captures,
	// which show exactly this - the same beacon's detections genuinely
	// flip-flopping between two satellite IDs, including several at the
	// same timestamp).
	ULONG  ulFinalSatellite  = ulBestSatellite;
	double dFinalFoaResidual = dMinFoaResidual;
	double dFinalToaResidual = dMinToaResidual;
	bool   bAmbiguousReject  = false;

	bool bTooClose = (ulBestSatellite2 != ulBestSatellite) &&
	                  (fabs(dMinFoaResidual2 - dMinFoaResidual) < c_dFoaAmbiguityMarginHz);

	if( bTooClose )
	{
		if( bIncumbentEvaluated && (fabs(dIncumbentFoaResidual - dMinFoaResidual) < c_dFoaAmbiguityMarginHz) )
		{
			ulFinalSatellite  = ulIncumbentSatellite;
			dFinalFoaResidual = dIncumbentFoaResidual;
			dFinalToaResidual = dIncumbentToaResidual;

			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: HYSTERESIS BcnId=%016I64X AntId=%u - top two candidates "
				"SatId=%lu (%.3f Hz) and SatId=%lu (%.3f Hz) are within %.1f Hz of each other; "
				"staying with already-locked SatId=%lu (%.3f Hz) instead of switching",
				pRawLpCalibObj->GetBcnId(), (unsigned)wAntID,
				ulBestSatellite, dMinFoaResidual, ulBestSatellite2, dMinFoaResidual2,
				c_dFoaAmbiguityMarginHz, ulIncumbentSatellite, dIncumbentFoaResidual);
		}
		else
		{
			bAmbiguousReject = true;

			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: AMBIGUOUS BcnId=%016I64X AntId=%u - top two candidates "
				"SatId=%lu (%.3f Hz) and SatId=%lu (%.3f Hz) are within %.1f Hz of each other "
				"with no usable incumbent to fall back on - rejecting detection rather than guess",
				pRawLpCalibObj->GetBcnId(), (unsigned)wAntID,
				ulBestSatellite, dMinFoaResidual, ulBestSatellite2, dMinFoaResidual2,
				c_dFoaAmbiguityMarginHz);
		}
	}

	// Accept/reject. An oversized FOA residual is deliberately NOT a rejection:
	// a reference beacon detection whose residual exceeds c_dMinFoaResidual is
	// exactly the case that most needs to reach _CalculateOffsets, because that
	// is the evidence the stored bias has drifted. Rejecting it instead (the
	// behaviour before this change) was self-sustaining - the gate is 200 Hz
	// and the accumulated FOA biases in lscalibdata.csv had reached -134 to
	// -1478 Hz, so every detection came back over the gate, was discarded, and
	// the bias that caused the overshoot could never be corrected. In the
	// 2026-07-29..08-04 capture that discarded 74.9% of all reference-beacon
	// detections and froze every lscalibdata.csv row after 2026/211 18:01.
	//
	// _CalculateOffsets clamps the residual to +/- the configured threshold
	// before applying gain, so an outlier can still only move the bias by
	// threshold*gain (200 Hz * 0.05 = 10 Hz) per detection. That clamp, not
	// this gate, is what bounds the damage from a bad residual.
	//
	// Genuine satellite ambiguity IS still a rejection: if the sweep cannot say
	// which satellite this was, crediting the residual to a specific
	// lscalibdata.csv row would corrupt whichever one it guessed.
	//
	// So is "no candidate survived the sweep at all" - every one was rejected on
	// orbit update, elevation, or the plate azimuth sector, so not one produced a
	// residual to judge. ulBestSatellite is then still the provisional incoming
	// SatId and dMinFoaResidual still its 10000.0 sentinel; accepting that would
	// wave an entirely unvetted detection through under a satellite ID nothing
	// ever checked. The old code caught this by accident, because the sentinel
	// failed the c_dMinFoaResidual test that used to gate acceptance - removing
	// that gate means it now has to be caught deliberately.
	// Single-survivor guard. Dropping the FOA gate above is safe only when the
	// winner actually beat something: the sweep's confidence comes from having
	// compared candidates, and the ambiguity check needs two of them to fire at
	// all. With exactly one candidate evaluated, nothing was compared and nothing
	// can be ambiguous, so an arbitrarily bad residual is accepted unopposed.
	//
	// That is precisely how satellite 433 was being credited with 6.5 kHz
	// frequency residuals in the 2026-08-05 capture: over-tight geometric
	// pruning left it as the only survivor in 16.6% of detections (up from 0.6%),
	// and it then won by default. So when the winner is unopposed, require it to
	// clear c_dMinFoaResidual after all - the gate is reinstated exactly where
	// there is no corroboration, and stays off where there is.
	bool bUnopposedReject = ( 1 == ulCandidatesEvaluated ) &&
	                        ( fabs(dFinalFoaResidual) >= c_dMinFoaResidual );

	// Bit-phase guard, same principle as the single-survivor guard above: where
	// the TOA evidence is weaker, require the frequency to corroborate it. A
	// winner that only cleared the TOA gate because 0.22 s was subtracted from
	// its residual has not actually demonstrated a range match - the raw
	// residual is still 220 ms - so it does not get the benefit of the dropped
	// FOA gate. A genuine bit-phase case pairs the 0.22 s offset with a small
	// frequency residual and still passes; a misidentified satellite pairs it
	// with kHz and does not. Only applies when the bit-phase branch actually
	// fired, so detections whose TOA is legitimately small are untouched.
	bool bBitPhaseReject = bToaGatePassed && bBestUsedBitPhase && !bToaGateFellBack &&
	                       ( fabs(dFinalFoaResidual) >= c_dMinFoaResidual );

	if( bAmbiguousReject || bUnopposedReject || bBitPhaseReject || (0 == ulCandidatesEvaluated) )
	{
		hr = EMS_FALSE;

		if( 0 == ulCandidatesEvaluated )
		{
			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: NO CANDIDATES BcnId=%016I64X AntId=%u - every satellite in "
				"%lu..%lu was rejected before a residual could be computed; discarding detection",
				pRawLpCalibObj->GetBcnId(), (unsigned)wAntID, ulMinSat, ulMaxSat);
		}
		else if( bUnopposedReject )
		{
			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: UNOPPOSED BcnId=%016I64X AntId=%u - SatId=%lu was the only "
				"candidate to survive the sweep and its FOA residual %.3f Hz exceeds %.1f Hz; "
				"nothing corroborates it, so discarding rather than crediting it by default",
				pRawLpCalibObj->GetBcnId(), (unsigned)wAntID, ulFinalSatellite,
				dFinalFoaResidual, c_dMinFoaResidual);
		}
		else if( bBitPhaseReject )
		{
			CTSiDebugTrace::LogFmtAlways(
				"_GetDBFSatellite: BIT-PHASE BcnId=%016I64X AntId=%u - SatId=%lu only cleared the "
				"TOA gate after the 0.22s bit-phase correction (raw TOA residual %.6f s) and its "
				"FOA residual %.3f Hz exceeds %.1f Hz; frequency does not corroborate the range, "
				"so discarding",
				pRawLpCalibObj->GetBcnId(), (unsigned)wAntID, ulFinalSatellite,
				dFinalToaResidual, dFinalFoaResidual, c_dMinFoaResidual);
		}
	}
	else
	{
		hr = EMS_OK;
		pRawLpCalibObj->SetSatId(ulFinalSatellite);

		// Beam lock still requires a residual inside the gate (and adequate
		// CNR). Which satellite a beam is tracking is a confidence judgement,
		// and an out-of-gate residual is precisely when that judgement is least
		// trustworthy - so an over-threshold detection updates the bias but is
		// not allowed to re-point the beam's satellite association.
		if( (fabs(dFinalFoaResidual) < c_dMinFoaResidual) && !bToaGateFellBack &&
			(dCNR > c_dDefaultCNRFilterThreshold) )
		{
			if(m_ulArrBeamId[ulBeamID] != ulFinalSatellite)
			{
				m_ulArrBeamId[ulBeamID] = ulFinalSatellite;
			}
		}
	}

	// Overall accept/reject decision for this detection: EMS_FALSE only when the
	// satellite was genuinely ambiguous, EMS_OK otherwise. Same frequency as
	// "REF BEACON HIT" since this only runs when lpRefBeaconData is set, so this
	// stays low-volume.
	//
	// OverGate/ToaFallback are reported so the two cases that used to be silent
	// rejections stay countable now that they are accepted - "how often is the
	// bias being driven by an out-of-gate residual" is the number to watch while
	// the stored biases converge back down.
	//
	// RunnerUp/Margin are logged unconditionally (not just when AMBIGUOUS or
	// HYSTERESIS fires) so c_dFoaAmbiguityMarginHz can actually be tuned from
	// data: without this, every detection that *doesn't* trigger the margin
	// check is a blind spot - there's no way to tell "no candidates were ever
	// close" from "candidates were close but just outside the margin".
	double dRawMargin = fabs(dMinFoaResidual2 - dMinFoaResidual);
	CTSiDebugTrace::LogFmtAlways(
		"_GetDBFSatellite: RESULT hr=0x%08lX BestSatId=%lu dMinFoaResidual=%.3f dMinToaResidual=%.6f "
		"RunnerUpSatId=%lu RunnerUpFoaResidual=%.3f Margin=%.3f OverGate=%s ToaFallback=%s "
		"BestAz=%.2f BestEl=%.2f AzOffset=%+.2f Candidates=%lu",
		(unsigned long)hr, ulFinalSatellite, dFinalFoaResidual, dFinalToaResidual,
		ulBestSatellite2, dMinFoaResidual2, dRawMargin,
		(fabs(dFinalFoaResidual) < c_dMinFoaResidual) ? "NO" : "YES",
		bToaGateFellBack ? "YES" : "NO",
		dBestSatAzimuth, dBestSatElevation,
		CDBFPlateConfig::instance()->AzimuthOffset( dBestSatAzimuth ),
		ulCandidatesEvaluated);

	//if ( m_lpTraceSatFile )
	{
		std::string x  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( pRawLpCalibObj->GetTimeMsg() ); 
		std::string y  = CEMSConversionUtil::ConvertToHexStringA( pRawLpCalibObj->GetBcnId() );

		//std::cout<<"SatId "<<ulSatID<<"ulSAcount "<<strSaSatData.ulSAcount<<std::endl;
		ULONG ulSatID = m_ulArrBeamId[ulBeamID];
		double dSA    = 0.0;

		for( ULONG i = 0; i < strSaSatData.ulSAcount; i++ )
		{
			if ( ulSatID == strSaSatData.ulSAsat1[i] )
			{
				dSA = strSaSatData.dSAangle[i];	
				break;
			}
		}
		ULONG ulRecId = 2;
		FILE* lpTraceSatFile = CHGTFileLogger::GetInstance()->GetTraceFile();

		// dMinToaResidual appended at the end: the winning candidate's time
		// residual (seconds), i.e. what actually passed the dTestTimeResidual
		// gate in the loop above - lets us see the real accepted-match
		// distribution instead of guessing at a threshold value.
		fprintf(lpTraceSatFile,"%d, %s, %s, %d, %d, %d, %d, %f, %d, %d, %d, %8.f, %8.3f, %10.3f, %d, %f,  %f, %f, %f, %f, %f, %f, %10.6f\n",
			ulRecId, x.c_str(), y.c_str(), pRawLpCalibObj->GetLutId(),
			wAntID, ulBeamID, m_ulArrBeamId[ulBeamID], dCNR, ulFirstSatId, ulBestSatellite2, ulBestSatellite, dMinFoaResidual2, dMinFoaResidual, dFOAinitial, hr,
			dDopplerUL, dDopplerDL, dBestSatElevation, dSA, site.degLatitude, site.degLongitude, site.fAltitude, dMinToaResidual );
		fflush( lpTraceSatFile );



	}


	return hr;
}

CEMSRawLpCalibObj* 
CHGTCalibrationObj::GetDBFBestSatellite(CEMSRawSpCalibObj*  pRawSpCalibObj, 
								LPEMSREFBEACONDATAEX lpRefBeaconData, HGTSASATData &strSaSatData)
{
	EMS_RESULT hr = EMS_OK;
	CEMSRawLpCalibObj *pRet = NULL; 

	try
	{

		pRet = _CreateRawLPCalibObject(pRawSpCalibObj, lpRefBeaconData);

		//Rich subroutine for DBF
		WORD wAntId = pRet->GetAntennaId();

		ULONG ulBeamId = (ULONG)(wAntId);

		double dMinSatSA;

		ULONG ulSat1, ulSat2;
		ULONG ulSat = pRet->GetSatId();

		if( ulSat > 400 && ulSat < 500)
		{
			ulSat1 = 401;
			ulSat2 = 436;
		}
		else if( ulSat > 600 && ulSat < 700)
		{
			ulSat1 = 630;
			ulSat2 = 660;
		}

		if(lpRefBeaconData)
		{
			double dFOAinitial = pRet->GetFrequency() - c_dFreqDBFoffset;
			pRet->SetFrequency(dFOAinitial);
				
			hr = _GetDBFSatellite(pRet, ulSat1, ulSat2, strSaSatData);
			//_GetDBFSatellite(pRet, ulSat1, ulSat2, strSaSatData);

			// RR - Feb.2, 2022 : If reference beacon FOA residual too large, do not use ths detection
			//
			// Amended: _GetDBFSatellite now returns EMS_FALSE only when the
			// satellite is genuinely ambiguous, not merely when the FOA
			// residual is large - an over-threshold residual is the signal
			// that the stored bias needs correcting, so the detection is kept
			// and allowed through to _CalculateOffsets, which clamps it.
			// Dropping the object here still means "skipping
			// CalibrateSpRawObject" in CHGTChannelCalibrationObj, which
			// suppresses the LP record, the bias update and the SARR FCAL
			// record all at once - so it must stay reserved for the case where
			// we genuinely cannot attribute the detection to a satellite.
			if( EMS_FALSE == hr )
			{
				pRet->Release();
				pRet = NULL;
			}

		}
		else
		{
			if( m_ulArrBeamId[ulBeamId] > 0)
				pRet->SetSatId(m_ulArrBeamId[ulBeamId]);
		}

		// Check Satellite Separation Angle
		if (pRet)
		{
			ULONG ulSatID = pRet->GetSatId();
			ULONG ulLutID = pRet->GetLutId();

			//std::cout<<"LutId "<<ulLutID<<" SatId "<<ulSatID<<" ulSAcount "<<strSaSatData.ulSAcount<<std::endl;
			for( ULONG i = 0; i < strSaSatData.ulSAcount; i++ )
			{
				if ( (ulSatID == strSaSatData.ulSAsat1[i]) )
				{
					pRet->SetDuplicateMsgCount( (WORD)  (strSaSatData.dSAangle[i]*10) );
					std::cout<<"SatId "<<ulSatID<<" SAangle "<<strSaSatData.dSAangle[i]<<std::endl;
					
					//if ( strSaSatData.dSAangle[i] < c_dSAthreshold )
					//{
					//	pRet->SetType(999);
					//}
					break;
				}
			}
		}

	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in GetDBFBestSatellite - returning NULL ***");
		if(pRet)
		{
			pRet->Release();
			pRet = NULL;
		}

		return pRet;
	}

	return pRet;
}

CEMSRawLpCalibObj*
CHGTCalibrationObj::CalibrateSpRawObject(CEMSRawSpCalibObj*  pRawSpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData)
{
	EMS_RESULT hr = EMS_OK;
	m_bSpSarrUpdated = false;
	CEMSRawLpCalibObj *pRet = NULL; 

	try
	{
		// Per-step hr tracing, limited to reference-beacon hits only (the rare,
		// diagnostically useful case) so this doesn't flood the trace with every
		// non-reference message processed.
		bool bTraceThis = (lpRefBeaconData != NULL);

		pRet = _CreateRawLPCalibObject(pRawSpCalibObj, lpRefBeaconData);

		if(bTraceThis)
			CTSiDebugTrace::LogFmtAlways("CalibrateSpRawObject: _CreateRawLPCalibObject -> pRet=%p", pRet);

		if(hr == EMS_OK )
			hr = _OrbitUpdate(pRet);

		if(bTraceThis)
			CTSiDebugTrace::LogHRAlways("CalibrateSpRawObject: _OrbitUpdate", hr);

		if(hr == EMS_OK )
			hr = _SetGSAzimuthElevation(pRet);

		if(bTraceThis)
			CTSiDebugTrace::LogHRAlways("CalibrateSpRawObject: _SetGSAzimuthElevation", hr);

		if(hr == EMS_OK )
			hr = _CorrectDownLink(pRet);

		if(bTraceThis)
			CTSiDebugTrace::LogHRAlways("CalibrateSpRawObject: _CorrectDownLink", hr);

		if(hr == EMS_OK)
			hr = _ApplyTOAFOAAdjustments(pRet);

		if(bTraceThis)
			CTSiDebugTrace::LogHRAlways("CalibrateSpRawObject: _ApplyTOAFOAAdjustments", hr);

		if( (lpRefBeaconData != NULL) && (hr == EMS_OK) )
		{
			hr =  _ComputeResidual(pRet);

			if(bTraceThis)
				CTSiDebugTrace::LogHRAlways("CalibrateSpRawObject: _ComputeResidual", hr);
				// RR test

				std::string x  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( pRet->GetTimeMsg() ); 
				std::string y  = CEMSConversionUtil::ConvertToHexStringA( pRet->GetBcnId() );

				FILE* lpTraceSatFile = CHGTFileLogger::GetInstance()->GetTraceFile();

				fprintf(lpTraceSatFile, "4, %s, %s, %I64d, %d, %d, %d,  %10.3f, %7.3f, %7.3f, %10.6f, %7.3f, %10.6f\n",
					x.c_str(), y.c_str(), pRet->GetTimeMsg(),
					pRet->GetLutId(), pRet->GetAntennaId(),  pRet->GetSatId(),
					pRet->GetFrequency(), pRet->GetCarrierPower(),
					m_lsCalibData.dFoaBias, m_lsCalibData.dToaBias,
					pRet->GetResidualFrequency(), pRet->GetResidualTime() );
				fflush(lpTraceSatFile);


		}

		if(hr != EMS_OK)
		{
			pRet->Release();
			pRet = NULL;
		}
	
	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in CalibrateSpRawObject - returning NULL ***");
		if(pRet)
		{
			pRet->Release();
			pRet = NULL;
		}
		return pRet;
	}

	return pRet;
}


CEMSRawLpCalibObj*
CHGTCalibrationObj::CalibrateLpRawObject(CEMSRawLpCalibObj*  pRawLpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData)
{
	EMS_RESULT hr = EMS_FALSE;
	m_bToaFoaSarrUpdated = false;
	CEMSRawLpCalibObj *pRet = NULL; 

	try
	{
		if(pRawLpCalibObj)
			pRet = new CEMSRawLpCalibObj(*pRawLpCalibObj->GetRawData());
		
		hr = _SetInitialFlags( pRet, lpRefBeaconData);

		if(hr == EMS_OK)
			//hr = _OrbitUpdate(pRet); //Assume the toafoa records orbital vector

		hr = _SetGSAzimuthElevation(pRet);

		if(hr == EMS_OK)
			hr = _ApplyTOAFOAAdjustments(pRet);

		if(hr == EMS_OK)
			hr = _DataValidation(pRet);

		if( (lpRefBeaconData != NULL) && (hr == EMS_OK) )
			hr =  _ComputeTOAFOAResidual(pRet);

	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in CalibrateLpRawObject - returning NULL ***");
		if(pRet)
		{
			pRet->Release();
			pRet = NULL;
		}
		return pRet;
	}

	return pRet;
}

//EMS_RESULT
//CHGTCalibrationObj::_MovingBeacon(CEMSRawLpCalibObj*  pRawLpCalibObj)
//{
//	EMS_RESULT hr = EMS_OK;
//
//	if( m_bMovingBeaconFirstTime )
//	{
//		// Read Moving Beacon Parameters
//		m_bMovingBeaconFirstTime = false;
//		m_i64MatchBeaconID = CEMSConversionUtil::ConvertHexStringToUInt64( "A79EEE26E32E190" );
//		m_i64MatchBeaconID = CEMSConversionUtil::ConvertHexStringToUInt64( "ADC389B8AA1B9D0" );
//		char cMovingBeaconmsg[36];
//		memcpy( &cMovingBeaconmsg, "FFFED056E1C4DC550DCE801D130F00000000", 36 );
//
//		CEMSConversionUtil::ConvertToByteArray( &cMovingBeaconmsg,18, &m_bMovingBeaconMsg );
//
//		m_dSpeed = 5.0/1000.0;   // km/sec
//		m_dHeading = 0.0; // degrees clockwise from north
//
//		FILE* fpInput = fopen( "d:\\MovingBeacon.csv", "rt" );
//
//		if ( fpInput )
//		{
//
//		}
//		else
//			return;
//	}
//
//	if ( m_i64MatchBeaconID == pRawLpCalibObj->GetBcnId() )
//	{
//
//		EMSRANGERATED rangeRateUL;
//		EMSRANGERATED rangeRateULMB;
//		EMSTIMECOORDD vSatellite;
//		EMSTIMECOORDD vSatelliteMB;
//		EMSVECTORD vBeacon;
//		EMSVECTORD vMBeacon;
//		EMSVECTORD vXaxis;
//		double dTransmitFrequency;
//		double dFrequency;
//		double dResidual;
//		double dDopplerUL;
//		double dMBdopplerUL;
//		double dFreqRes;
//
//		vXaxis.dX = 1.0;
//		vXaxis.dY = 0.0;
//		vXaxis.dZ = 0.0;
//
//		vSatellite = 	pRawLpCalibObj->GetSatPVC();
//		vBeacon       = pRawLpCalibObj->GetBcnVector();
//		hr = CEMSRangeRate::Calculate( vBeacon, &vSatellite, 1, &rangeRateUL );
//
//		vSatelliteMB = vSatellite;
//		vMBeacon  = CEMSVector::Normalize( vBeacon );
//		vMBeacon  = CEMSVector::CrossProduct( vBeacon, vXaxis );
//		vMBeacon  = CEMSVector::Normalize( vMBeacon );
//
//		vSatelliteMB.coord.velocity.dX -= vMBeacon.dX * m_dSpeed * cos(m_dHeading);
//		vSatelliteMB.coord.velocity.dY -= vMBeacon.dY * m_dSpeed * sin(m_dHeading);
//
//		hr = CEMSRangeRate::Calculate( vBeacon, &vSatelliteMB, 1, &rangeRateULMB );
//
//		dTransmitFrequency = pRawLpCalibObj->GetTxFrequency();
//		dFrequency = pRawLpCalibObj->GetFrequency();
//		dResidual  = pRawLpCalibObj->GetResidualFrequency();
//		
//		dMBdopplerUL = dFrequency * ( rangeRateULMB.dRangeRate / rangeRateULMB.dRange ) / c_dVelocityOfLight;
//		dDopplerUL   = dFrequency * ( rangeRateUL.dRangeRate / rangeRateUL.dRange ) / c_dVelocityOfLight;
//
//		dFreqRes = dMBdopplerUL - dDopplerUL;
//		dFrequency += dFreqRes;
//
//
//		// Set Output
//		pRawLpCalibObj->SetBcnId( m_i64MatchBeaconID );
//		pRawLpCalibObj->SetBeaconMsg( m_bMovingBeaconMsg );
//
//		pRawLpCalibObj->SetFrequency(dFrequency);
//		pRawLpCalibObj->SetResidualFrequency( dResidual + dFreqRes );
//
//		hr = EMS_OK;
//
//	}
//	else
//	{
//		hr = EMS_FALSE;
//	}
//	return hr;
//}



