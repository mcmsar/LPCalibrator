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

#include "HGTChannelCalibrationObj.h"
#include "emsexcpt.h"
#include "EMSCLOCK.H"
#include "HGTCalibrationObjectsContainer.h"
#include "CBeaconMessage.h"
#include <iostream>
#include "convutility.h"
#include "HGTFileLogger.h"
#include "TSiDebugTrace.h"

const INT64 c_i64HistoryAgeoutNanos = 25000000000;		// 25 seconds
//const double c_dFreqDBFoffset = 11175.0; //in Hz
const double c_dFreqDBFoffset = 0.0; //in Hz

CHGTChannelCalibrationObj::CHGTChannelCalibrationObj() :  CApiObjBase( TEXT("CHGTChannelCalibrationObj") ), m_pRefBcns(NULL)
{
}

CHGTChannelCalibrationObj::CHGTChannelCalibrationObj( const CHGTChannelCalibrationObj& Obj) : CApiObjBase( TEXT("CHGTChannelCalibrationObj") ), 
																								m_pRefBcns(NULL)
{
}

CHGTChannelCalibrationObj::~CHGTChannelCalibrationObj()
{
}

EMS_RESULT 
CHGTChannelCalibrationObj::Initialize(RefBcn* pRefBcns)
{
	EMS_RESULT hr = EMS_OK;

	if(m_pRefBcns)
	{
		delete m_pRefBcns;
		m_pRefBcns = NULL;
	}

	m_pRefBcns = pRefBcns;

	return hr;
}

EMS_RESULT 
CHGTChannelCalibrationObj::Reset()
{

	return EMS_OK;
}


void
CHGTChannelCalibrationObj::AddRawSpCalibObj( CEMSRawSpCalibObj* pRawCalibObj )
{

	if( pRawCalibObj )
	{
		try
		{
			bool bDup       = _IsDuplicate(pRawCalibObj);
			bool bFrameOK   = _CheckFrameSynchBitErrors(pRawCalibObj);

			if(!bDup && bFrameOK)
			{
				BYTE* msg = pRawCalibObj->GetBeaconMsg();
				CTSiDebugTrace::LogFmt(
					"SP FILTER PASS: BcnId=%016I64X SatId=%lu AntId=%u Freq=%.3f "
					"BeaconMsg[0..3]=%02X %02X %02X %02X",
					pRawCalibObj->GetBeaconID(),
					(unsigned long)pRawCalibObj->GetSatId(),
					(unsigned)pRawCalibObj->GetAntennaId(),
					pRawCalibObj->GetFrequency(),
					msg[0], msg[1], msg[2], msg[3]);

				_SaveInputHistory(pRawCalibObj);
			}
			else
			{
				BYTE* msg = pRawCalibObj->GetBeaconMsg();
				CTSiDebugTrace::LogFmt(
					"SP FILTER REJECT: BcnId=%016I64X SatId=%lu AntId=%u "
					"FrameOK=%d Dup=%d BeaconMsg[0..3]=%02X %02X %02X %02X",
					pRawCalibObj->GetBeaconID(),
					(unsigned long)pRawCalibObj->GetSatId(),
					(unsigned)pRawCalibObj->GetAntennaId(),
					(int)bFrameOK, (int)bDup,
					msg[0], msg[1], msg[2], msg[3]);
			}
		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** EXCEPTION in AddRawSpCalibObj ***");
			throw;
		}
	}
}

bool
CHGTChannelCalibrationObj::_CheckFrameSynchBitErrors( CEMSRawSpCalibObj* pRawSpCalibObj )
{
	bool bRet = false;
	BYTE cBeaconMsg[ 18 ];
	memcpy( cBeaconMsg, pRawSpCalibObj->GetBeaconMsg(), 18*sizeof(BYTE) );

	CBeaconMessage oBcn(cBeaconMsg);

	if( cBeaconMsg[2] == 0x2f || cBeaconMsg[2] == 0xd0 )
	{
		if( ! (cBeaconMsg[1] & 1 ) )
			bRet = true;
	}

	return bRet;
}

std::string 
CHGTChannelCalibrationObj::_FormatInputTime( EMSTIME time )
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

