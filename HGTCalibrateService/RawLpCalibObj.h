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

#ifndef __EMS_RAW_LP_CALIB_OBJECT_H__
#define __EMS_RAW_LP_CALIB_OBJECT_H__

#include "aobjbase.h"
#include "emserror.h"
#include "CALIB406EXT.h"

class CEMSRawLpCalibObj : public CApiObjBase
{
public:
	CEMSRawLpCalibObj();
	CEMSRawLpCalibObj( const CEMSRawLpCalibObj& rawObj );
	CEMSRawLpCalibObj( const EMSCALIB406DATAEXT& rawData );
	virtual ~CEMSRawLpCalibObj();

	EMS_RESULT Initialize();
	EMS_RESULT Reset();

	inline void Set( CEMSRawLpCalibObj& rawObj )
	{
		memcpy( &m_rawCalibData, &rawObj.m_rawCalibData, sizeof( EMSCALIB406DATAEXT ) );
		m_timeStored = rawObj.GetTimeStored();
	}

	inline void Set( const CEMSRawLpCalibObj& rawObj )
	{
		memcpy( &m_rawCalibData, &rawObj.m_rawCalibData, sizeof( EMSCALIB406DATAEXT ) );
	}
	
	inline void Set( const EMSCALIB406DATAEXT&  rawData )
	{
		memcpy( &m_rawCalibData, &rawData, sizeof( EMSCALIB406DATAEXT ) );
	}

	inline void SetRawSpCalib(EMSCALIB406DATA2& rawSpCalibData)
	{
		memcpy( &m_rawCalibData.calib406, &rawSpCalibData, sizeof( EMSCALIB406DATA2 ) );
	}


	inline void GetRawData( EMSCALIB406DATAEXT&  rawData )
	{
		memcpy( &rawData, &m_rawCalibData, sizeof( EMSCALIB406DATAEXT ) );
	}

	inline EMSCALIB406DATAEXT* GetRawData(){ return &m_rawCalibData; }

	void ResetStatusFlags();
	void UpdatePassId();

	EMSTIME GetTime(){ return m_rawCalibData.calib406.hdr.id.time;}
	void SetTime(EMSTIME time){m_rawCalibData.calib406.hdr.id.time = time;}
	WORD GetAntennaId(){return m_rawCalibData.calib406.wAntennaID;}
	void SetAntennaId(WORD wAntId){m_rawCalibData.calib406.wAntennaID = wAntId;}
	WORD GetType(){ return m_rawCalibData.calib406.hdr.wType; }
	void SetType(WORD type){ m_rawCalibData.calib406.hdr.wType = type; }
	ULONG GetSatId(){ return m_rawCalibData.calib406.hdr.id.ulSatID; }
	void SetSatId(ULONG satId){ m_rawCalibData.calib406.hdr.id.ulSatID = satId; }
	ULONG GetLutId(){ return m_rawCalibData.calib406.hdr.id.ulLutID; }
	void SetLutId(ULONG lutId){ m_rawCalibData.calib406.hdr.id.ulLutID = lutId; }
	DWORD GetProcessFlags(){ return m_rawCalibData.extras.dwProcessFlags; }
	void SetProcessFlags(DWORD dwFlags) { m_rawCalibData.extras.dwProcessFlags = dwFlags; }
	INT64 GetBcnId(){ return m_rawCalibData.calib406.i64BeaconID; }
	void SetBcnId( INT64 i64BeaconID ) { m_rawCalibData.calib406.i64BeaconID = i64BeaconID; }
	void SetBcnVector(EMSVECTORD bcnVector){ m_rawCalibData.extras.BeaconVector = bcnVector;} //verify ??
	EMSVECTORD GetBcnVector(){return m_rawCalibData.extras.BeaconVector;}
	void SetTxFrequency(double dFrequency){ m_rawCalibData.extras.dTxFrequency = dFrequency; }
	double GetTxFrequency(){return m_rawCalibData.extras.dTxFrequency;}
	EMSTIME GetTimeMsg(){ return m_rawCalibData.calib406.timeMsg;}
	void SetTimeMsg(EMSTIME timeMsg){ m_rawCalibData.calib406.timeMsg = timeMsg;}
	void SetReserve0(DWORD dwReserve){m_rawCalibData.calib406.dwReserve0 = dwReserve; }
	DWORD GetReserve0(){return m_rawCalibData.calib406.dwReserve0;}
	void SetReserve3(double dReserve){m_rawCalibData.calib406.dReserve3 = dReserve; }
	double GetReserve3(){return m_rawCalibData.calib406.dReserve3;}
	void SetPassId(ULONG ulOrbitNumber){m_rawCalibData.calib406.ulPassID = ulOrbitNumber;}
	ULONG GetPassId(){ return m_rawCalibData.calib406.ulPassID;}
	void SetSatPVC(EMSTIMECOORDD timeCoord){m_rawCalibData.extras.SatTPVC = timeCoord;} //verify ??
	EMSTIMECOORDD GetSatPVC(){return m_rawCalibData.extras.SatTPVC;}
	void SetAzimuth(EMSDEGREES az){ m_rawCalibData.calib406.dReserve1 = az;}
	EMSDEGREES GetAzimuth(){return m_rawCalibData.calib406.dReserve1;} 
	void SetElevation(EMSDEGREES el){ m_rawCalibData.calib406.dReserve2 = el;}
	EMSDEGREES GetElevation(){return m_rawCalibData.calib406.dReserve2;}
	double GetFreqOffset(){return m_rawCalibData.calib406.dFreqOffset;}
	void SetFreqOffset(double freqOffset){ m_rawCalibData.calib406.dFreqOffset = freqOffset;}
	double GetFrequency(){return m_rawCalibData.calib406.dFrequency;}
	void SetFrequency(double frequency){ m_rawCalibData.calib406.dFrequency = frequency;}
	INT64 GetTimeOffset(){return m_rawCalibData.calib406.i64TimeOffset;}
	void SetTimeOffset(INT64 i64TimeOffset){m_rawCalibData.calib406.i64TimeOffset = i64TimeOffset;}
	EMSTIME GetTransmitTime(){return m_rawCalibData.extras.timeTransmit;}
	void SetTransmitTime(EMSTIME txTime){m_rawCalibData.extras.timeTransmit = txTime;}
	double GetResidualTime(){ return m_rawCalibData.extras.dTimeResidual;}
	void SetResidualTime(double resTime){ m_rawCalibData.extras.dTimeResidual = resTime;}
	double GetResidualFrequency(){ return m_rawCalibData.extras.dFreqResidual;}
	void SetResidualFrequency(double resFreq){ m_rawCalibData.extras.dFreqResidual = resFreq;}

