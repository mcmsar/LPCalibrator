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

#include "HGTCalibrationObjectsContainer.h"
#include "emsexcpt.h"
#include "CSVReader.h"
#include "convutility.h"
#include "LogHelper.h"
#include "calib406msg.h"
#include "HGTFileLogger.h"
#include "TSiDebugTrace.h"
#include <math.h>
#include "emsvectr.h"

//#define INITGUID
//#include <initguid.h>

const std::string cstrCalibDataDir = "C:\\Program Files (x86)\\EMS Technologies\\LUT Server\\Config\\"; 
const std::string cstrCalibDataFile = "lscalibdata";

CHGTCalibrationObjectsContainer::~CHGTCalibrationObjectsContainer()
{

}

CHGTCalibrationObjectsContainer * CHGTCalibrationObjectsContainer::instance()
{
	static CHGTCalibrationObjectsContainer inst;
	return &inst;   
}

EMS_RESULT
CHGTCalibrationObjectsContainer::_InitOrbitObj()
{
	CTSiDebugTrace::Log("CalibObjContainer::_InitOrbitObj: entry");
	EMS_RESULT hr = EMS_OK;

	hr = CoCreateInstance( CLSID_EMSOrbit, 0, CLSCTX_ALL, IID_IEMSOrbit4,
							(LPVOID*)&m_lpOrbit );
	CTSiDebugTrace::LogHR("CalibObjContainer::_InitOrbitObj: CoCreateInstance(CLSID_EMSOrbit/IEMSOrbit4)", hr);

	if ( hr != EMS_OK )
	  	throw CEMSException( EMS_CALIB406_ORBIT_ERROR, hr );

	return hr;
}

EMS_RESULT
CHGTCalibrationObjectsContainer::Initialize( )
{
	CTSiDebugTrace::Log("CalibObjContainer::Initialize: entry");
	EMS_RESULT hr = EMS_OK;
	hr = _InitOrbitObj();
	CTSiDebugTrace::LogHR("CalibObjContainer::Initialize: _InitOrbitObj", hr);
	if(hr == EMS_OK)
	{
		CTSiDebugTrace::Log("CalibObjContainer::Initialize: calling _DeSerialize (reads lscalibdata.csv)");
		_DeSerialize();
		CTSiDebugTrace::Log("CalibObjContainer::Initialize: _DeSerialize complete");
	}
	CTSiDebugTrace::LogHR("CalibObjContainer::Initialize: exit", hr);
	return hr;
}

void 
CHGTCalibrationObjectsContainer::DeleteInstance()
{
	_Serialize();

	if ( m_lpOrbit )
	{
		m_lpOrbit->Release();
		m_lpOrbit = 0;
	}
}

void 
CHGTCalibrationObjectsContainer::SaveSarrData()
{
	_SaveOldLscalibdata();
	_Serialize();
}

std::string
CHGTCalibrationObjectsContainer::_CreateKey( ULONG ulLutId, WORD wAntId, ULONG ulConstellation )
{
	// Create the key.
	char szKey[ 128 ];

	// DBF test
	if (wAntId>100) wAntId = (wAntId/100)*100;

	sprintf( szKey, "%d_%d_%d", ulLutId, wAntId, ulConstellation);

	std::string oszKey( szKey );
	return oszKey;
}

void
CHGTCalibrationObjectsContainer::GetCalibrationParameters(ULONG ulLutId, WORD wAntId, 
															ULONG ulSatId, HGTLSCALIBDATA* lsCalibData)
{
	CHGTCalibrationObj *pCalibObj = NULL;
	std::string oszKey;

	try
	{

		ULONG ulConstellation = _GetConstellation(ulSatId);

		// Check if DBF
		if( wAntId > 99 )
		{
			wAntId = (wAntId/100)*100;
			ulConstellation = ulSatId;
		}

		oszKey = _CreateKey( ulLutId, wAntId, ulConstellation );

		if(m_omapInputCalibObjs.IsInMap(oszKey))
		{
			pCalibObj = m_omapInputCalibObjs.Get( oszKey );
			memcpy(lsCalibData, pCalibObj->GetLsCalibData(), sizeof(HGTLSCALIBDATA));
		}
		else
		{
			//Use default calib parameters - TBD
		}

	}
	catch( ... )
	{
		throw;
	}
	
}