std::string
CHGTChannelCalibrationObj::_CreateKey( CEMSRawSpCalibObj* pRawSpCalibObj )
{
	// Create the key.
	char szKey[ 128 ];

	UINT64 timeKey = (UINT64)( (double)pRawSpCalibObj->GetTimeMsg().intTime * 1e-9 ); // nearest second

	sprintf( szKey, "%I64X_%d_%d_%I64d", pRawSpCalibObj->GetBeaconID(), 
						pRawSpCalibObj->GetLutId(), pRawSpCalibObj->GetAntennaId(), timeKey);

	std::string oszKey( szKey );
	return oszKey;
}

void 
CHGTChannelCalibrationObj::AddRawLpCalibObj(CEMSRawLpCalibObj*  pRawCalibObj)
{
	if( pRawCalibObj )
	{
		try
		{
			m_lstRawLpCalib.Add( pRawCalibObj );
		}
		catch( ... )
		{
			throw;
		}
	}
}


void 
CHGTChannelCalibrationObj::PerformLpCalibration( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalibObjs,
													CEMSPointerList<CEMSSarrCalibObj>& rolstSarrCalib)
{

	CEMSRawLpCalibObj* pCalibObj = NULL;
	
	if( 0 < m_lstRawLpCalib.Count() )
	{
		// Verbose tier - mirrors "SP CALIBRATE: PerformSpCalibration called".
		// The std::cout this replaced went to a console that doesn't exist when
		// running as a service, so it was never actually visible anywhere.
		CTSiDebugTrace::LogFmt(
			"LP CALIBRATE: PerformLpCalibration called, count=%d",
			(int)m_lstRawLpCalib.Count());
		CHGTCalibrationObj* pCalibrationObj = NULL;
		CEMSRawLpCalibObj* pRawLpCalibObj = NULL;
		CEMSSarrCalibObj* pSarrCalibData = NULL;
		HGTLSCALIBDATA* pLsCalibData = NULL;

		try
		{
	
			m_lstRawLpCalib.MoveFirst();


			while( pRawLpCalibObj = m_lstRawLpCalib.GetNext() )
			{
				pCalibrationObj = CHGTCalibrationObjectsContainer::instance()->GetCalibrationObj(
																			pRawLpCalibObj->GetLutId(),
																			pRawLpCalibObj->GetAntennaId(),
																			pRawLpCalibObj->GetSatId());

				if(pCalibrationObj)
				{
					LPEMSREFBEACONDATAEX lpRefBeaconData = NULL;
					if(m_pRefBcns)
						m_pRefBcns->IsRefBcn( pRawLpCalibObj->GetBcnId(), lpRefBeaconData );

					// Mirrors "SP CALIBRATE: REF BEACON HIT" - only logs actual
					// hits (low volume) so we can tell whether the LP path is
					// finding reference beacons at all, independent of whether
					// _CalculateToaFoaOffsets goes on to update bias.
					if(lpRefBeaconData)
					{
						CTSiDebugTrace::LogFmtAlways(
							"LP CALIBRATE: REF BEACON HIT BcnId=%016I64X uFlag=0x%08lX LutId=%lu AntId=%u SatId=%lu",
							pRawLpCalibObj->GetBcnId(),
							(unsigned long)lpRefBeaconData->data.uFlag,
							pRawLpCalibObj->GetLutId(),
							(unsigned)pRawLpCalibObj->GetAntennaId(),
							pRawLpCalibObj->GetSatId());
					}

					pCalibObj = pCalibrationObj->CalibrateLpRawObject(pRawLpCalibObj, lpRefBeaconData);

					if( pCalibObj )
					{
						rolstCalibObjs.Add( pCalibObj );
						pCalibObj->Release();
						pCalibObj = NULL;
	
					}

					if(pCalibrationObj->IsToaFoaSarrUpdate())
					{
						pSarrCalibData = new CEMSSarrCalibObj();
						pCalibrationObj->GetSarrCalibData(pSarrCalibData);
						if(pSarrCalibData)
						{
							rolstSarrCalib.Add(pSarrCalibData);  //Write to FCAL table
							pSarrCalibData->Release();
							pSarrCalibData = NULL;
						}
						else
						{
							pSarrCalibData->Release();
							pSarrCalibData = NULL;
						}
					}
				}

				m_lstRawLpCalib.RemoveCurrent();
				pRawLpCalibObj->Release();
				pRawLpCalibObj = NULL;
			}
		}
		catch( ... )
		{
			if( pCalibrationObj )
			{
				pCalibrationObj->Release();
				pCalibrationObj = NULL;
			}

			if( pCalibObj )
			{
				pCalibObj->Release();
				pCalibObj = NULL;
			}
		}
	}

}

