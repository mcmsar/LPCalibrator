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
#include "EMSCLOCK.H"
#include "HGTRawCalibInput.h"
#include "messages.h"
#include "leodefns.h"
#include "TSiDebugTrace.h"

#include <iostream>

const ULONG CHGTRawCalibInput::ms_culTimeout = 1000;		// 1 sec

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

CHGTRawCalibInput::CHGTRawCalibInput() :  m_bRunning(false), 
												m_lpDataPipeline(NULL),
												m_lpSPCalibInputData(NULL),
												m_lpLPCalibInputData(NULL)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}

CHGTRawCalibInput::CHGTRawCalibInput( const CHGTRawCalibInput& x ) :   m_bRunning(false),
																				m_lpDataPipeline(NULL),
																				m_lpSPCalibInputData(NULL),
																				m_lpLPCalibInputData(NULL)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}


CHGTRawCalibInput::~CHGTRawCalibInput()
{
	Stop();
}

void 
CHGTRawCalibInput::_UnRegisterInputChannelForSPCalib()
{
	if(m_lpDataPipeline)
	{
		if ( m_lpSPCalibInputData )
		{
			HRESULT hr = m_lpDataPipeline->UnRegisterSink( m_lpSPCalibInputData );
			CEMSLogMsgParam	param(1);

			if( SUCCEEDED(hr ) )
			{
				TCHAR szInfo[256] = "";
				sprintf( szInfo, TEXT("UnRegister Sink SP Calib succeeded with  hr = %d"), hr );	
				param.Add( szInfo );
				CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );
			}
			else
			{
				TCHAR szInfo[256] = "";
				sprintf( szInfo, TEXT("UnRegister Sink SP Calib failed with  hr = %d"), hr );		
				param.Add( szInfo );

				CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, param );

			}

			m_lpSPCalibInputData->Release();
			m_lpSPCalibInputData = NULL;
		}
	}
}

void 
CHGTRawCalibInput::_UnRegisterInputChannelForLPCalib()
{
	if(m_lpDataPipeline)
	{
		if ( m_lpLPCalibInputData )
		{
			HRESULT hr = m_lpDataPipeline->UnRegisterSink( m_lpLPCalibInputData );
			CEMSLogMsgParam	param(1);

			if( SUCCEEDED(hr ) )
			{
				TCHAR szInfo[256] = "";
				sprintf( szInfo, TEXT("UnRegister Sink LP Calib succeeded with  hr = %d"), hr );	
				param.Add( szInfo );
				CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, param );

				std::cout<<"TOAFOA channel Unregistered "<<std::endl;
			}
			else
			{
				TCHAR szInfo[256] = "";
				sprintf( szInfo, TEXT("UnRegister Sink LP Calib failed with  hr = %d"), hr );		
				param.Add( szInfo );

				CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, param );

			}

			m_lpLPCalibInputData->Release();
			m_lpLPCalibInputData = NULL;
		}
	}
}

void CHGTRawCalibInput::_ReleaseObjects()
{
	if ( m_hEventSignal != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventSignal );
		m_hEventSignal = INVALID_HANDLE_VALUE;
	}

	_UnRegisterInputChannelForSPCalib();

	_UnRegisterInputChannelForLPCalib();

	if(m_lpDataPipeline)
	{
		m_lpDataPipeline->Release();
		m_lpDataPipeline = NULL;
	}
}