CHGTCalibrationObj*
CHGTCalibrationObjectsContainer::GetCalibrationObj( ULONG ulLutId, WORD wAntId, ULONG ulSatId)
{
	CHGTCalibrationObj *pCalibObj = NULL;
	std::string oszKey;

	try
	{
		ms_mtxCalibObj.Enter();

		ULONG ulConstellation = _GetConstellation(ulSatId);

		// Check if DBF
		if( wAntId > 99 )
		{
			wAntId = (wAntId/100)*100;
			ulConstellation = ulSatId;
		}

		oszKey = _CreateKey( ulLutId, wAntId, ulConstellation );

		if(m_omapInputCalibObjs.IsInMap(oszKey))
		{
			pCalibObj = m_omapInputCalibObjs.Get( oszKey );
		}
		else
		{
			pCalibObj = _AddCalibrationObj(ulLutId, wAntId, ulConstellation);
		}


		ms_mtxCalibObj.Leave();
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in GetCalibrationObj ***");
		ms_mtxCalibObj.Leave();
		throw;
	}

	return pCalibObj;
}

ULONG
CHGTCalibrationObjectsContainer::_GetConstellation( ULONG ulSatId )
{
	ULONG ulConstellation = 0;

	if ( (ulSatId > 200) && (ulSatId < 300) ) ulConstellation = 200;
	if ( (ulSatId > 300) && (ulSatId < 400) ) ulConstellation = 300;
	if ( (ulSatId > 400) && (ulSatId < 500) ) ulConstellation = 400;
	if ( (ulSatId > 500) && (ulSatId < 600) ) ulConstellation = 500;
	if ( (ulSatId > 600) && (ulSatId < 700) ) ulConstellation = 600;

	// RR - Special case for Beidou M13 and M14
	//if ( (ulSatId > 550) && (ulSatId < 590) ) ulConstellation = 700;
	if ( (ulSatId > 600) && (ulSatId < 650) ) ulConstellation = 700;

	return ulConstellation;
}

HGTLSCALIBDATA 
CHGTCalibrationObjectsContainer::_GetDefaultData(ULONG ulLutId, WORD wAntId, ULONG ulConstellation)
{
	HGTLSCALIBDATA lsCalibData;
	memset(&lsCalibData, 0, sizeof(HGTLSCALIBDATA));

	EMSLOCATION siteLocation;
	memset(&siteLocation, 0, sizeof(EMSLOCATION));

	try
	{
		std::list<HGTLSCALIBDATA>::iterator it = m_lstlsCalibData.begin();

		while( it != m_lstlsCalibData.end() )
		{
			if((it->ulLutId ==  ulLutId) )
			{
				siteLocation = it->siteLocation;

				if ( (it->wAntId == wAntId) && (it->ulConstellation == ulConstellation))
				{
					lsCalibData = *it;

					break;
				}
			}
			it++;
		}
	}
	catch(...)
	{
		throw;
	}

	if((lsCalibData.ulLutId == 0) && (lsCalibData.wAntId == 0) && (lsCalibData.ulConstellation == 0) )
	{
		if( ( siteLocation.degLatitude != 0.0 ) && 
			( siteLocation.degLongitude != 0.0 ) &&
			( siteLocation.fAltitude != 0.0 ) )
		{
			lsCalibData.ulLutId = ulLutId;
			lsCalibData.wAntId = wAntId;
			lsCalibData.ulConstellation = ulConstellation;
			lsCalibData.dFoaBias = 0;
			lsCalibData.dFoaGain = 0.01;
			lsCalibData.dFoaThreshold = 10;
			lsCalibData.dToaGain = 0.001;
			lsCalibData.dToaThreshold = 100;
			//lsCalibData.i64ToaBias = 0;
			lsCalibData.dToaBias = 0.0;
			lsCalibData.siteLocation = siteLocation;
			lsCalibData.timeLastUpdate.intTime = 0;
			lsCalibData.ulFoaCount = 0;
			lsCalibData.ulToaCount = 0;
			lsCalibData.wAntType = 1;
		}
		else
		{

			CEMSLogMsgParam	param(1);

			TCHAR szInfo[256] = "";
			sprintf( szInfo, 
				TEXT("Calib Service - Site Location is not configured for Lut Id = %d , Antenna Id = %d "), ulLutId, wAntId );		
			param.Add( szInfo );

			CEMSLogHelper::LogWarningMsg( EMS_CALIB406_TRACER, param );
		}

	}

	return lsCalibData;
}

