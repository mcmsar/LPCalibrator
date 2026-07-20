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

#ifndef __EMS_RAW_SPCALIB_OBJECT_H__
#define __EMS_RAW_SPCALIB_OBJECT_H__

#include "aobjbase.h"
#include "emserror.h"
#include "calib406.h"
#include "PointerMap.h"

class CEMSRawSpCalibObj : public CApiObjBase
{
public:
	CEMSRawSpCalibObj();
	CEMSRawSpCalibObj( const CEMSRawSpCalibObj& rawObj );
	CEMSRawSpCalibObj( const EMSCALIB406DATA2& rawData );
	virtual ~CEMSRawSpCalibObj();

	EMS_RESULT Initialize();
	EMS_RESULT Reset();

	inline void Set( const CEMSRawSpCalibObj& rawObj )
	{
		memcpy( &m_rawSpCalibData, &rawObj.m_rawSpCalibData, sizeof( EMSCALIB406DATA2 ) );
	}
	
	inline void Set( const EMSCALIB406DATA2&  rawData )
	{
		memcpy( &m_rawSpCalibData, &rawData, sizeof( EMSCALIB406DATA2 ) );
	}

	inline void GetRawData( EMSCALIB406DATA2&  rawData )
	{
		memcpy( &rawData, &m_rawSpCalibData, sizeof( EMSCALIB406DATA2 ) );
	}

	inline EMSCALIB406DATA2* GetRawData(){ return &m_rawSpCalibData; }

	void AddRawCalibObj( CEMSRawSpCalibObj *pCalibData );

	EMSTIME GetTimeStamp(){ return m_rawSpCalibData.hdr.id.time; }
	ULONG GetSatId() { return m_rawSpCalibData.hdr.id.ulSatID; }
	void SetSatId(ULONG ulSatId){m_rawSpCalibData.hdr.id.ulSatID = ulSatId;}
	ULONG GetLutId() { return m_rawSpCalibData.hdr.id.ulLutID; }
	WORD GetAntennaId() { return m_rawSpCalibData.wAntennaID; }
	void SetAntennaId(WORD wAntId){m_rawSpCalibData.wAntennaID = wAntId;}
	INT64 GetBeaconID(){return m_rawSpCalibData.i64BeaconID;}
	EMSTIME GetTimeMsg(){return m_rawSpCalibData.timeMsg;}
	double GetCarrierPower(){ return m_rawSpCalibData.dCarrierPower;}
	BYTE* GetBeaconMsg(){ return m_rawSpCalibData.cBeaconMsg; }

	int GetBitErrorField1(){return m_rawSpCalibData.wBitErrorField1;}
	int GetBitErrorField2(){return m_rawSpCalibData.wBitErrorField2;}

	EMSTIME GetTimeStored(){ return m_timeStored;}
	void SetTimeStored(EMSTIME time){ m_timeStored = time; }

	double GetFrequency(){return m_rawSpCalibData.dFrequency;}
	void SetFrequency(double frequency){ m_rawSpCalibData.dFrequency = frequency;}

	WORD GetDuplicateMsgCount(){return m_rawSpCalibData.wDuplicateMsgCount;}
	void SetDuplicateMsgCount(WORD wmsgcount){ m_rawSpCalibData.wDuplicateMsgCount = wmsgcount;}

private:
	EMSTIME				   m_timeStored;
	EMSCALIB406DATA2     m_rawSpCalibData;
};

#endif