EMS_RESULT
CHGTRawCalibInput::_RegisterInputChannelForSPCalib()
{
	CTSiDebugTrace::Log("RawCalibInput::_RegisterInputChannelForSPCalib: entry");
	EMS_RESULT hr = EMS_OK;
	HRESULT hrRegSink = EMS_OK;

	try
	{
		// Register Input Channel for SP Calib
		m_lpSPCalibInputData = new CEMSPacketSink( LEO_CALIBRATE_406_2_DATA, sizeof( EMSCALIB406DATA2) );

		if ( m_lpSPCalibInputData )
		{
			hr = m_lpSPCalibInputData->SetReceivingProcessID( EMSLOGCAT_Calib406 );
			CTSiDebugTrace::LogHR("RawCalibInput::_RegisterSPCalib: SetReceivingProcessID", hr);
			if ( SUCCEEDED(hr) )
			{
				hr = m_lpDataPipeline->RegisterSink( m_lpSPCalibInputData );
				hrRegSink = hr;
				CTSiDebugTrace::LogHR("RawCalibInput::_RegisterSPCalib: RegisterSink(LEO_CALIBRATE_406_2_DATA)", hr);
			}
		}
		else
		{
			hr = EMS_NO_MEMORY;
			CTSiDebugTrace::Log("RawCalibInput::_RegisterSPCalib: *** new CEMSPacketSink failed (out of memory) ***");
		}

		if( SUCCEEDED(hrRegSink ) )
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Register Sink SP Calib succeeded with  hr = %d"), hr );
			CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, szInfo );

		}
		else
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Register Sink SP Calib failed with  hr = %d"), hr );			

			CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, szInfo );

		}
	}
	catch ( CEMSException ex )
	{
		hr = ex.GetErrCode();

		CEMSLogHelper::LogAlarmMsg( hr , ex.GetSubErrCode() );
	}

	return hr;
}

EMS_RESULT
CHGTRawCalibInput::_RegisterInputChannelForLPCalib()
{
	CTSiDebugTrace::Log("RawCalibInput::_RegisterInputChannelForLPCalib: entry");
	EMS_RESULT hr = EMS_OK;
	HRESULT hrRegSink = EMS_OK;

	try
	{
		// Register Input Channel for TOAFOA
		m_lpLPCalibInputData = new CEMSPacketSink( MEO_CALIBRATE_406EX_DATA, sizeof( EMSCALIB406DATAEXT) );

		if ( m_lpLPCalibInputData )
		{
			hr = m_lpLPCalibInputData->SetReceivingProcessID( EMSLOGCAT_Calib406 );
			CTSiDebugTrace::LogHR("RawCalibInput::_RegisterLPCalib: SetReceivingProcessID", hr);
			if ( SUCCEEDED(hr) )
			{
				hr = m_lpDataPipeline->RegisterSink( m_lpLPCalibInputData );
				hrRegSink = hr;
				CTSiDebugTrace::LogHR("RawCalibInput::_RegisterLPCalib: RegisterSink(MEO_CALIBRATE_406EX_DATA)", hr);
			}
		}
		else
		{
			hr = EMS_NO_MEMORY;
			CTSiDebugTrace::Log("RawCalibInput::_RegisterLPCalib: *** new CEMSPacketSink failed (out of memory) ***");
		}


		if( SUCCEEDED(hrRegSink ) )
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Register Sink LP Calib (TOAFOA) succeeded with  hr = %d"), hr );
			CEMSLogHelper::LogInfoMsg( EMS_CALIB406_TRACER, szInfo );

			std::cout<<"TOAFOA channel registered "<<std::endl;
		}
		else
		{
			TCHAR szInfo[256] = "";
			sprintf( szInfo, TEXT("Register Sink LP Calib(TOAFOA) failed with  hr = %d"), hr );			

			CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_TRACER, szInfo );

		}
	}
	catch ( CEMSException ex )
	{
		hr = ex.GetErrCode();

		CEMSLogHelper::LogAlarmMsg( hr , ex.GetSubErrCode() );
	}

	return hr;
}


EMS_RESULT
CHGTRawCalibInput::_CreateObjects( void )
{
	CTSiDebugTrace::Log("RawCalibInput::_CreateObjects: entry");
	EMS_RESULT hr = EMS_OK;

	try
	{
		hr = CoCreateInstance( CLSID_EMSDataPipeline, 0, CLSCTX_ALL,
						IID_IEMSDataPipeline, (LPVOID*)&m_lpDataPipeline );
		CTSiDebugTrace::LogHR("RawCalibInput::_CreateObjects: CoCreateInstance(CLSID_EMSDataPipeline)", hr);

		if ( ( hr == EMS_OK ) && m_lpDataPipeline )
		{
			hr = _RegisterInputChannelForSPCalib();
			CTSiDebugTrace::LogHR("RawCalibInput::_CreateObjects: _RegisterInputChannelForSPCalib", hr);
		}

		if( ( hr == EMS_OK ) && m_lpDataPipeline )
		{
			hr = _RegisterInputChannelForLPCalib();
			CTSiDebugTrace::LogHR("RawCalibInput::_CreateObjects: _RegisterInputChannelForLPCalib", hr);
		}
	}
	catch ( CEMSException ex )
	{
		hr = ex.GetErrCode();
		CTSiDebugTrace::LogHR("RawCalibInput::_CreateObjects: caught CEMSException", hr);
		CEMSLogHelper::LogAlarmMsg( hr , ex.GetSubErrCode() );
	}

	CTSiDebugTrace::LogHR("RawCalibInput::_CreateObjects: exit", hr);
	return hr;
}