CHGTCalibrationObj* 
CHGTCalibrationObjectsContainer::_AddCalibrationObj( ULONG ulLutId, WORD wAntId, ULONG ulConstellation)
{
	CHGTCalibrationObj *pCalibObj = NULL;
	try
	{
		ms_mtxCalibObj.Enter();

		std::string oszKey = _CreateKey( ulLutId, wAntId, ulConstellation );

		HGTLSCALIBDATA lsCalibData = _GetDefaultData(ulLutId, wAntId, ulConstellation); 

		//for sp - if site location is not known - ignore the record
		if( (lsCalibData.ulLutId > 0) && (lsCalibData.ulConstellation > 0))
		{
			pCalibObj = new CHGTCalibrationObj(lsCalibData, m_lpOrbit);

			m_omapInputCalibObjs.Add(oszKey, pCalibObj);
		}

		ms_mtxCalibObj.Leave();
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _AddCalibrationObj ***");
		ms_mtxCalibObj.Leave();
		throw;
	}

	return pCalibObj;
}

INT64
CHGTCalibrationObjectsContainer::_ConvertStringToInt64(std::string str)
{
	INT64 i64OutPut;
	std::istringstream stream (str);
	stream >> i64OutPut;

	return i64OutPut;
}

std::string 
CHGTCalibrationObjectsContainer::_FormatTime( EMSTIME time )
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

int 
CHGTCalibrationObjectsContainer::_ConvertToInt(const char* str)
{
	int ret = 0;
	sscanf(str, "%d", &ret);
	return ret;
}

INT64 
CHGTCalibrationObjectsContainer::_GetTimeInt64(std::string timeStr)
{
	std::vector<std::string> wordVector;
	wordVector.clear();
	std::stringstream stringStream(timeStr);
	std::string line;
	int year = 0;int jday = 0;int hour = 0;int min = 0;int sec = 0;
	double secs = 0.0;

	while(std::getline(stringStream, line)) 
	{
		std::size_t prev = 0, pos;
		while ((pos = line.find_first_of("/ :", prev)) != std::string::npos)
		{
			if (pos > prev)
				wordVector.push_back(line.substr(prev, pos-prev));
			prev = pos+1;
		}
		if (prev < line.length())
			wordVector.push_back(line.substr(prev, std::string::npos));
	}

	if(wordVector.size() > 0)
		year = _ConvertToInt(wordVector[0].c_str());
	if(wordVector.size() > 1)
		jday = _ConvertToInt(wordVector[1].c_str());
	if(wordVector.size() > 2)
		hour = _ConvertToInt(wordVector[2].c_str());
	if(wordVector.size() > 3)
		min = _ConvertToInt(wordVector[3].c_str());
	if(wordVector.size() > 4)
	{
		sec = _ConvertToInt(wordVector[4].c_str());
		secs = CEMSConversionUtil::ConvertToDouble(wordVector[4].c_str());
	}
	double diff = secs - (double)sec;
	CEMSTime tm(0);
	tm.SetTime(year, jday, hour, min, sec, 0);
	tm.AddSeconds(diff);

	return tm.GetTimeNano();
}

