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



#include <initguid.h>
#include "LogHelper.h"
#include "calib406msg.h"
#include "leodefns.h"
#include "HGTLPCalibOutput.h"
#include "convutility.h"
#include "CBcnMsgDecodeBase.h"
#include "CBeaconID.h"
#include "TSiDebugTrace.h"
#include "HGTCalibrationObjectsContainer.h"

#include <sstream>
#include <stdlib.h>

const ULONG CHGTLPCalibOutput::ms_culTimeout = 100;		// 100 millisecs
const ULONG CHGTLPCalibOutput::ms_culsleepTime = 200; //in milli seconds
const ULONG CHGTLPCalibOutput::ms_culMaxRetries = 10;

bool   CHGTLPCalibOutput::ms_bFoaThresholdOverrideEnabled = false;
double CHGTLPCalibOutput::ms_dFoaThresholdOverride = 0.0;

// Fallback FOA-residual gate used only when no per-antenna calibration
// parameters can be found for a record (see _OutputRawLPCalibData). Matches
// the container's own default dFoaThreshold (HGTCalibrationObjectsContainer.cpp)
// so behavior is unchanged for antennas with no lscalibdata.csv entry yet.
static const double c_dDefaultFoaOutputThresholdHz = 10.0;

// Test-only marker file for _LoadFoaThresholdOverride. Deliberately not next
// to lscalibdata.csv - that file is rewritten wholesale by SaveSarrData on a
// timer, so any value hand-edited there gets overwritten within a couple of
// minutes by whatever the service already has in memory. This file is only
// ever read once, at Start(), so it's immune to that.
static const char c_szFoaThresholdOverridePath[] = "C:\\TSiDebugTrace.txt.foathreshold";

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

CHGTLPCalibOutput::CHGTLPCalibOutput() :  m_bRunning(false),
											m_lpDataPipeline(NULL),
											m_lpCalib406ExOutputData(NULL),
											m_lpSarrCalibData(NULL),
											m_bLastSendSuccessLpCalib(true),
											m_bLastSendSuccessSarrCalib(true)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}

CHGTLPCalibOutput::CHGTLPCalibOutput( const CHGTLPCalibOutput& x ) :   m_bRunning(false),
																		  m_lpDataPipeline(NULL),
																		  m_lpCalib406ExOutputData(NULL),
																		  m_lpSarrCalibData(NULL),
																		  m_bLastSendSuccessLpCalib(true),
																		  m_bLastSendSuccessSarrCalib(true)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}


CHGTLPCalibOutput::~CHGTLPCalibOutput()
{
	Stop();
}

void CHGTLPCalibOutput::_ReleaseObjects()
{
	if ( m_hEventSignal != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventSignal );
		m_hEventSignal = INVALID_HANDLE_VALUE;
	}

	if ( m_lpDataPipeline )
	{

		if ( m_lpCalib406ExOutputData )
		{
			m_lpDataPipeline->UnRegisterSource( m_lpCalib406ExOutputData );
			m_lpCalib406ExOutputData->Release();
			m_lpCalib406ExOutputData = 0;
		}

		if ( m_lpSarrCalibData )
		{
			m_lpDataPipeline->UnRegisterSource( m_lpSarrCalibData );
			m_lpSarrCalibData->Release();
			m_lpSarrCalibData = 0;
		}

		m_lpDataPipeline->Release();
		m_lpDataPipeline = 0;
	}
}