bool 
CHGTChannelCalibrationObj::_IsDuplicate( CEMSRawSpCalibObj* pRawSpCalibObjCurrent )
{
	bool bRet = false;

	std::string oszKey = _CreateKey( pRawSpCalibObjCurrent );

	if( m_omapInputHistory.IsInMap( oszKey ) )
	{
		
		CEMSRawSpCalibObj* recHistory = m_omapInputHistory.Get( oszKey );

		int ibitDiff = 0;
		ibitDiff  = pRawSpCalibObjCurrent->GetBitErrorField1() - recHistory->GetBitErrorField1();
		ibitDiff += pRawSpCalibObjCurrent->GetBitErrorField2() - recHistory->GetBitErrorField2();

		double dCNRdiff = pRawSpCalibObjCurrent->GetCarrierPower() - recHistory->GetCarrierPower();

		if( ibitDiff > 0 )
		{
			// this is a duplicate.
			bRet = true;
		}
		else if ( dCNRdiff <= 0.0 )
		{
			// this is a duplicate.
			bRet = true;
		}

		if( !bRet )
		{
			m_omapInputHistory.Remove( oszKey );
		}
	}

	return bRet;
}

void 
CHGTChannelCalibrationObj::_SaveInputHistory( CEMSRawSpCalibObj* pCalibObj )
{
	pCalibObj->SetTimeStored(CEMSSystemClock::GetTime());
	 
	std::string oszKey = _CreateKey( pCalibObj );

	m_omapInputHistory.Add( oszKey, pCalibObj );

}