EMS_RESULT
CHGTRawCalibInput::Start()
{
	CTSiDebugTrace::Log("RawCalibInput::Start: entry");
	EMS_RESULT hr = EMS_OK;

	hr = _CreateObjects();
	CTSiDebugTrace::LogHR("RawCalibInput::Start: _CreateObjects", hr);

	if( !m_bRunning  && (EMS_OK == hr))
	{
		CTSiDebugTrace::Log("RawCalibInput::Start: starting thread");
		start();
	}

	return hr;
}

void 
CHGTRawCalibInput::Stop()
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
CHGTRawCalibInput::IsRunning()
{
	return m_bRunning;
}
		
void 
CHGTRawCalibInput::Init()
{
}

void
CHGTRawCalibInput::run()
{
	CTSiDebugTrace::Log("RawCalibInput::run: thread entry");
	EMS_RESULT hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
	CTSiDebugTrace::LogHR("RawCalibInput::run: CoInitializeEx", hr);

	CEMSLogHelper::Init( 0, EMSLOGCAT_Calib406 );

	if( EMS_OK == hr )
	{
		CTSiDebugTrace::Log("RawCalibInput::run: starting event loop");
		m_bRunning = true;

		try
		{
			// Setup the data processing loop.
			
			HANDLE ahEvents[4];
			ahEvents[0] = m_hStopEvent;
			ahEvents[1] = m_hEventSignal;
			ahEvents[2] = m_lpSPCalibInputData->GetEventHandle();
			ahEvents[3] = m_lpLPCalibInputData->GetEventHandle();

			long lHandles = 4;

			bool bRunLoop = true;
			static long s_nRawInputCount = 0;

			while( bRunLoop )
			{
				try
				{
					EMS_RESULT hr = WaitForMultipleObjects( lHandles, ahEvents, FALSE, ms_culTimeout );

					++s_nRawInputCount;
					if(s_nRawInputCount % 5 == 0)
						CTSiDebugTrace::LogFmt("RawCalibInput: heartbeat iteration=%ld (%.0f sec)", s_nRawInputCount, s_nRawInputCount * 1.0);

					switch( hr )
					{
						case WAIT_OBJECT_0:
							{
								CTSiDebugTrace::Log("RawCalibInput: stop event received");
								bRunLoop = false;
							}
							break;

						case WAIT_OBJECT_0 + 1:     //signalled for some other reason
							CTSiDebugTrace::Log("RawCalibInput: signal event (no-op)");
							break;

						case WAIT_OBJECT_0 + 2:		// SPCALIB Records Received
							CTSiDebugTrace::Log("RawCalibInput: SP data event - calling _PopulateInputSpCalibData");
							m_exception.SetMark( __LINE__, __FILE__ );
							hr = _PopulateInputSpCalibData();
							CTSiDebugTrace::Log("RawCalibInput: _PopulateInputSpCalibData returned");
							break;

						case WAIT_OBJECT_0 + 3:		// TOAFOA Records Received
							CTSiDebugTrace::Log("RawCalibInput: LP data event - calling _PopulateInputLpCalibData");
							m_exception.SetMark( __LINE__, __FILE__ );
							hr = _PopulateInputLpCalibData();
							CTSiDebugTrace::Log("RawCalibInput: _PopulateInputLpCalibData returned");
							break;

						case WAIT_TIMEOUT:   // timed out
							CTSiDebugTrace::LogFmt("RawCalibInput: timeout iter=%ld - polling LP then SP", s_nRawInputCount);
							m_exception.SetMark( __LINE__, __FILE__ );
							hr = _PopulateInputLpCalibData();
							CTSiDebugTrace::Log("RawCalibInput: LP poll done, polling SP");
							hr = _PopulateInputSpCalibData();
							CTSiDebugTrace::Log("RawCalibInput: SP poll done");
							break;

						default:
							CTSiDebugTrace::LogFmt("RawCalibInput: WaitForMultipleObjects returned unexpected value 0x%08X", (unsigned)hr);
							break;
					}
				}
				catch( CEMSException& e )
				{
					CTSiDebugTrace::LogFmtAlways("*** EXCEPTION in RawCalibInput loop (CEMSException hr=0x%08X) - _ReleaseObjects called ***",
						(unsigned long)e.GetErrCode());
					_ReleaseObjects();
					CEMSLogHelper::LogException( e );
				}
				catch( ... )
				{
					CTSiDebugTrace::LogAlways("*** EXCEPTION in RawCalibInput loop (unknown) - _ReleaseObjects called ***");
					_ReleaseObjects();
					throw;
				}
			}
		}
		catch( CEMSException& e )
		{
			CTSiDebugTrace::LogFmtAlways("*** FATAL EXCEPTION in RawCalibInput run() (CEMSException hr=0x%08X) - thread exiting ***",
				(unsigned long)e.GetErrCode());
			_ReleaseObjects();
			CEMSLogHelper::LogException( e );
			// No point in throwing the exception again.  There's nothing available to catch it.

		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** FATAL EXCEPTION in RawCalibInput run() (unknown) - thread exiting ***");
			_ReleaseObjects();
			// No point in throwing the exception again.  There's nothing available to catch it.
		}

		m_bRunning = false;

		_ReleaseObjects();
		CoUninitialize();
	}
}