void
CHGTCalibrationObjectsContainer::_DeSerialize()
{
	memset(m_szSerializeFileNameCSV, 0, sizeof(m_szSerializeFileNameCSV));
	sprintf( m_szSerializeFileNameCSV, "%s%s.csv", cstrCalibDataDir.c_str(), cstrCalibDataFile.c_str() );

	CTSiDebugTrace::LogFmt("CSV READ: opening file: %s", m_szSerializeFileNameCSV);

	HGTLSCALIBDATA lsCalibData;
	m_lstlsCalibData.clear();
	CHGTCalibrationObj *pCalibObj = NULL;

	try
	{
		std::ifstream file(m_szSerializeFileNameCSV);
		if(file.good())
		{
			CTSiDebugTrace::Log("CSV READ: file opened successfully");
			CSVRow row;
			file >> row;  //Get the header
			CTSiDebugTrace::Log("CSV READ: header row skipped");

			int nRowCount  = 0;
			int nValidRows = 0;

			while(file >> row)
			{
				nRowCount++;
				memset(&lsCalibData, 0, sizeof(HGTLSCALIBDATA));

				if(row.size() <= 1)
				{
					CTSiDebugTrace::LogFmt("CSV READ  row %2d: skipped (only %d column(s))", nRowCount, (int)row.size());
					continue;
				}

				lsCalibData.ulLutId          = CEMSConversionUtil::ConvertToULong(row[0].c_str());
				lsCalibData.wAntId           = FromString<WORD>(row[1]);
				lsCalibData.wAntType         = FromString<WORD>(row[2]);
				lsCalibData.ulConstellation  = CEMSConversionUtil::ConvertToULong(row[3].c_str());
				lsCalibData.siteLocation.degLatitude  = CEMSConversionUtil::ConvertToDouble(row[4].c_str());
				lsCalibData.siteLocation.degLongitude = CEMSConversionUtil::ConvertToDouble(row[5].c_str());
				lsCalibData.siteLocation.fAltitude    = CEMSConversionUtil::ConvertToFloat(row[6].c_str());
				lsCalibData.dFoaGain         = CEMSConversionUtil::ConvertToDouble(row[7].c_str());
				lsCalibData.dFoaThreshold    = CEMSConversionUtil::ConvertToDouble(row[8].c_str());
				lsCalibData.dToaGain         = CEMSConversionUtil::ConvertToDouble(row[9].c_str());
				lsCalibData.dToaThreshold    = CEMSConversionUtil::ConvertToDouble(row[10].c_str());
				lsCalibData.timeLastUpdate.intTime = _GetTimeInt64(row[11]);
				lsCalibData.dFoaBias         = CEMSConversionUtil::ConvertToFloat(row[12].c_str());
				lsCalibData.dToaBias         = CEMSConversionUtil::ConvertToDouble(row[13].c_str());
				lsCalibData.ulFoaCount       = CEMSConversionUtil::ConvertToULong(row[14].c_str());
				lsCalibData.ulToaCount       = CEMSConversionUtil::ConvertToULong(row[15].c_str());
				lsCalibData.arrComments      = row[16];

				CTSiDebugTrace::LogFmt(
					"CSV READ  row %2d: LutId=%5lu AntId=%3d AntType=%d Const=%3lu "
					"Lat=%8.3f Lon=%9.3f Alt=%7.1f "
					"FOAGain=%.4f FOAThr=%6.1f TOAGain=%.5f TOAThr=%6.1f "
					"LastUpdate=%-22s FOABias=%8.3f TOABias=%12.6f "
					"FOACnt=%5lu TOACnt=%5lu Comments=%s",
					nRowCount,
					lsCalibData.ulLutId, (int)lsCalibData.wAntId, (int)lsCalibData.wAntType,
					lsCalibData.ulConstellation,
					lsCalibData.siteLocation.degLatitude, lsCalibData.siteLocation.degLongitude,
					(double)lsCalibData.siteLocation.fAltitude,
					lsCalibData.dFoaGain, lsCalibData.dFoaThreshold,
					lsCalibData.dToaGain, lsCalibData.dToaThreshold,
					row[11].c_str(),
					lsCalibData.dFoaBias, lsCalibData.dToaBias,
					lsCalibData.ulFoaCount, lsCalibData.ulToaCount,
					lsCalibData.arrComments.c_str());

				if ( (lsCalibData.ulConstellation > 0) && (lsCalibData.ulLutId > 0) )
				{
					nValidRows++;
					m_lstlsCalibData.push_back(lsCalibData);

					std::string oszKey = _CreateKey( lsCalibData.ulLutId,
													 lsCalibData.wAntId, lsCalibData.ulConstellation );

					pCalibObj = new CHGTCalibrationObj(lsCalibData, m_lpOrbit);
					m_omapInputCalibObjs.Add(oszKey, pCalibObj);
				}
				else
				{
					CTSiDebugTrace::LogFmt(
						"CSV READ  row %2d: SKIPPED - LutId or Constellation is 0 (LutId=%lu Const=%lu)",
						nRowCount, lsCalibData.ulLutId, lsCalibData.ulConstellation);
				}
			}

			CTSiDebugTrace::LogFmtAlways("CSV READ: done - total rows=%d, valid/loaded=%d, skipped=%d",
				nRowCount, nValidRows, nRowCount - nValidRows);
		}
		else
		{
			CTSiDebugTrace::LogFmtAlways("CSV READ: *** file.good() returned false - file not found or not readable: %s ***",
				m_szSerializeFileNameCSV);
			return;
		}

		file.close();
		CTSiDebugTrace::Log("CSV READ: file closed");
	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("CSV READ: *** EXCEPTION caught in _DeSerialize ***");
		return;
	}
}