EMS_RESULT
CHGTLPCalibOutput::_CreateObjects( void )
{
	CTSiDebugTrace::Log("LPCalibOutput::_CreateObjects: entry");
	EMS_RESULT hr = EMS_OK;

	try
	{
		hr = CoCreateInstance( CLSID_EMSDataPipeline, 0, CLSCTX_ALL,
						IID_IEMSDataPipeline, (LPVOID*)&m_lpDataPipeline );
		CTSiDebugTrace::LogHR("LPCalibOutput::_CreateObjects: CoCreateInstance(CLSID_EMSDataPipeline)", hr);

		if ( ( hr == EMS_OK ) && m_lpDataPipeline )
		{
			// Register Output Channels
			m_lpCalib406ExOutputData = new CEMSPacketSource( LEO_CALIBRATE_406EX_DATA, sizeof( EMSCALIB406DATAEXT ) );

			if ( m_lpCalib406ExOutputData )
			{
				hr = m_lpDataPipeline->RegisterSource( m_lpCalib406ExOutputData );
				CTSiDebugTrace::LogHR("LPCalibOutput::_CreateObjects: RegisterSource(LEO_CALIBRATE_406EX_DATA)", hr);
				if( SUCCEEDED(hr ) )
				{
					TCHAR szInfo[256] = "";
					sprintf( szInfo, TEXT("Register Source for LEO_CALIBRATE_406EX_DATA succeeded with  hr = %d"), hr );
					CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, szInfo );
				}
				else
				{
					TCHAR szInfo[256] = "";
					sprintf( szInfo, TEXT("Register Source for LEO_CALIBRATE_406EX_DATA failed with  hr = %d"), hr );
					CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, szInfo );
				}
			}
			else
			{
				hr = EMS_NO_MEMORY;
				CTSiDebugTrace::Log("LPCalibOutput::_CreateObjects: *** new CEMSPacketSource(LEO_CALIBRATE_406EX_DATA) failed ***");
			}

			if ( hr != EMS_OK )
				throw CEMSException( EMS_CALIB406_BASE_CODE, hr );

			m_lpSarrCalibData = new CEMSPacketSource( LEO_SARR_CALIB_DATA, sizeof( EMSSARRCALIBDATA ) );

			if ( m_lpSarrCalibData )
			{
				hr = m_lpDataPipeline->RegisterSource( m_lpSarrCalibData );
				CTSiDebugTrace::LogHR("LPCalibOutput::_CreateObjects: RegisterSource(LEO_SARR_CALIB_DATA)", hr);
				if( SUCCEEDED(hr ) )
				{
					TCHAR szInfo[256] = "";
					sprintf( szInfo, TEXT("Register Source for LEO_SARR_CALIB_DATA succeeded with  hr = %d"), hr );
					CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, szInfo );
				}
				else
				{
					TCHAR szInfo[256] = "";
					sprintf( szInfo, TEXT("Register Source for LEO_SARR_CALIB_DATA failed with  hr = %d"), hr );
					CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, szInfo );
				}
			}
			else
			{
				hr = EMS_NO_MEMORY;
				CTSiDebugTrace::Log("LPCalibOutput::_CreateObjects: *** new CEMSPacketSource(LEO_SARR_CALIB_DATA) failed ***");
			}

			if ( hr != EMS_OK )
				throw CEMSException( EMS_CALIB406_BASE_CODE, hr );
		}
	}
	catch ( CEMSException ex )
	{
		hr = ex.GetErrCode();
		CTSiDebugTrace::LogHR("LPCalibOutput::_CreateObjects: caught CEMSException", hr);
		CEMSLogHelper::LogAlarmMsg( hr , ex.GetSubErrCode() );
	}
	catch (... )
	{
		CTSiDebugTrace::Log("LPCalibOutput::_CreateObjects: *** caught unknown exception ***");
		hr = EMS_FALSE;
	}

	CTSiDebugTrace::LogHR("LPCalibOutput::_CreateObjects: exit", hr);
	return hr;
}


EMS_RESULT
CHGTLPCalibOutput::Start()
{
	CTSiDebugTrace::Log("LPCalibOutput::Start: entry");
	EMS_RESULT hr = EMS_OK;

	_LoadFoaThresholdOverride();

	hr = _CreateObjects();
	CTSiDebugTrace::LogHR("LPCalibOutput::Start: _CreateObjects", hr);

	if( !m_bRunning )
	{
		CTSiDebugTrace::Log("LPCalibOutput::Start: starting thread");
		start();
	}

	return hr;
}

// See c_szFoaThresholdOverridePath. Format: the file's contents are a single
// number, e.g. "150" or "150.0", read with the marker's mere presence (not
// its content) governing the debug-log toggle already used elsewhere -
// here the *value* matters, so a malformed/empty file is treated the same
// as a missing one (override left disabled) rather than guessed at.
void
CHGTLPCalibOutput::_LoadFoaThresholdOverride()
{
	ms_bFoaThresholdOverrideEnabled = false;
	ms_dFoaThresholdOverride = 0.0;

	FILE* lpFile = fopen(c_szFoaThresholdOverridePath, "r");
	if( !lpFile )
		return;

	char szBuf[64] = "";
	fgets(szBuf, sizeof(szBuf) - 1, lpFile);
	fclose(lpFile);

	char* pEnd = NULL;
	double dValue = strtod(szBuf, &pEnd);

	if( (pEnd != szBuf) && (dValue > 0.0) )
	{
		ms_bFoaThresholdOverrideEnabled = true;
		ms_dFoaThresholdOverride = dValue;

		CTSiDebugTrace::LogFmtAlways(
			"LP OUTPUT: FOA threshold override ACTIVE - using %.3f Hz for every "
			"record instead of each antenna's configured lscalibdata.csv value "
			"(from %s - delete and restart to disable)",
			dValue, c_szFoaThresholdOverridePath);
	}
	else
	{
		CTSiDebugTrace::LogFmtAlways(
			"LP OUTPUT: %s exists but did not contain a usable positive number "
			"(\"%s\") - override left disabled",
			c_szFoaThresholdOverridePath, szBuf);
	}
}