void 
CHGTRawCalibInput::GetRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstSpCalibObj )
{
	try
	{
		ms_mtxSpCalibData.Enter();

		lstSpCalibObj = m_lstInputSpCalib;

		m_lstInputSpCalib.Clear();

		ms_mtxSpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxSpCalibData.Leave();
		throw;
	}
}

void 
CHGTRawCalibInput::_AddRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstSpCalibObj )
{
	try
	{
		if( lstSpCalibObj.Count() > 0 )
		{
			ms_mtxSpCalibData.Enter();

			m_lstInputSpCalib += lstSpCalibObj;
				
			ms_mtxSpCalibData.Leave();
		}
	}
	catch( ... )
	{
		ms_mtxSpCalibData.Leave();
		throw;
	}
}

std::string 
CHGTRawCalibInput::_FormatInputTime( EMSTIME time )
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
CHGTRawCalibInput::_PopulateInputSpCalibData()
{
	EMS_RESULT hr = EMS_FALSE;
	static EMSCALIB406DATA2 calib406Data;
	try
	{
		if ( m_lpSPCalibInputData )
		{
			ULONG ulBytesRead = 0;

			CEMSPointerList<CEMSRawSpCalibObj>  lstRawSpCalibData;

			hr = m_lpSPCalibInputData->Read( (BYTE*)&calib406Data, sizeof( calib406Data ), &ulBytesRead );

			ULONG ulRead = 0;
			while ( hr == EMS_OK && (ulRead < 128) )
			{
				ulRead++;

				if(EMS_OK == hr)
				{
					CEMSRawSpCalibObj *pRawSpCalibObj = new CEMSRawSpCalibObj(calib406Data);
					if(pRawSpCalibObj)
					{
						lstRawSpCalibData.Add(pRawSpCalibObj);

						std::string tStr = _FormatInputTime(calib406Data.timeMsg);
						CTSiDebugTrace::LogFmt(
							"PIPELINE IN SP #%lu: LutId=%5lu SatId=%3lu AntId=%3d "
							"Freq=%12.3f CNR=%5.1f BcnId=%016I64X TimeMsg=%s",
							ulRead,
							calib406Data.hdr.id.ulLutID,
							calib406Data.hdr.id.ulSatID,
							(int)calib406Data.wAntennaID,
							calib406Data.dFrequency,
							calib406Data.dCarrierPower,
							calib406Data.i64BeaconID,
							tStr.c_str());
					}

					pRawSpCalibObj->Release();
					pRawSpCalibObj = NULL;
				}

				hr = m_lpSPCalibInputData->Read( (BYTE*)&calib406Data, sizeof( calib406Data ), &ulBytesRead );
			}

			if(ulRead > 0)
				CTSiDebugTrace::LogFmt("PIPELINE IN SP: batch complete - %lu record(s) queued", ulRead);

			_AddRawSpCalibObjList(lstRawSpCalibData);
		}
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PopulateInputSpCalibData ***");
		throw;
	}

	return hr;
}