void 
CHGTCalibrationObjectsContainer::_WriteCalibCSVHeader(FILE* lpCalibCSV)
{
	if(lpCalibCSV)
	{
		fprintf( lpCalibCSV, "LUT ID,AntennaID,Ant Type,Constellation,Latitude (degN),Longitude (deg E)," );
		fprintf( lpCalibCSV, "Altitude (m),FOA Gain,FOA Threshold (Hz),TOA Gain,TOA Threshold (msec)," );
		fprintf( lpCalibCSV, "Last Update Time,FOA Bias (Hz),TOA Bias (msec),FOA Count,  TOAcount,Comments,\n" );
	}
}

void
CHGTCalibrationObjectsContainer::_SaveOldLscalibdata()
{
	std::string line;
	char tmpFile[256];

	//For Read csv file
	memset(m_szSerializeFileNameCSV, 0, sizeof(m_szSerializeFileNameCSV));
	sprintf( m_szSerializeFileNameCSV, "%s%s.csv", cstrCalibDataDir.c_str(), cstrCalibDataFile.c_str() );

	//Write to tmp file
	memset(tmpFile, 0, sizeof(tmpFile));
	sprintf( tmpFile, "%s%s.tmp", cstrCalibDataDir.c_str(), cstrCalibDataFile.c_str() );

	CTSiDebugTrace::LogFmt("CSV BACKUP: copying '%s' -> '%s'", m_szSerializeFileNameCSV, tmpFile);

	try
	{
		//Creating ofstream & ifstream class object
		std::ifstream ini_file(m_szSerializeFileNameCSV);
		std::ofstream out_file(tmpFile);

		if(ini_file && out_file)
		{
			int nLines = 0;
			while(getline(ini_file, line))
			{
				out_file << line << "\n";
				nLines++;
			}
			CTSiDebugTrace::LogFmt("CSV BACKUP: complete - %d lines copied", nLines);
		}
		else
		{
			CTSiDebugTrace::LogFmtAlways("CSV BACKUP: *** failed to open source or destination file (src=%s dst=%s) ***",
				m_szSerializeFileNameCSV, tmpFile);
			THROW_NULL_POINTER_EXCEPTION();
		}

		//Closing file
		ini_file.close();
		out_file.close();
	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("CSV BACKUP: *** EXCEPTION caught in _SaveOldLscalibdata ***");
		return;
	}
}