void
CHGTChannelCalibrationObj::PerformSpCalibration( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalibObjs,
													CEMSPointerList<CEMSSarrCalibObj>& rolstSarrCalib)
{
	CHGTFileLogger::FlushDBFDetections();

	CEMSRawLpCalibObj* pCalibObj = NULL;


	if( 0 < m_omapInputHistory.Count() )
	{

		CHGTCalibrationObj* pCalibrationObj = NULL;
		CEMSRawSpCalibObj* pRawSpCalibObj = NULL;
		CEMSSarrCalibObj* pSarrCalibData = NULL;
		HGTLSCALIBDATA* pLsCalibData = NULL;
		HGTSASATData strSaSatData;
		LPEMSREFBEACONDATAEX lpRefBeaconData = NULL;

		try
		{
			EMSTIME timeNow = CEMSSystemClock::GetTime();
			CTSiDebugTrace::LogFmt(
				"SP CALIBRATE: PerformSpCalibration called, history count=%d",
				(int)m_omapInputHistory.Count());
			CTSiDebugTrace::Log("SP CALIBRATE: calling MoveFirst");
			m_omapInputHistory.MoveFirst();
			CTSiDebugTrace::Log("SP CALIBRATE: entering GetNext loop");
			while(pRawSpCalibObj = m_omapInputHistory.GetNext())
			{
				CTSiDebugTrace::LogFmt("SP CALIBRATE: GetNext returned item ptr=%p", pRawSpCalibObj);
				INT64 i64AgeNanos = timeNow.intTime - pRawSpCalibObj->GetTimeStored().intTime;
				CTSiDebugTrace::LogFmt("SP CALIBRATE: age=%.3fs (need >25s to process)", i64AgeNanos * 1e-9);
				if( i64AgeNanos > c_i64HistoryAgeoutNanos )
				{
					CTSiDebugTrace::LogFmt(
						"SP CALIBRATE: processing aged record BcnId=%016I64X SatId=%lu AntId=%u age=%.1fs",
						pRawSpCalibObj->GetBeaconID(),
						(unsigned long)pRawSpCalibObj->GetSatId(),
						(unsigned)pRawSpCalibObj->GetAntennaId(),
						(double)i64AgeNanos * 1e-9);

					pCalibrationObj = CHGTCalibrationObjectsContainer::instance()->GetCalibrationObj(
																				pRawSpCalibObj->GetLutId(),
																				pRawSpCalibObj->GetAntennaId(),
																				pRawSpCalibObj->GetSatId());

					lpRefBeaconData = NULL;
					if(m_pRefBcns)
						m_pRefBcns->IsRefBcn( pRawSpCalibObj->GetBeaconID(), lpRefBeaconData );

					// Only log reference-beacon hits - non-hits are the vast majority of
					// traffic and add no diagnostic value for the stuck-at-zero-bias issue.
					if(lpRefBeaconData)
					{
						CTSiDebugTrace::LogFmtAlways(
							"SP CALIBRATE: REF BEACON HIT BcnId=%016I64X uFlag=0x%08lX Lat=%.4f Lon=%.4f Freq=%.1f calibObjFound=%s",
							pRawSpCalibObj->GetBeaconID(),
							(unsigned long)lpRefBeaconData->data.uFlag,
							lpRefBeaconData->data.dLatitude,
							lpRefBeaconData->data.dLongitude,
							lpRefBeaconData->data.dFrequency,
							pCalibrationObj ? "YES" : "NO");
					}

					if(pCalibrationObj)
					{

						if(pRawSpCalibObj->GetAntennaId() > 99) //DBF
						{
							//HGTSASATData strSaSatData;
							memset(&strSaSatData, 0, sizeof(HGTSASATData));
							strSaSatData = CHGTCalibrationObjectsContainer::instance()->GetSatelliteSeparationAngle(pRawSpCalibObj->GetLutId());

							// Check time of last update
							EMSTIME timeNow = CEMSSystemClock::GetTime();
							INT64 i64CheckNanoSeconds = ( ((INT64) 5 ) * ((INT64) 60) * ((INT64) 1000000000) );//5 mins
		
							if( (timeNow.intTime - strSaSatData.timeLastUpdate.intTime) >= i64CheckNanoSeconds )
							{
								//Calculate Separation angle
								CHGTCalibrationObjectsContainer::instance()->CalCulateSatSeparationAngle();
								strSaSatData = CHGTCalibrationObjectsContainer::instance()->GetSatelliteSeparationAngle(pRawSpCalibObj->GetLutId());
							}



							pCalibObj = pCalibrationObj->GetDBFBestSatellite(pRawSpCalibObj, lpRefBeaconData, strSaSatData);

							if(pCalibObj)
							{
								pRawSpCalibObj->SetDuplicateMsgCount(pCalibObj->GetDuplicateMsgCount());
								//Apply calibration to best dbf satellite - 22nd NOV 2021 - Smruti
								if( pCalibObj->GetSatId() != pRawSpCalibObj->GetSatId())
								{
						
									pRawSpCalibObj->SetSatId(pCalibObj->GetSatId());
									//pRawSpCalibObj->SetDuplicateMsgCount(pCalibObj->GetDuplicateMsgCount());


									pCalibrationObj = CHGTCalibrationObjectsContainer::instance()->GetCalibrationObj(
																					pRawSpCalibObj->GetLutId(),
																					pRawSpCalibObj->GetAntennaId(),
																					pRawSpCalibObj->GetSatId());
								}
								//DBF filter operates over a 190 kHz band so that its centre is offset.
								double dFrequency = pRawSpCalibObj->GetFrequency() - c_dFreqDBFoffset;
								pRawSpCalibObj->SetFrequency(dFrequency);
							}
							else
							{
								CTSiDebugTrace::LogFmtAlways(
									"SP CALIBRATE DBF: GetDBFBestSatellite returned NULL "
									"BcnId=%016I64X SatId=%lu AntId=%u - skipping CalibrateSpRawObject",
									pRawSpCalibObj->GetBeaconID(),
									(unsigned long)pRawSpCalibObj->GetSatId(),
									(unsigned)pRawSpCalibObj->GetAntennaId());
								pCalibrationObj->Release();
								pCalibrationObj = NULL;
							}

						// DBF input trace — queued for per-second file output
						// Previous version using CEMSRawSpCalibObj:
						//{
						//	std::string strTime = CEMSConversionUtil::ConvertToDateTimeHiResString2A(
						//		pRawSpCalibObj->GetTimeMsg());
						//	INT64 i64BcnId = pRawSpCalibObj->GetBeaconID();
						//	char szBcnId[32] = "N/A";
						//	if(i64BcnId != 0)
						//		sprintf(szBcnId, "%015llX", i64BcnId);
						//	char szLine[512];
						//	sprintf(szLine, "%s, %u, %u, %s\n",
						//		strTime.c_str(),
						//		pRawSpCalibObj->GetTimeMsg(),
						//		(ULONG)pRawSpCalibObj->GetAntennaId(),
						//		pRawSpCalibObj->GetSatId(),
						//		szBcnId);
						//	CHGTFileLogger::QueueDBFDetection(std::string(szLine));
						//}
						{
							// Use LP calibrated time if available, otherwise fall back to SP
							EMSTIME tMsg = pCalibObj ? pCalibObj->GetTimeMsg() : pRawSpCalibObj->GetTimeMsg();
							const char* szSource = pCalibObj ? "LP" : "SP";
							std::string strTime = CEMSConversionUtil::ConvertToDateTimeHiResString2A(tMsg);
							INT64 i64BcnId = pRawSpCalibObj->GetBeaconID();
							char szBcnId[32] = "N/A";
							if(i64BcnId != 0)
								sprintf(szBcnId, "%015llX", i64BcnId);
							char szLine[512];
							sprintf(szLine, "%s, %u, %u, %s, %s, %.9f, %.3f\n",
								strTime.c_str(),
								(ULONG)pRawSpCalibObj->GetAntennaId(),
								pRawSpCalibObj->GetSatId(),
								szBcnId,
								szSource,
								// -9999 is far outside any naturally-occurring residual (TOA is
								// clamped to +/-0.5 sec, FOA residuals observed so far top out
								// around a few hundred Hz) - marks "no valid residual" without
								// looking like a best-possible (0.0) match to ingestion, which
								// treats lower residuals as better.
								pCalibObj ? pCalibObj->GetResidualTime() : -9999.0,
								pCalibObj ? pCalibObj->GetResidualFrequency() : -9999.0);
							CHGTFileLogger::QueueDBFDetection(std::string(szLine));
						}
					}

						if(pCalibrationObj)
						{
							pCalibObj = pCalibrationObj->CalibrateSpRawObject(pRawSpCalibObj, lpRefBeaconData);
							if( pCalibObj )
							{
								rolstCalibObjs.Add( pCalibObj );
								pCalibObj->Release();
								pCalibObj = NULL;

								if(pCalibrationObj->IsSpSarrUpdate())
								{
									pSarrCalibData = new CEMSSarrCalibObj();
									pCalibrationObj->GetSarrCalibData(pSarrCalibData);
									if(pSarrCalibData)
									{
										rolstSarrCalib.Add(pSarrCalibData);  //Write to FCAL table
										pSarrCalibData->Release();
										pSarrCalibData = NULL;
									}
									else
									{
										pSarrCalibData->Release();
										pSarrCalibData = NULL;
									}
								}
							}
						}
					}

					m_omapInputHistory.RemoveCurrent();
				}

				CTSiDebugTrace::LogFmt("SP CALIBRATE: about to Release item ptr=%p", pRawSpCalibObj);
				pRawSpCalibObj->Release();
				CTSiDebugTrace::Log("SP CALIBRATE: item Release complete");
				pRawSpCalibObj = NULL;
			}
			CTSiDebugTrace::Log("SP CALIBRATE: GetNext loop exited normally");
		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** EXCEPTION in PerformSpCalibration ***");
			if( pCalibrationObj )
			{
				pCalibrationObj->Release();
				pCalibrationObj = NULL;
			}

			if( pCalibObj )
			{
				pCalibObj->Release();
				pCalibObj = NULL;
			}

		}

	}
}