void 
CHGTLPCalibOutput::Stop()
{
	_ReleaseObjects();

	if( m_bRunning )
	{
		SignalStop();

		if( !wait( 15000 ) )
			stop();
	}
}

bool 
CHGTLPCalibOutput::IsRunning()
{
	return m_bRunning;
}
		
void 
CHGTLPCalibOutput::Init()
{
}


EMS_RESULT 
CHGTLPCalibOutput::_SendLPCalibDataToPipeline(CEMSRawLpCalibObj* pCal)
{
	EMS_RESULT hr = EMS_OK;
	ULONG ulBytesWritten = 0;

	//std:string str = CEMSConversionUtil::ConvertToStringA(pCal->GetLutId() );
	//str += "9";
	//ULONG ulLutId = CEMSConversionUtil::ConvertToULong(str.c_str());

	//pCal->SetLutId(ulLutId); //For testing

	// Check for ELT(DT) - RR June 8, 2022
	EMSCALIB406DATAEXT LPdata;
	memcpy( &LPdata, pCal, sizeof( EMSCALIB406DATAEXT ));
    BYTE b1 = LPdata.calib406.cBeaconMsg[4] >> 1;
    BYTE b2 = LPdata.calib406.cBeaconMsg[5] >> 1;

	if( b1 == 1 && b2 == 1)
	{
		LPdata.calib406.hdr.wType &= 0x8000;
		memcpy( pCal, &LPdata, sizeof( EMSCALIB406DATAEXT ));
	}

 
	hr = m_lpCalib406ExOutputData->Write( (LPBYTE)pCal->GetRawData(), sizeof( EMSCALIB406DATAEXT ), &ulBytesWritten );

	if( EMS_OK != hr )
	{
		CEMSLogMsgParam	param(1);	
		// Don't keep logging the write error. It could be that the pipeline has overflowed.
		if ( m_bLastSendSuccessLpCalib )
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Calib Service - Failed to output records to the pipeline with  hr = %d"), hr );		
			param.Add( szInfo );

			//CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, param );
			CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );
		}
		m_bLastSendSuccessLpCalib = false;
	}
	else
	{
		m_bLastSendSuccessLpCalib = true;
	}


	return hr;
}