void
CHGTCalibrationObjectsContainer::_Serialize()
{
	memset(m_szSerializeFileNameCSV, 0, sizeof(m_szSerializeFileNameCSV));
	sprintf( m_szSerializeFileNameCSV, "%s%s.csv", cstrCalibDataDir.c_str(), cstrCalibDataFile.c_str() );

	CTSiDebugTrace::LogFmt("CSV WRITE: opening file for write: %s", m_szSerializeFileNameCSV);

	FILE* lpCalibCSV = NULL;

	try
	{
		lpCalibCSV = fopen( m_szSerializeFileNameCSV, "wt" );
		if(lpCalibCSV == NULL)
		{
			CTSiDebugTrace::LogFmtAlways("CSV WRITE: *** fopen FAILED for '%s' ***", m_szSerializeFileNameCSV);
			THROW_NULL_POINTER_EXCEPTION();
		}

		CTSiDebugTrace::Log("CSV WRITE: file opened, writing header");

		//Write header
		_WriteCalibCSVHeader(lpCalibCSV);

		CHGTCalibrationObj* pCalibObj = NULL;
		HGTLSCALIBDATA *pLsCalibData = NULL;
		int nRowCount = 0;

		m_omapInputCalibObjs.MoveFirst();

		while(pCalibObj = m_omapInputCalibObjs.GetNext())
		{
			pLsCalibData = pCalibObj->GetLsCalibData();

			if(pLsCalibData)
			{
				nRowCount++;
				std::string timeStr = _FormatTime(pLsCalibData->timeLastUpdate);

				CTSiDebugTrace::LogFmt(
					"CSV WRITE row %2d: LutId=%5lu AntId=%3d AntType=%d Const=%3lu "
					"Lat=%8.3f Lon=%9.3f Alt=%7.1f "
					"FOAGain=%.4f FOAThr=%6.1f TOAGain=%.5f TOAThr=%6.1f "
					"LastUpdate=%-22s FOABias=%8.3f TOABias=%12.6f "
					"FOACnt=%5lu TOACnt=%5lu Comments=%s",
					nRowCount,
					pLsCalibData->ulLutId, (int)pLsCalibData->wAntId, (int)pLsCalibData->wAntType,
					pLsCalibData->ulConstellation,
					pLsCalibData->siteLocation.degLatitude, pLsCalibData->siteLocation.degLongitude,
					(double)pLsCalibData->siteLocation.fAltitude,
					pLsCalibData->dFoaGain, pLsCalibData->dFoaThreshold,
					pLsCalibData->dToaGain, pLsCalibData->dToaThreshold,
					timeStr.c_str(),
					pLsCalibData->dFoaBias, pLsCalibData->dToaBias,
					pLsCalibData->ulFoaCount, pLsCalibData->ulToaCount,
					pLsCalibData->arrComments.c_str());

				fprintf( lpCalibCSV, "%d,%d,%d,",
							pLsCalibData->ulLutId, pLsCalibData->wAntId, pLsCalibData->wAntType);
				fprintf( lpCalibCSV, "%d,%8.3f,%8.3f,%8.3f,",
							pLsCalibData->ulConstellation, pLsCalibData->siteLocation.degLatitude,
							pLsCalibData->siteLocation.degLongitude, pLsCalibData->siteLocation.fAltitude);
				fprintf( lpCalibCSV, "%8.3f,%8.3f,%8.3f,",
							pLsCalibData->dFoaGain, pLsCalibData->dFoaThreshold, pLsCalibData->dToaGain);

				fprintf( lpCalibCSV, "%8.3f,%s,%8.3f,",
							pLsCalibData->dToaThreshold, timeStr.c_str(), pLsCalibData->dFoaBias);

				std::string strToaBias = CEMSConversionUtil::ConvertToStringA(pLsCalibData->dToaBias);
				fprintf( lpCalibCSV, "%s,%5d,%5d,%s\n",
							strToaBias.c_str(), pLsCalibData->ulFoaCount, pLsCalibData->ulToaCount,
							pLsCalibData->arrComments.c_str());
			}

			pCalibObj->Release();
			pCalibObj = NULL;
		}

		CTSiDebugTrace::LogFmtAlways("CSV WRITE: complete - wrote %d data rows", nRowCount);
		fclose(lpCalibCSV);
		CTSiDebugTrace::Log("CSV WRITE: file closed");
	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("CSV WRITE: *** EXCEPTION caught in _Serialize ***");
		return;
	}
}