	//For Downlink
	DWORD GetResolutionFlags(){return m_rawCalibData.calib406.dwResolutionFlags;}
	void SetResolutionFlags(DWORD dwResFlag){ m_rawCalibData.calib406.dwResolutionFlags = dwResFlag;}

	//For Uplink
	DWORD GetDataFlags(){return m_rawCalibData.calib406.dwDataFlags;}
	void SetDataFlags(DWORD dwDataFlag){ m_rawCalibData.calib406.dwDataFlags = dwDataFlag;}

	BYTE* GetBeaconMsg(){return m_rawCalibData.calib406.cBeaconMsg;}
	void SetBeaconMsg(BYTE* BcnMsg)
	{
		//memset(m_rawCalibData.calib406.cBeaconMsg, 0, sizeof(m_rawCalibData.calib406.cBeaconMsg));
		memcpy(m_rawCalibData.calib406.cBeaconMsg, BcnMsg, sizeof(m_rawCalibData.calib406.cBeaconMsg));
	}

	WORD GetBitErrorField1(){return m_rawCalibData.calib406.wBitErrorField1;}
	void SetBitErrorField1(WORD werrFld){ m_rawCalibData.calib406.wBitErrorField1 = werrFld;}

	WORD GetBitErrorField2(){return m_rawCalibData.calib406.wBitErrorField2;}
	void SetBitErrorField2(WORD werrFld){ m_rawCalibData.calib406.wBitErrorField2 = werrFld;}

	double GetCarrierPower(){ return m_rawCalibData.calib406.dCarrierPower;}
	void SetCarrierPower(double dPower){ m_rawCalibData.calib406.dCarrierPower = dPower; }

	EMSTIME GetTimeStored(){ return m_timeStored;}
	void SetTimeStored(EMSTIME time){ m_timeStored = time; }

	WORD GetBurstNumber(){return m_rawCalibData.extras.wBurstNumber;}
	void SetBurstNumber(WORD wburstnum){ m_rawCalibData.extras.wBurstNumber = wburstnum;}

	WORD GetDuplicateMsgCount(){return m_rawCalibData.calib406.wDuplicateMsgCount;}
	void SetDuplicateMsgCount(WORD wmsgcount){ m_rawCalibData.calib406.wDuplicateMsgCount = wmsgcount;}

private:
	EMSCALIB406DATAEXT     m_rawCalibData;
	EMSTIME				   m_timeStored;

};

#endif
