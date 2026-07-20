/*********************************************************************
*	              Copyright (c) 2006 by EMS Technologies, Inc.,
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

#ifndef __HGT_CALIBRATION_OBJ_H__
#define __HGT_CALIBRATION_OBJ_H__

#pragma warning(disable : 4786)

#include "aobjbase.h"
#include "emstypes.h"
#include "emserror.h"
#include "emsexcpt.h" 			// CEMSException
#include "RawLpCalibObj.h"
#include "PointerList.h"
#include "RawSpCalibObj.h"
#include "EMSCLOCK.H"

#include "emsorbit.h"			// IEMSOrbit
#include "emscoord.h"
#include "emsconst.h"
#include "SarrCalibObj.h"
#include "EMSBEACN.H"
#include <string>

const ULONG INTERFERER_TYPE = 0x24;
const ULONG MISCODED_TYPE = 0x25;

const int c_iMaxDataSource = 100;

typedef enum _tagEMSSOURCESTATUS
{
	EMS_CALIB_SOURCE_OK = 0,
	EMS_CALIB_SOURCE_ALARM = 1
} EMSSOURCESTATUS;

typedef struct _tagEMSDATASOURCESTATUS
{
	ULONG	         ulSatID;
	ULONG	         ulLUTID;
	ULONG	         ulAntID;
	EMSSOURCESTATUS  eStatus;
	EMSTIME			 tLastUpdate;
	int              iAlarmCount;
} EMSDataSourceStatus;

typedef struct tagHGTLSCalibData
{
	ULONG ulLutId;
	WORD wAntId;
	WORD wAntType;
	ULONG ulConstellation;
	EMSLOCATION	siteLocation;
	double dFoaGain;
	double dFoaThreshold;
	double dToaGain;
	double dToaThreshold;
	EMSTIME timeLastUpdate;
	double dFoaBias;  //frequencyoffset
	double dToaBias; //Time offset
	ULONG ulFoaCount;
	ULONG ulToaCount;
	std::string arrComments;
} HGTLSCALIBDATA;

//For Satellite Separation Angle
typedef struct tagHGTSASATData
{
	ULONG	ulSAcount;
	ULONG	ulSAsat1[100];
	ULONG	ulSAsat2[100];
	double	dSAangle[100];
	EMSTIME timeLastUpdate;
}HGTSASATData;

class CHGTCalibrationObj :	public CApiObjBase
{			
	public:
		CHGTCalibrationObj();
		CHGTCalibrationObj( CHGTCalibrationObj& Obj );
		CHGTCalibrationObj( const HGTLSCALIBDATA& lsCalibData, IEMSOrbit4 *lpOrbit);
		~CHGTCalibrationObj();

		 EMS_RESULT Init();

		HGTLSCALIBDATA* GetLsCalibData( ){return &m_lsCalibData;}
		bool IsSpSarrUpdate(){ return m_bSpSarrUpdated;}
		bool IsToaFoaSarrUpdate(){ return m_bToaFoaSarrUpdated;}


		void GetSarrCalibData(CEMSSarrCalibObj* pSarr)
		{
			pSarr->Set(*m_sarrCalibData.GetRawData());
		}

		CEMSRawLpCalibObj* CalibrateSpRawObject(CEMSRawSpCalibObj*  pRawCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData);
		CEMSRawLpCalibObj* CalibrateLpRawObject(CEMSRawLpCalibObj*  pRawLpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData);

		CEMSRawLpCalibObj* GetDBFBestSatellite(CEMSRawSpCalibObj*  pRawSpCalibObj, 
										LPEMSREFBEACONDATAEX lpRefBeaconData, HGTSASATData &strSaSatData);
	
	private:

		//EMS_RESULT _InitOrbitObj();

		EMS_RESULT _CreateObjects( void );
		void _ReleaseObjects( void );


		EMS_RESULT _SetInitialFlags( CEMSRawLpCalibObj* pRawLpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData );
		CEMSRawLpCalibObj* _CreateRawLPCalibObject(CEMSRawSpCalibObj*  pRawSpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData);
		EMS_RESULT _OrbitUpdate(CEMSRawLpCalibObj* pRawLpCalibObj);
		EMS_RESULT _SetGSAzimuthElevation(CEMSRawLpCalibObj* pRawLpCalibObj);

		EMS_RESULT _RestoreMeasuredSignal( CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _RemoveDownlinkPropogationTime( CEMSRawLpCalibObj* pRawLpCalibObj ) ;
		EMS_RESULT _RemoveDownlinkDoppler( CEMSRawLpCalibObj* pRawLpCalibObj ) ;
		EMS_RESULT _DoSpectrumInversion( CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _CorrectDownLink(CEMSRawLpCalibObj* pRawLpCalibObj);

		EMS_RESULT _CalcTimeResidual( EMSRANGERATED rangeRateUL, CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _CalcFreqResidual( EMSRANGERATED rangeRateUL, CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _ComputeResidual(CEMSRawLpCalibObj* pRawLpCalibObj);
		EMS_RESULT _ComputeTOAFOAResidual(CEMSRawLpCalibObj* pRawLpCalibObj);

		BOOL IsRLS( const BYTE *msg);
		BOOL IsELT_DT( const BYTE *msg);
		void _CheckPDF2( CEMSRawLpCalibObj* pRawLpCalibObj );
		void _CheckProtocol( CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _AdjustSarrFreqAndTime( CEMSRawLpCalibObj* pRawLpCalibObj );

		bool _IsHarmonic( double dFreqDiff );
		bool _CheckDataSourceStatus();
		bool _CheckDataSourceTime();
		void _UpdateAlarmStatus( double dValue );
		EMS_RESULT _SetOffsets(); 
		EMS_RESULT _CalculateOffsets( CEMSRawLpCalibObj* pRawLpCalibObj );
		EMS_RESULT _CalculateToaFoaOffsets( CEMSRawLpCalibObj* pRawLpCalibObj );

		EMS_RESULT _ApplyTOAFOAAdjustments(CEMSRawLpCalibObj* pRawLpCalibObj);
		EMS_RESULT _DataValidation(CEMSRawLpCalibObj* pRawLpCalibObj);

		std::string _FormatInputTime( EMSTIME time );

		//For DBF
		EMS_RESULT _GetDBFSatellite(CEMSRawLpCalibObj* pRawSpCalibObj, ULONG ulMinSat, ULONG ulMaxSat, HGTSASATData &strSaSatData);
		EMS_RESULT _AdjustDBFSarrFreqAndTime( CEMSRawLpCalibObj* pRawLpCalibObj, HGTLSCALIBDATA& lsCalibData, ULONG ulSat );

		// Moving beacon
		//EMS_RESULT _MovingBeacon(CEMSRawLpCalibObj*  pRawLpCalibObj, LPEMSREFBEACONDATAEX lpRefBeaconData);


	private:

		IEMSOrbit4				*m_lpOrbit;
		CEMSException			m_exception;
	
		ULONG	m_ulSARRNumber;

		EMSDataSourceStatus     m_aDataSourceStatus[c_iMaxDataSource];

		HGTLSCALIBDATA		m_lsCalibData;  //data members in structure
		bool				m_bSpSarrUpdated;
		bool				m_bToaFoaSarrUpdated;

		CEMSSarrCalibObj	m_sarrCalibData;

		//For DBF
		ULONG m_ulArrBeamId[1000];


		double				 m_dDopplerShiftDL;
		double				 m_dDopplerShiftUL;

		// For Moving beacon
		bool				m_bMovingBeaconFirstTime;
		INT64               m_i64MatchBeaconID;
		INT64               m_i64MovingBeaconID;
		byte				m_bMovingBeaconMsg[ 18 ];
		double				m_dSpeed;
		double				m_dHeading;


};

#endif