void 
CHGTCalibrationObjectsContainer::_GetLutIdSiteLocation()
{
	CHGTCalibrationObj* pCalibObj = NULL;
	HGTLSCALIBDATA *pLsCalibData = NULL;

	if(m_mapLutIdSitelocation.size() > 0)
		m_mapLutIdSitelocation.clear();

	m_omapInputCalibObjs.MoveFirst();

	while(pCalibObj = m_omapInputCalibObjs.GetNext())
	{
		pLsCalibData = pCalibObj->GetLsCalibData();

		if(pLsCalibData)
		{
			m_mapLutIdSitelocation[pLsCalibData->ulLutId] = pLsCalibData->siteLocation;
		}

		pCalibObj->Release();
		pCalibObj = NULL;
	}
}

EMS_RESULT 
CHGTCalibrationObjectsContainer::CalCulateSatSeparationAngle()
{
	EMS_RESULT hr = EMS_OK;

	EMSTIME timeMsg = CEMSSystemClock::GetTime();

	_GetLutIdSiteLocation();

	if(m_lpOrbit)
	{
		EMSTLEDATA tleData;
		ULONG ulCount = 0;
		EMSVECTORD      vDiff1;
		EMSVECTORD      vDiff2;
		EMSVECTORD      vObserver;
		EMSTIMECOORDD	vCoord1;
		EMSTIMECOORDD	vCoord2;
		CEMSCoordSystem CS;
		HGTSASATData strSatSatData;

		CTSiDebugTrace::LogFmt(
				"ORBIT IN CalCulateSatSeparationAngle: entry, %lu LUTs to process",
				(unsigned long)m_mapLutIdSitelocation.size());

		for (std::map<ULONG, EMSLOCATION>::iterator it=m_mapLutIdSitelocation.begin(); it!=m_mapLutIdSitelocation.end(); ++it)
		{
			ULONG ulLutId = it->first;
			EMSLOCATION location = it->second;
			vObserver = CS.ToEarthFixedDouble( location );

			ULONG ulMinSat = 401;
			//ULONG ulMaxSat = 450;  //For testing
			ULONG ulMaxSat = 650;
			ULONG ulSat3 = 0;

			memset( &strSatSatData, 0, sizeof(strSatSatData));
			strSatSatData.ulSAcount = 0;
			strSatSatData.timeLastUpdate = CEMSSystemClock::GetTime();

			// Satellite Separation Angle Data

			double dSA = 0.0;
			double dMinSatSA = 0.0;

			ULONG ulConstalation1 = 0;
			ULONG ulConstalation2 = 0;

			CTSiDebugTrace::LogFmt(
					"ORBIT IN CalCulateSatSeparationAngle: LutId=%lu starting sat range %lu..%lu",
					ulLutId, ulMinSat, ulMaxSat - 1);

			ULONG ulSat1FailCount = 0;

			for( ULONG ulSat1 = ulMinSat; ulSat1 < ulMaxSat; ulSat1++)
			{
				memset( &vCoord1, 0, sizeof( vCoord1 ) );

				hr = m_lpOrbit->EarthFixedOrbitDouble( ulSat1, &timeMsg, 1, &vCoord1, &ulCount );

				if ( EMS_OK == hr )
				{

					ulConstalation1 = ulSat1/100;
					dMinSatSA = 90;

					ulSat3    = 0;

					vDiff1.dX = vCoord1.coord.radius.dX - vObserver.dX;
					vDiff1.dY = vCoord1.coord.radius.dY - vObserver.dY;
					vDiff1.dZ = vCoord1.coord.radius.dZ - vObserver.dZ;
					vDiff1 = CEMSVector::Normalize( vDiff1 );

					ULONG ulSat2FailCount = 0;

					for( ULONG ulSat2 = ulMinSat; ulSat2 < ulMaxSat; ulSat2++)
					{

						ulConstalation2 = ulSat2/100;
						if ( ( ulConstalation1 == ulConstalation2 ) && (ulSat2 != ulSat1) )
						{
							memset( &vCoord2,0,sizeof(EMSTIMECOORDD ) );
							hr = m_lpOrbit->EarthFixedOrbitDouble( ulSat2, &timeMsg, 1, &vCoord2, &ulCount );

							if ( EMS_OK == hr )
							{

								vDiff2.dX = vCoord2.coord.radius.dX - vObserver.dX;
								vDiff2.dY = vCoord2.coord.radius.dY - vObserver.dY;
								vDiff2.dZ = vCoord2.coord.radius.dZ - vObserver.dZ;
								vDiff2 = CEMSVector::Normalize( vDiff2 );

								dSA = CEMSVector::DotProduct( vDiff1, vDiff2 );

								if( dSA >= 0.0 )
								{
									dSA = acos(dSA)*57.29577951; // convert to degrees
									if ( dSA < dMinSatSA)
									{
										dMinSatSA = dSA;
										ulSat3 = ulSat2;
									}
								}
							}
							else
							{
								ulSat2FailCount++;
								hr = EMS_OK; //testing
							}
						}

					}

					if( ulSat2FailCount > 0 )
					{
						CTSiDebugTrace::LogFmt(
							"ORBIT IN CalCulateSatSeparationAngle: LutId=%lu Sat1=%lu inner-loop EarthFixedOrbit failures=%lu",
							ulLutId, ulSat1, ulSat2FailCount);
					}

					//Store information in memory
					strSatSatData.ulSAsat1[strSatSatData.ulSAcount]  = ulSat1;
					strSatSatData.ulSAsat2[strSatSatData.ulSAcount]  = ulSat3;
					strSatSatData.dSAangle[strSatSatData.ulSAcount] = dMinSatSA;
					strSatSatData.ulSAcount++;
				}
				else
				{
					ulSat1FailCount++;
					hr = EMS_OK; //For testing
				}
			}

			CTSiDebugTrace::LogFmt(
					"ORBIT IN CalCulateSatSeparationAngle: LutId=%lu complete, %lu pairs found, %lu outer-sat orbit failures",
					ulLutId, (unsigned long)strSatSatData.ulSAcount, ulSat1FailCount);

			m_mapLutIdSeparationAngle[ulLutId] = strSatSatData;

			//For logging
			std::string x  = CEMSConversionUtil::ConvertToDateTimeHiResString2A( timeMsg ); 

			FILE* lpTraceSatFile = CHGTFileLogger::GetInstance()->GetTraceFile();
			fprintf(lpTraceSatFile, "10, %s, %d", x.c_str(), ulLutId);
			for( ULONG i = 0; i < strSatSatData.ulSAcount; i++ )
			{
				fprintf(lpTraceSatFile, ",%4d",strSatSatData.ulSAsat1[i]);
			}
			fprintf(lpTraceSatFile, "\n");
			fprintf(lpTraceSatFile, "11, %s, %d", x.c_str(), ulLutId);
			for( ULONG i = 0; i < strSatSatData.ulSAcount; i++ )
			{
				fprintf(lpTraceSatFile, ",%4d",strSatSatData.ulSAsat2[i]);
			}
			fprintf(lpTraceSatFile, "\n");
			fprintf(lpTraceSatFile, "12, %s, %d", x.c_str(), ulLutId);
			for( ULONG i = 0; i < strSatSatData.ulSAcount; i++ )
			{
				fprintf(lpTraceSatFile, ",%4.1f",strSatSatData.dSAangle[i]);
			}
			fprintf(lpTraceSatFile, "\n");

			fflush(lpTraceSatFile);

		} //End of LutIdSitelocation map iter
	}
	else
	{
		// Orbit unavailable
		CTSiDebugTrace::Log("ORBIT IN CalCulateSatSeparationAngle: *** m_lpOrbit is NULL - returning EMS_FALSE ***");
		hr = EMS_FALSE;
	}

	return hr;
}

HGTSASATData 
CHGTCalibrationObjectsContainer::GetSatelliteSeparationAngle(ULONG ulLutId)
{
	HGTSASATData satData;
	memset(&satData, 0, sizeof(satData));

	for (std::map<ULONG, HGTSASATData>::iterator it=m_mapLutIdSeparationAngle.begin(); it!=m_mapLutIdSeparationAngle.end(); ++it)
	{
		if(it->first == ulLutId)
		{
			satData = it->second;
			break;
		}
	}
	return satData;
}