void 
CHGTLPCalibOutput::_OutputRawLPCalibData(CEMSPointerList<CEMSRawLpCalibObj>& olstCalibRecs)
{

	CEMSRawLpCalibObj* pCal = NULL;
	try
	{

		EMS_RESULT hr = EMS_OK;
		if( (olstCalibRecs.Count() > 0) && m_lpCalib406ExOutputData )
		{
			olstCalibRecs.MoveFirst();
			while( pCal = olstCalibRecs.GetNext() )
			{
				// Send to pipeline.
				BOOL bDone = FALSE;
				int nRetries = 0;
				int nChannel = LEO_CALIBRATE_406EX_DATA;
				EMSDATACHANNELINFO info;
				int bufavaialble = 0;
				while ( !bDone )
				{
					hr = EMS_OK;
					double dFreqRes = pCal->GetResidualFrequency();
					WORD wType = pCal->GetType();

					// The residual is only ever non-zero for reference-beacon records
					// (see CHGTCalibrationObj::CalibrateSpRawObject - _ComputeResidual
					// runs only when a ref beacon was matched); ordinary passes always
					// clear this gate. Use the antenna's own configured FOA threshold
					// (lscalibdata.csv "FOA Threshold (Hz)") rather than a fixed value,
					// so this gate tracks whatever the site has actually been tuned to
					// accept instead of silently rejecting everything once true bias
					// drifts past a hardcoded number.
					double dFoaOutputThreshold;
					if( ms_bFoaThresholdOverrideEnabled )
					{
						dFoaOutputThreshold = ms_dFoaThresholdOverride;
					}
					else
					{
						HGTLSCALIBDATA lsCalibData = HGTLSCALIBDATA();
						lsCalibData.dFoaThreshold = c_dDefaultFoaOutputThresholdHz;
						CHGTCalibrationObjectsContainer::instance()->GetCalibrationParameters(
							pCal->GetLutId(), pCal->GetAntennaId(), pCal->GetSatId(), &lsCalibData);
						dFoaOutputThreshold = lsCalibData.dFoaThreshold;
					}

					if (fabs(dFreqRes) < dFoaOutputThreshold)
					{
						hr = _SendLPCalibDataToPipeline(pCal);
					}
					else
					{
						// Always-logged: this record is otherwise dropped with zero trace,
						// which made "reference beacons aren't showing up in LP Calibration"
						// unanswerable from the log alone.
						CTSiDebugTrace::LogFmtAlways(
							"LP OUTPUT: DROPPED BcnId=%016I64X LutId=%lu AntId=%u SatId=%lu "
							"FreqResidual=%.3f threshold=%.3f",
							pCal->GetBcnId(), pCal->GetLutId(), (unsigned)pCal->GetAntennaId(),
							pCal->GetSatId(), dFreqRes, dFoaOutputThreshold);
					}

					switch ( hr )
					{
						case EMS_PIPE_OVERFLOW:
							hr = m_lpDataPipeline->GetChannelInfo(nChannel, &info);
							if( (EMS_OK == hr) && info.bValidChannel)
							{
								bufavaialble = (int)info.wMaxBuffers - (int)info.wHighBuffers;

								if (  (bufavaialble <= 2)  &&  ( nRetries++ < ms_culMaxRetries ) )
								{

									CEMSLogMsgParam param;
									std::stringstream ss;
									ss << nRetries;

									std::stringstream ssChannel;
									ssChannel << nChannel;

									std::stringstream ssSleepTime;
									ssSleepTime << ms_culsleepTime;

									std::string retStr = " Pipeline Overflowed for channel: ";
									retStr += ssChannel.str();
									retStr += " will sleep for : ";
									retStr += ssSleepTime.str();
									retStr += " millisecs with retry count : ";
									retStr += ss.str();

									param.Add(retStr.c_str()); 
									CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );

									Sleep( ms_culsleepTime ); 
								}
								else
									bDone = TRUE;
							}
							break;

						default:
							bDone = TRUE;
							break;
					}
				}

				pCal->Release();
				pCal = NULL;
			}
		}
		if( !m_lpCalib406ExOutputData )
		{
			wchar_t  wszMsg[256];
			memset( wszMsg, 0, sizeof(wszMsg) );
			swprintf( wszMsg, L"Calib Service - Pipeline channel 34 is not initialized." );
			CEMSLogHelper::LogMsg( EMSLogMsgWarn, EMSLOGMSGTYPESTR_WARNING_1, wszMsg );
		}

	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _OutputRawLPCalibData ***");
		if( pCal )
		{
			pCal->Release();
			pCal = NULL;
		}
		wchar_t  wszMsg[256];
		memset( wszMsg, 0, sizeof(wszMsg) );
		swprintf( wszMsg, L"Calib Service - Failed to output records to the pipeline ... exception" );
		CEMSLogHelper::LogMsg( EMSLogMsgWarn, EMSLOGMSGTYPESTR_WARNING_1, wszMsg );
	}

}

EMS_RESULT 
CHGTLPCalibOutput::_SendSarrCalibDataToPipeline(CEMSSarrCalibObj* pFcal)
{
	EMS_RESULT hr = EMS_OK;
	ULONG ulBytesWritten = 0;

	//std:string str = CEMSConversionUtil::ConvertToStringA(pFcal->GetLutId() );
	//str += "9";
	//ULONG ulLutId = CEMSConversionUtil::ConvertToULong(str.c_str());

	//pFcal->SetLutId(ulLutId); //For testing

	hr = m_lpSarrCalibData->Write( (LPBYTE)pFcal->GetRawData(), sizeof( EMSSARRCALIBDATA ), &ulBytesWritten );

	if( EMS_OK != hr )
	{
		CEMSLogMsgParam	param(1);	
		// Don't keep logging the write error. It could be that the pipeline has overflowed.
		if ( m_bLastSendSuccessSarrCalib )
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Calib Service - Failed to output records to the pipeline with  hr = %d"), hr );		
			param.Add( szInfo );

			//CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, param );
			CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );
		}
		m_bLastSendSuccessSarrCalib = false;
	}
	else
	{
		m_bLastSendSuccessSarrCalib = true;
	}


	return hr;
}

