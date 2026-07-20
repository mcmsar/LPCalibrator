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

#ifndef __HGT_CALIBRATION_OBJ_CONTAINER_H__
#define __HGT_CALIBRATION_OBJ_CONTAINER_H__

#include "HGTCalibrationObj.h"
#include "PointerMap.h"
#include <string>

class CHGTCalibrationObjectsContainer
{			
	public:
		void DeleteInstance ();
		EMS_RESULT Initialize( );
		static CHGTCalibrationObjectsContainer *instance();

		CHGTCalibrationObj* GetCalibrationObj(ULONG ulLutId, WORD wAntId, ULONG ulSatId);

		void SaveSarrData();

		//For DBF
		void GetCalibrationParameters(ULONG ulLutId, WORD wAntId, ULONG ulSatId, HGTLSCALIBDATA* lsCalibData);

		IEMSOrbit4* GetOrbitInstance()
		{
			if(m_lpOrbit)
				return m_lpOrbit;
			else
				return NULL;
		}

		EMS_RESULT CalCulateSatSeparationAngle();
		HGTSASATData GetSatelliteSeparationAngle(ULONG ulLutId);

	private:
		CHGTCalibrationObjectsContainer() { m_lpOrbit = NULL;}
		~CHGTCalibrationObjectsContainer();

	private:
		void _Serialize();
		void _DeSerialize();
		CHGTCalibrationObj* _AddCalibrationObj( ULONG ulLutId, WORD wAntId, ULONG ulConstellation );
		HGTLSCALIBDATA _GetDefaultData(ULONG ulLutId, WORD wAntId, ULONG ulConstellation);
		std::string _CreateKey( ULONG ulLutId, WORD wAntId, ULONG ulConstellation );
		void _WriteCalibCSVHeader(FILE* lpCalibCSV);
		INT64 _ConvertStringToInt64(std::string str);
		ULONG _GetConstellation( ULONG ulSatId );
		INT64 _GetTimeInt64(std::string timeStr);
		std::string _FormatTime( EMSTIME time );
		int _ConvertToInt(const char* str);
		void _SaveOldLscalibdata();
		EMS_RESULT _InitOrbitObj();

		void _GetLutIdSiteLocation();


	private:

		CEMSCriticalSection     ms_mtxCalibObj;
		char m_szSerializeFileNameCSV[256];

		CEMSPointerMap<std::string, CHGTCalibrationObj> m_omapInputCalibObjs;

		std::list<HGTLSCALIBDATA>  m_lstlsCalibData;

		IEMSOrbit4				*m_lpOrbit;
		std::map<ULONG, EMSLOCATION> m_mapLutIdSitelocation;
		std::map<ULONG, HGTSASATData> m_mapLutIdSeparationAngle;
};

#endif