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

#ifndef __EMS_SARR_CALIB_OBJECT_H__
#define __EMS_SARR_CALIB_OBJECT_H__

#include "aobjbase.h"
#include "emserror.h"
#include "sarrcalib.h"

class CEMSSarrCalibObj : public CApiObjBase
{
public:
	CEMSSarrCalibObj();
	CEMSSarrCalibObj( const CEMSSarrCalibObj& rawObj );
	CEMSSarrCalibObj( const EMSSARRCALIBDATA& rawData );
	virtual ~CEMSSarrCalibObj();

	EMS_RESULT Initialize();
	EMS_RESULT Reset();

	inline void Set( const CEMSSarrCalibObj& rawObj )
	{
		memcpy( &m_rawSarrCalibData, &rawObj.m_rawSarrCalibData, sizeof( EMSSARRCALIBDATA ) );
	}
	
	inline void Set( const EMSSARRCALIBDATA&  rawData )
	{
		memcpy( &m_rawSarrCalibData, &rawData, sizeof( EMSSARRCALIBDATA ) );
	}

	inline void GetRawData( EMSSARRCALIBDATA&  rawData )
	{
		memcpy( &rawData, &m_rawSarrCalibData, sizeof( EMSSARRCALIBDATA ) );
	}

	inline EMSSARRCALIBDATA* GetRawData(){ return &m_rawSarrCalibData; }

	void SetSarrNumber(ULONG ulSarrNumber){ m_rawSarrCalibData.ulSARRNumber = ulSarrNumber;}
	ULONG GetSarrNumber(){ return m_rawSarrCalibData.ulSARRNumber;}

	void SetLutId(ULONG ulLutId){ m_rawSarrCalibData.hdr.ulLutID = ulLutId;}
	ULONG GetLutId(){ return m_rawSarrCalibData.hdr.ulLutID;}

	void SetSatId( ULONG ulSatId){m_rawSarrCalibData.hdr.ulSatID = ulSatId;}
	ULONG GetSatId(){ return m_rawSarrCalibData.hdr.ulSatID;}

	void SetPassId(ULONG ulPassId){ m_rawSarrCalibData.ulPassID = ulPassId;}
	ULONG GetPassId(){ return m_rawSarrCalibData.ulPassID;}

	void SetSource(WORD wSource){ m_rawSarrCalibData.wSource = wSource;}
	WORD GetSource(){ return m_rawSarrCalibData.wSource;}

	void SetFreqOffset(float fFreqOffset){ m_rawSarrCalibData.fFreqOffset = fFreqOffset; }
	float GetFreqOffset(){ return m_rawSarrCalibData.fFreqOffset;}

	void SetFreqDrift(float fFreqDrift){ m_rawSarrCalibData.fFreqDrift = fFreqDrift;}
	float GetFreqDrift(){ return m_rawSarrCalibData.fFreqDrift;}

	void SetTimeOffset(INT64 i64TimeOffset){m_rawSarrCalibData.i64TimeOffset = i64TimeOffset;}
	INT64 GetTimeOffset(){return m_rawSarrCalibData.i64TimeOffset;}

	void SetTimeEffective(EMSTIME timeEffective){ m_rawSarrCalibData.timeEffective = timeEffective;}
	EMSTIME GetTimeEffective(){ return m_rawSarrCalibData.timeEffective; }

	void SetTime(EMSTIME time){ m_rawSarrCalibData.hdr.time = time; }
	EMSTIME GetTime(){ return m_rawSarrCalibData.hdr.time; }


private:
	EMSSARRCALIBDATA	m_rawSarrCalibData;

};

#endif