void 
CHGTLPCalibOutput::_OutputSARRCalibData(CEMSPointerList<CEMSSarrCalibObj>& olstCalibRecs)
{

	CEMSSarrCalibObj* pFcal = NULL;
	try
	{
		ULONG ulBytesWritten = 0;
		EMS_RESULT hr = EMS_OK;
		if( (olstCalibRecs.Count() > 0) && m_lpSarrCalibData )
		{
			olstCalibRecs.MoveFirst();
			while( pFcal = olstCalibRecs.GetNext() )
			{
				// Send to pipeline.
				BOOL bDone = FALSE;
				int nRetries = 0;
				int nChannel = LEO_SARR_CALIB_DATA;
				EMSDATACHANNELINFO info;
				int bufavaialble = 0;
				while ( !bDone )
				{
					hr = _SendSarrCalibDataToPipeline(pFcal);

					switch ( hr )
					{
						case EMS_PIPE_OVERFLOW:
							hr = m_lpDataPipeline->GetChannelInfo(nChannel, &info);
							if( (EMS_OK == hr) && info.bValidChannel)
							{
								bufavaialble = (int)info.wMaxBuffers - (int)info.wHighBuffers;

								if (  (bufavaialble <= 2)  &&  ( nRetries++ < ms_culMaxRetries ) )
								{

									CEMSLogMsgParam param;
									std::stringstream ss;
									ss << nRetries;

									std::stringstream ssChannel;
									ssChannel << nChannel;

									std::stringstream ssSleepTime;
									ssSleepTime << ms_culsleepTime;

									std::string retStr = " Pipeline Overflowed for channel: ";
									retStr += ssChannel.str();
									retStr += " will sleep for : ";
									retStr += ssSleepTime.str();
									retStr += " millisecs with retry count : ";
									retStr += ss.str();

									param.Add(retStr.c_str()); 
									CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );

									Sleep( ms_culsleepTime ); 
								}
								else
									bDone = TRUE;
							}
							break;

						default:
							bDone = TRUE;
							break;
					}
				}

				pFcal->Release();
				pFcal = NULL;
			}
		}
		if( !m_lpSarrCalibData )
		{
			wchar_t  wszMsg[256];
			memset( wszMsg, 0, sizeof(wszMsg) );
			swprintf( wszMsg, L"Calib Service - Pipeline channel 29 is not initialized." );
			CEMSLogHelper::LogMsg( EMSLogMsgWarn, EMSLOGMSGTYPESTR_WARNING_1, wszMsg );
		}
		
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _OutputSARRCalibData ***");
		if( pFcal )
		{
			pFcal->Release();
			pFcal = NULL;
		}
		wchar_t  wszMsg[256];
		memset( wszMsg, 0, sizeof(wszMsg) );
		swprintf( wszMsg, L"Calib Service - Failed to output records to the pipeline 29 ... exception" );
		CEMSLogHelper::LogMsg( EMSLogMsgWarn, EMSLOGMSGTYPESTR_WARNING_1, wszMsg );
	}
}


void 
CHGTLPCalibOutput::AddRawLPCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  rolstLpRawCalib )
{
	try
	{
		if( rolstLpRawCalib.Count() > 0 )
		{
			ms_mtxLpcalibData.Enter();

			m_olstLpRawCalib += rolstLpRawCalib;
				
			ms_mtxLpcalibData.Leave();
		}
	}
	catch( ... )
	{
		ms_mtxLpcalibData.Leave();
		throw;
	}
}

void
CHGTLPCalibOutput::AddSARRCalibObjList( CEMSPointerList<CEMSSarrCalibObj>&  rolstSarrCalib )
{
	try
	{
		if( rolstSarrCalib.Count() > 0 )
		{
			ms_mtxSarrCalibData.Enter();

			m_olstSarrCalib += rolstSarrCalib;
				
			ms_mtxSarrCalibData.Leave();
		}
	}
	catch( ... )
	{
		ms_mtxSarrCalibData.Leave();
		throw;
	}
}


void 
CHGTLPCalibOutput::_GetRawLPCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalib )
{
	try
	{
		ms_mtxLpcalibData.Enter();

		rolstCalib = m_olstLpRawCalib;

		m_olstLpRawCalib.Clear();

		ms_mtxLpcalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxLpcalibData.Leave();
		throw;
	}
}