void
CHGTRawCalibInput::GetRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstLpCalibObj )
{
	try
	{
		ms_mtxLpCalibData.Enter();

		lstLpCalibObj = m_lstInputLpCalib;

		m_lstInputLpCalib.Clear();

		ms_mtxLpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxLpCalibData.Leave();
		throw;
	}
}

void 
CHGTRawCalibInput::_AddRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstLpCalibObj )
{
	try
	{
		if( lstLpCalibObj.Count() > 0 )
		{
			ms_mtxLpCalibData.Enter();

			m_lstInputLpCalib += lstLpCalibObj;
				
			ms_mtxLpCalibData.Leave();
		}
	}
	catch( ... )
	{
		ms_mtxLpCalibData.Leave();
		throw;
	}
}

EMS_RESULT
CHGTRawCalibInput::_PopulateInputLpCalibData()
{
	EMS_RESULT hr = EMS_FALSE;
	static EMSCALIB406DATAEXT lpCalibData;
	try
	{
		if ( m_lpLPCalibInputData )
		{
			ULONG ulBytesRead = 0;

			CEMSPointerList<CEMSRawLpCalibObj>  lstRawLpCalibData;

			hr = m_lpLPCalibInputData->Read( (BYTE*)&lpCalibData, sizeof( lpCalibData ), &ulBytesRead );

			ULONG ulRead = 0;
			while ( hr == EMS_OK && (ulRead < 128) )
			{
				ulRead++;

				if(EMS_OK == hr)
				{
					CEMSRawLpCalibObj *pRawLpCalibObj = new CEMSRawLpCalibObj(lpCalibData);
					if(pRawLpCalibObj)
					{
						lstRawLpCalibData.Add(pRawLpCalibObj);

						std::string tStr = _FormatInputTime(lpCalibData.calib406.timeMsg);
						CTSiDebugTrace::LogFmt(
							"PIPELINE IN LP #%lu: LutId=%5lu SatId=%3lu AntId=%3d Type=0x%04X "
							"Freq=%12.3f CNR=%5.1f BcnId=%016I64X TimeMsg=%s",
							ulRead,
							lpCalibData.calib406.hdr.id.ulLutID,
							lpCalibData.calib406.hdr.id.ulSatID,
							(int)lpCalibData.calib406.wAntennaID,
							(unsigned)lpCalibData.calib406.hdr.wType,
							lpCalibData.calib406.dFrequency,
							lpCalibData.calib406.dCarrierPower,
							lpCalibData.calib406.i64BeaconID,
							tStr.c_str());
					}

					pRawLpCalibObj->Release();
					pRawLpCalibObj = NULL;
				}

				hr = m_lpLPCalibInputData->Read( (BYTE*)&lpCalibData, sizeof( lpCalibData ), &ulBytesRead );
			}

			if( ulRead > 0 )
			{
				printf("Number LP records = %d (LUT %d)", ulRead, lpCalibData.calib406.hdr.id.ulLutID);
				CTSiDebugTrace::LogFmt("PIPELINE IN LP: batch complete - %lu record(s) queued, last LutId=%lu",
					ulRead, lpCalibData.calib406.hdr.id.ulLutID);
			}

			_AddRawLpCalibObjList(lstRawLpCalibData);
		}
	}
	catch( ... )
	{
		throw;
	}

	return hr;
}