void 
CHGTLPCalibOutput::_GetSARRCalibObjList( CEMSPointerList<CEMSSarrCalibObj>& rolstFcal )
{
	try
	{
		ms_mtxSarrCalibData.Enter();

		rolstFcal = m_olstSarrCalib;

		m_olstSarrCalib.Clear();

		ms_mtxSarrCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxSarrCalibData.Leave();
		throw;
	}
}


void
CHGTLPCalibOutput::_DoIdle()
{
	try
	{
		CTSiDebugTrace::Log("LPOut:[a] _GetRawLPCalibObjList");
		CEMSPointerList<CEMSRawLpCalibObj> olstCalib;
		_GetRawLPCalibObjList( olstCalib );
		if( olstCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("LPCalibOutput::_DoIdle: sending %d LP records", (int)olstCalib.Count());
			_OutputRawLPCalibData(olstCalib);
			CTSiDebugTrace::Log("LPCalibOutput::_DoIdle: _OutputRawLPCalibData complete");
		}

		CTSiDebugTrace::Log("LPOut:[b] _GetSARRCalibObjList");
		CEMSPointerList<CEMSSarrCalibObj> olstFcal;
		_GetSARRCalibObjList( olstFcal );
		if( olstFcal.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("LPCalibOutput::_DoIdle: sending %d SARR records", (int)olstFcal.Count());
			_OutputSARRCalibData(olstFcal);
			CTSiDebugTrace::Log("LPCalibOutput::_DoIdle: _OutputSARRCalibData complete");
		}

		CTSiDebugTrace::Log("LPOut:[done]");
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in LPCalibOutput::_DoIdle ***");
		//throw;
	}
}

void
CHGTLPCalibOutput::run()
{
	CTSiDebugTrace::Log("LPCalibOutput::run: thread entry");
	EMS_RESULT hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
	CTSiDebugTrace::LogHR("LPCalibOutput::run: CoInitializeEx", hr);

	CEMSLogHelper::Init( 0, EMSLOGCAT_Calib406 );

	if( EMS_OK == hr )
	{
		CTSiDebugTrace::Log("LPCalibOutput::run: starting event loop");
		m_bRunning = true;

		try
		{
			// Setup the data processing loop.
			HANDLE ahEvents[3];
			ahEvents[0] = m_hStopEvent;
			ahEvents[1] = 0;
			ahEvents[2] = 0;

			long lHandles = 1;

			bool bRunLoop = true;
			static long s_nLPOutCount = 0;

			while( bRunLoop )
			{
				try
				{
					EMS_RESULT hr = WaitForMultipleObjects( lHandles, ahEvents, FALSE, ms_culTimeout );

					switch( hr )
					{
						case WAIT_OBJECT_0:
							{
								bRunLoop = false;
							}
							break;
						case WAIT_TIMEOUT:   // timed out
						default:
							{
								++s_nLPOutCount;
								if(s_nLPOutCount % 10 == 0)
									CTSiDebugTrace::LogFmt("LPCalibOutput: alive iteration=%ld (%.0f sec)", s_nLPOutCount, s_nLPOutCount * 0.1);
								m_exception.SetMark( __LINE__, __FILE__ );
								_DoIdle();
							}
							break;
					}
				}
				catch( CEMSException& e )
				{
					CTSiDebugTrace::LogFmtAlways("*** EXCEPTION in LPCalibOutput loop (CEMSException hr=0x%08X) - _ReleaseObjects called ***",
						(unsigned long)e.GetErrCode());
					_ReleaseObjects();
					CEMSLogHelper::LogException( e );
				}
				catch( ... )
				{
					CTSiDebugTrace::LogAlways("*** EXCEPTION in LPCalibOutput loop (unknown) - _ReleaseObjects called ***");
					_ReleaseObjects();
					throw;
				}
			}
		}
		catch( CEMSException& e )
		{
			CTSiDebugTrace::LogFmtAlways("*** FATAL EXCEPTION in LPCalibOutput run() (CEMSException hr=0x%08X) - thread exiting ***",
				(unsigned long)e.GetErrCode());
			_ReleaseObjects();
			CEMSLogHelper::LogException( e );

			// No point in throwing the exception again.  There's nothing available to catch it.

		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** FATAL EXCEPTION in LPCalibOutput run() (unknown) - thread exiting ***");
			_ReleaseObjects();
			// No point in throwing the exception again.  There's nothing available to catch it.
		}

		m_bRunning = false;

		_ReleaseObjects();
		CoUninitialize();
	}
}
