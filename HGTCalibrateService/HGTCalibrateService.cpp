// HGTCalibrateService.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#pragma warning(disable:4786)
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <errno.h>
#include "emstime.h"
#include "calib406msg.h"
#include "emsclock.h"
#include "LogHelper.h"
#include "ccbootstrap.h"
#include <xercesc/util/PlatformUtils.hpp>

#define INITGUID
#include <initguid.h>

#include "HGTCalibrateService.h"
#include "emscmreg.h"
#include "emsconfg.h"
#include "ConfigManagerProxyClsid.h"

#include "HGTCalibrationObjectsContainer.h"
#include "HGTFileLogger.h"
#include "TSiDebugTrace.h"

const TCHAR c_szServiceName[] 		= TEXT("EMSLS600Calib406");
const ULONG c_ulTimeoutInterval		= 100; 			// i.e. every 100 ms.

FILE*		CHGTFileLogger::m_lpTraceSatFile = NULL;
ULONG		CHGTFileLogger::m_ulRecCount=0;
ULONG		CHGTFileLogger::m_ulSequence=0;
DWORD		CHGTFileLogger::m_dwLastDBFFlush=0;
std::vector<std::string> CHGTFileLogger::m_vecDBFQueue;

FILE*            CTSiDebugTrace::m_lpFile = NULL;
CRITICAL_SECTION CTSiDebugTrace::m_cs;
char             CTSiDebugTrace::m_szFilePath[MAX_PATH] = "";
ULONGLONG        CTSiDebugTrace::m_ullLastPruneTick = 0;
bool             CTSiDebugTrace::m_bDebugEnabled = false;
__int64          CTSiDebugTrace::m_i64RetentionSeconds = 12 * 3600;

CHGTCalibrateService::CHGTCalibrateService()
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
	m_hEventStop = CreateEvent( NULL, FALSE, FALSE, NULL );

	m_lpRefBeaconData = NULL;
	m_lpRefBeaconObj = NULL;
	m_lRefBeaconCount = 0;
	memset(&m_timeLastSave, 0, sizeof(m_timeLastSave));
	memset(&m_timeLastSA, 0, sizeof(m_timeLastSA));
}

CHGTCalibrateService::~CHGTCalibrateService()
{
	if ( m_lpRefBeaconData )
	{
		delete [ ] m_lpRefBeaconData;
		m_lpRefBeaconData = 0;
	}

	CEMSLogHelper::Reset();

	if ( m_hEventSignal != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventSignal );
		m_hEventSignal = INVALID_HANDLE_VALUE;
	}

	if ( m_hEventStop != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventStop );
		m_hEventStop = INVALID_HANDLE_VALUE;
	}

}

EMS_RESULT CHGTCalibrateService::StopEMSService( void )
{
	CTSiDebugTrace::LogFmt("StopEMSService: called (TID=%lu) - setting stop event", GetCurrentThreadId());
	if ( m_hEventStop != INVALID_HANDLE_VALUE )
	{
		SetEvent( m_hEventStop );
	}
	return EMS_OK;
}

EMS_RESULT CHGTCalibrateService::PauseEMSService( void )
{
	if ( !m_bPaused )
	{
		m_bPaused = TRUE;

		SetEvent( m_hEventSignal );
	}
	return EMS_OK; 
}

EMS_RESULT CHGTCalibrateService::ContinueEMSService( void )
{
	if ( m_bPaused )
	{
		m_bPaused = FALSE;

		SetEvent( m_hEventSignal );
	}
	return EMS_OK;
}

EMS_RESULT CHGTCalibrateService::_CreateObjects( void )
{
	CTSiDebugTrace::Log("_CreateObjects: entry");
	EMS_RESULT hr = EMS_OK;

	try
	{
		CEMSLogHelper::Init( 0, EMSLOGCAT_Calib406 );
		CTSiDebugTrace::Log("_CreateObjects: CEMSLogHelper::Init done");

		if( CCBootstrap::IsMEOSystem() )
		{
			CTSiDebugTrace::Log("_CreateObjects: IsMEOSystem=true -> using CLSID_EMSConfigProxy");
			hr = CoCreateInstance( CLSID_EMSConfigProxy, 0, CLSCTX_ALL,
											IID_IEMSRefBeaconInfo, (void**) &m_lpRefBeaconObj);
		}
		else
		{
			CTSiDebugTrace::Log("_CreateObjects: IsMEOSystem=false -> using CLSID_EMSRefBeaconInfo");
			hr = CoCreateInstance( CLSID_EMSRefBeaconInfo, 0, CLSCTX_ALL,
							IID_IEMSRefBeaconInfo, (LPVOID*)&m_lpRefBeaconObj );
		}
		CTSiDebugTrace::LogHR("_CreateObjects: CoCreateInstance(IEMSRefBeaconInfo)", hr);

		if ( hr != EMS_OK )
			throw CEMSException( EMS_CALIB406_REF_BEACON_ERROR, hr );

		//Creating the logger instance
		CHGTFileLogger* filePtr = CHGTFileLogger::GetInstance();
		CTSiDebugTrace::Log("_CreateObjects: CHGTFileLogger::GetInstance() done");
	}

	catch ( CEMSException ex )
	{
		hr = ex.GetErrCode();
		CTSiDebugTrace::LogHR("_CreateObjects: caught CEMSException", hr);
		CEMSLogHelper::LogAlarmMsg( hr , ex.GetSubErrCode() );
	}

	CTSiDebugTrace::LogHR("_CreateObjects: exit", hr);
	return hr;
}

void CHGTCalibrateService::_ReleaseObjects( void )
{

	if ( m_lpRefBeaconObj )
	{
		m_lpRefBeaconObj->Release();
		m_lpRefBeaconObj = 0;
	}
}

EMS_RESULT CHGTCalibrateService::_LoadRefBeaconData( void )
{
	CTSiDebugTrace::Log("_LoadRefBeaconData: entry");
	EMS_RESULT hr = EMS_FALSE;

	if ( m_lpRefBeaconObj )
	{
		ULONG ulCount = 0;

		hr = m_lpRefBeaconObj->GetCount( &ulCount );
		CTSiDebugTrace::LogFmt("_LoadRefBeaconData: GetCount -> hr=0x%08X, count=%lu", (unsigned long)hr, ulCount);

		if ( ( hr == EMS_OK ) && ulCount )
		{
			m_lpRefBeaconData= new EMSREFBEACONDATAEX[ ulCount ];

			if ( m_lpRefBeaconData )
			{
				memset( m_lpRefBeaconData, 0, sizeof( EMSREFBEACONDATAEX ) * ulCount );

				for ( int i = 0; ( i < (int)ulCount) && ( hr == EMS_OK ); i++ )
				{
					hr = m_lpRefBeaconObj->GetIndexedData( i,
								&m_lpRefBeaconData[i].i64BeaconID,
								&m_lpRefBeaconData[i].data );

					if ( hr == EMS_OK )
					{
					 	m_lRefBeaconCount++;
					}
					else
					{
						CTSiDebugTrace::LogFmt("_LoadRefBeaconData: GetIndexedData[%d] failed hr=0x%08X", i, (unsigned long)hr);
					}
				}
				CTSiDebugTrace::LogFmt("_LoadRefBeaconData: loaded %ld of %lu beacon records", m_lRefBeaconCount, ulCount);
			}
			else
			{
				CTSiDebugTrace::Log("_LoadRefBeaconData: *** new EMSREFBEACONDATAEX[] failed (out of memory) ***");
			}
		}
		else
		{
			CTSiDebugTrace::LogFmt("_LoadRefBeaconData: no beacon data to load (hr=0x%08X, count=%lu)", (unsigned long)hr, ulCount);
		}
	}
	else
	{
		CTSiDebugTrace::Log("_LoadRefBeaconData: m_lpRefBeaconObj is NULL, skipping");
	}

	CTSiDebugTrace::LogHR("_LoadRefBeaconData: exit", hr);
	return hr;
}

EMS_RESULT CHGTCalibrateService::_Initialize( void )
{
	CTSiDebugTrace::Log("_Initialize: entry");
	EMS_RESULT hr = EMS_FALSE;

	if (m_lpRefBeaconObj )
	{
		hr = _LoadRefBeaconData( );
		CTSiDebugTrace::LogHR("_Initialize: _LoadRefBeaconData", hr);
	}
	else
	{
		CTSiDebugTrace::Log("_Initialize: m_lpRefBeaconObj is NULL, skipping _LoadRefBeaconData");
	}

	if(EMS_OK == hr)
	{
		m_oSPCalibrateEngine.InitRefBcns(m_lpRefBeaconData, m_lRefBeaconCount);
		CTSiDebugTrace::LogFmt("_Initialize: SPCalibrateEngine.InitRefBcns done (beaconCount=%ld)", m_lRefBeaconCount);
		m_oLPCalibrateEngine.InitRefBcns(m_lpRefBeaconData, m_lRefBeaconCount);
		CTSiDebugTrace::Log("_Initialize: LPCalibrateEngine.InitRefBcns done");
	}

	CTSiDebugTrace::Log("_Initialize: calling CalibrationObjectsContainer::Initialize (loads lscalibdata.csv)");
	hr = CHGTCalibrationObjectsContainer::instance()->Initialize();
	CTSiDebugTrace::LogHR("_Initialize: CalibrationObjectsContainer::Initialize", hr);

	if(hr == EMS_OK)
	{
		//hr = CHGTCalibrationObjectsContainer::instance()->CalCulateSatSeparationAngle();
		m_timeLastSA = CEMSSystemClock::GetTime();
		CTSiDebugTrace::Log("_Initialize: initialization complete");
	}

	CTSiDebugTrace::LogHR("_Initialize: exit", hr);
	return hr;
}

bool 
CHGTCalibrateService::_IsTimeToSaveSarrData()
{
	bool bRet = true;
	EMSTIME timeNow = CEMSSystemClock::GetTime();
	// check  and last saved time
	if( 0 != m_timeLastSave.intTime )
	{
		INT64 i64CheckNanoSeconds = ( ((INT64) 2 ) * ((INT64) 60) * ((INT64) 1000000000) );//2 mins (DEBUG - was 30)
		
		if( (timeNow.intTime - m_timeLastSave.intTime) <= i64CheckNanoSeconds )
		{
			bRet = false;
		}
	}

	return bRet;
}

bool 
CHGTCalibrateService::_IsTimeToCalculateSA()
{
	bool bRet = false;
	EMSTIME timeNow = CEMSSystemClock::GetTime();
	// check  and last SA calculation time
	if( 0 != m_timeLastSA.intTime )
	{
		INT64 i64CheckNanoSeconds = ( ((INT64) 15 ) * ((INT64) 60) * ((INT64) 1000000000) );//15 mins
		
		if( (timeNow.intTime - m_timeLastSA.intTime) >= i64CheckNanoSeconds )
		{
			bRet = true;
		}
	}

	return bRet;
}

EMS_RESULT
CHGTCalibrateService::_DoIdle( void )
{
    EMS_RESULT hr = EMS_OK;

	static long s_nIdleCount = 0;
	++s_nIdleCount;
	if(s_nIdleCount == 1)
		CTSiDebugTrace::Log("_DoIdle: first iteration - main loop is running");
	else if(s_nIdleCount % 30 == 0)  // every ~3 seconds at 100ms interval (DEBUG - was 600/60s)
		CTSiDebugTrace::LogFmt("_DoIdle: heartbeat iteration=%ld (%.0f sec elapsed)", s_nIdleCount, s_nIdleCount * 0.1);

	try
	{
		CTSiDebugTrace::Log("_DoIdle:[a] GetRawSpCalibObjList");
		CEMSPointerList<CEMSRawSpCalibObj>  olstInputRawSpCalib;
		m_oRawCalibInput.GetRawSpCalibObjList(olstInputRawSpCalib);

		if(olstInputRawSpCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[a2] AddSpCalib count=%d", (int)olstInputRawSpCalib.Count());
			m_oSPCalibrateEngine.AddSpCalib(olstInputRawSpCalib);
		}

		CTSiDebugTrace::Log("_DoIdle:[b] GetRawLPCalib(SP)");
		CEMSPointerList<CEMSRawLpCalibObj> olstRawLPCalib;
		m_oSPCalibrateEngine.GetRawLPCalib(olstRawLPCalib);

		if(olstRawLPCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[b2] AddRawLPCalibObjList(SP->LPOut) count=%d", (int)olstRawLPCalib.Count());
			m_oLPCalibOutput.AddRawLPCalibObjList(olstRawLPCalib);
		}

		CTSiDebugTrace::Log("_DoIdle:[c] GetSARRCalib(SP)");
		CEMSPointerList<CEMSSarrCalibObj> olstSarrCalib;
		m_oSPCalibrateEngine.GetSARRCalib(olstSarrCalib);

		if(olstSarrCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[c2] AddSARRCalibObjList(SP->LPOut) count=%d", (int)olstSarrCalib.Count());
			m_oLPCalibOutput.AddSARRCalibObjList(olstSarrCalib);
		}

		//TOAFOA
		CTSiDebugTrace::Log("_DoIdle:[d] GetRawLpCalibObjList");
		CEMSPointerList<CEMSRawLpCalibObj>  olstInputToaFoaCalib;
		m_oRawCalibInput.GetRawLpCalibObjList(olstInputToaFoaCalib);

		if(olstInputToaFoaCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[d2] AddLpCalib(LP->LPEng) count=%d", (int)olstInputToaFoaCalib.Count());
			m_oLPCalibrateEngine.AddLpCalib(olstInputToaFoaCalib);
		}

		CTSiDebugTrace::Log("_DoIdle:[e] GetRawLPCalib(LPEng)");
		CEMSPointerList<CEMSRawLpCalibObj> olstToaFoaCalib;
		m_oLPCalibrateEngine.GetRawLPCalib(olstToaFoaCalib);

		if(olstToaFoaCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[e2] AddRawLPCalibObjList(LPEng->LPOut) count=%d", (int)olstToaFoaCalib.Count());
			m_oLPCalibOutput.AddRawLPCalibObjList(olstToaFoaCalib);
		}

		CTSiDebugTrace::Log("_DoIdle:[f] GetSARRCalib(LPEng)");
		CEMSPointerList<CEMSSarrCalibObj> olstToaFoaSarrCalib;
		m_oLPCalibrateEngine.GetSARRCalib(olstToaFoaSarrCalib);

		if(olstToaFoaSarrCalib.Count() > 0)
		{
			CTSiDebugTrace::LogFmt("_DoIdle:[f2] AddSARRCalibObjList(LPEng->LPOut) count=%d", (int)olstToaFoaSarrCalib.Count());
			m_oLPCalibOutput.AddSARRCalibObjList(olstToaFoaSarrCalib);
		}

		//Update lscalibdata.csv periodically
		CTSiDebugTrace::Log("_DoIdle:[g] CSV timer check");
		if(_IsTimeToSaveSarrData())
		{
			CTSiDebugTrace::LogFmt("_DoIdle: CSV timer fired (iteration=%ld) - calling SaveSarrData", s_nIdleCount);
			CHGTCalibrationObjectsContainer::instance()->SaveSarrData();
			m_timeLastSave = CEMSSystemClock::GetTime();
			CTSiDebugTrace::Log("_DoIdle: SaveSarrData complete");
		}

		//Calculate Separation angle
		CTSiDebugTrace::Log("_DoIdle:[h] SA timer check");
		if(_IsTimeToCalculateSA())
		{
			CHGTCalibrationObjectsContainer::instance()->CalCulateSatSeparationAngle();
			m_timeLastSA = CEMSSystemClock::GetTime();
		}

		CTSiDebugTrace::Log("_DoIdle:[done]");
	}
	catch(...)
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _DoIdle - main loop iteration failed ***");
		hr = EMS_FALSE;
	}
	return hr;
}


EMS_RESULT CHGTCalibrateService::_ServiceProcessLoop( void )
{
	CTSiDebugTrace::Log("_ServiceProcessLoop: entry - starting worker threads");
	EMS_RESULT hr = EMS_OK;

	m_oLPCalibOutput.Start();
	CTSiDebugTrace::Log("_ServiceProcessLoop: LPCalibOutput.Start() done");
	m_oSPCalibrateEngine.Start();
	CTSiDebugTrace::Log("_ServiceProcessLoop: SPCalibrateEngine.Start() done");
	m_oLPCalibrateEngine.Start();
	CTSiDebugTrace::Log("_ServiceProcessLoop: LPCalibrateEngine.Start() done");
	m_oRawCalibInput.Start();
	CTSiDebugTrace::Log("_ServiceProcessLoop: RawCalibInput.Start() done - all threads running, entering main loop");

	HANDLE hEvent[3];
	BOOL bLoop = TRUE;
	DWORD dwCount = 0;

	hEvent[dwCount++] = m_hEventStop;

	hEvent[dwCount++] = m_hEventSignal;


	while ( bLoop )
	{
		hr = WaitForMultipleObjects( dwCount, hEvent, FALSE, c_ulTimeoutInterval );

		try
		{
			switch ( hr )
			{
				case WAIT_OBJECT_0:			// signalled to stop
					CTSiDebugTrace::Log("_ServiceProcessLoop: WAIT_OBJECT_0 - stop event fired, exiting main loop");
					bLoop = FALSE;
					break;

				case WAIT_OBJECT_0 + 1:		// signalled for some other reason
					break;

				case WAIT_TIMEOUT:			// timed out
					m_exception.SetMark( __LINE__, __FILE__ );
					hr = _DoIdle();
					break;

				default:
					CTSiDebugTrace::LogFmt("_ServiceProcessLoop: WaitForMultipleObjects unexpected return 0x%08X (GetLastError=%lu)",
						(unsigned)hr, GetLastError());
					break;
			}
		}

		catch ( CEMSException ex )
		{
			CTSiDebugTrace::LogFmtAlways("*** EXCEPTION in _ServiceProcessLoop (CEMSException) hr=0x%08X ***",
				(unsigned long)ex.GetErrCode());
			CEMSLogHelper::LogException( ex );
		}

		catch ( ... )
		{
			CTSiDebugTrace::LogAlways("*** EXCEPTION in _ServiceProcessLoop (unknown exception) ***");
			CEMSLogHelper::LogException( m_exception );
		}
	}

	CTSiDebugTrace::Log("_ServiceProcessLoop: main loop exited - stopping worker threads");
	m_oRawCalibInput.Stop();
	CTSiDebugTrace::Log("_ServiceProcessLoop: RawCalibInput.Stop() done");
	m_oSPCalibrateEngine.Stop();
	CTSiDebugTrace::Log("_ServiceProcessLoop: SPCalibrateEngine.Stop() done");
	m_oLPCalibrateEngine.Stop();
	CTSiDebugTrace::Log("_ServiceProcessLoop: LPCalibrateEngine.Stop() done");
	m_oLPCalibOutput.Stop();
	CTSiDebugTrace::Log("_ServiceProcessLoop: LPCalibOutput.Stop() done - shutdown complete");

	CHGTCalibrationObjectsContainer::instance()->DeleteInstance();

	return hr;
}

// Static SCM control handler — replaces the base class handler so we can log
// what control code the SCM (or pipeline) sends us just before we die.
// Event handles are cached here from StartEMSService (where they're accessible
// as private members) so the static handler can signal them directly.
static SERVICE_STATUS_HANDLE s_hSvcStatus   = NULL;
static HANDLE                s_hEventStop   = INVALID_HANDLE_VALUE;
static HANDLE                s_hEventSignal = INVALID_HANDLE_VALUE;

static DWORD WINAPI _TrapSCMHandler(DWORD dwControl, DWORD dwEventType,
                                     LPVOID lpEventData, LPVOID lpContext)
{
    CTSiDebugTrace::LogFmt(
        "SCMHandler: control=0x%08X eventType=%lu TID=%lu",
        dwControl, dwEventType, GetCurrentThreadId());

    switch (dwControl)
    {
        case SERVICE_CONTROL_STOP:
            CTSiDebugTrace::Log("SCMHandler: SERVICE_CONTROL_STOP - setting stop event");
            if (s_hEventStop != INVALID_HANDLE_VALUE) SetEvent(s_hEventStop);
            break;
        case SERVICE_CONTROL_PAUSE:
            CTSiDebugTrace::Log("SCMHandler: SERVICE_CONTROL_PAUSE - setting signal event");
            if (s_hEventSignal != INVALID_HANDLE_VALUE) SetEvent(s_hEventSignal);
            break;
        case SERVICE_CONTROL_CONTINUE:
            CTSiDebugTrace::Log("SCMHandler: SERVICE_CONTROL_CONTINUE - setting signal event");
            if (s_hEventSignal != INVALID_HANDLE_VALUE) SetEvent(s_hEventSignal);
            break;
        case SERVICE_CONTROL_INTERROGATE:
            CTSiDebugTrace::Log("SCMHandler: SERVICE_CONTROL_INTERROGATE");
            break;
        default:
            CTSiDebugTrace::LogFmt("SCMHandler: unrecognised control code 0x%08X", dwControl);
            break;
    }
    return NO_ERROR;
}

EMS_RESULT CHGTCalibrateService::StartEMSService( DWORD dwArgc, LPTSTR *lpszArgv )
{
	CTSiDebugTrace::Log("StartEMSService: entry");
	EMS_RESULT hr = EMS_OK;

	ReportServiceStatus( EMS_SERVICE_RUNNING );
	CTSiDebugTrace::Log("StartEMSService: ReportServiceStatus(EMS_SERVICE_RUNNING) done");

	// Cache event handles so _TrapSCMHandler can signal them without needing
	// access to the protected/private members of this class.
	s_hEventStop   = m_hEventStop;
	s_hEventSignal = m_hEventSignal;

	// Replace the base class SCM handler with ours so we can log what control
	// code the SCM (or pipeline) sends us, and route STOP cleanly instead of
	// letting the base class call TerminateProcess.
	s_hSvcStatus = RegisterServiceCtrlHandlerEx(
		c_szServiceName, _TrapSCMHandler, this);
	CTSiDebugTrace::LogFmt("StartEMSService: RegisterServiceCtrlHandlerEx -> hSS=%p",
		(void*)s_hSvcStatus);

	hr = _CreateObjects();
	CTSiDebugTrace::LogHR("StartEMSService: _CreateObjects", hr);

	if ( hr == EMS_OK )
	{
		hr = _Initialize();
		CTSiDebugTrace::LogHR("StartEMSService: _Initialize", hr);

		if ( hr == EMS_OK )
		{
			CEMSLogHelper::LogInfoMsg( EMS_CALIB406_SERVICE_START );
			CTSiDebugTrace::Log("StartEMSService: entering _ServiceProcessLoop (main loop)");

			hr = _ServiceProcessLoop();		// main loop
			CTSiDebugTrace::LogHR("StartEMSService: _ServiceProcessLoop returned", hr);
		}
		else
		{
			CTSiDebugTrace::Log("StartEMSService: _Initialize failed - not entering main loop");
			CEMSLogHelper::LogAlarmMsg( EMS_CALIB406_SERVICE_FAILED );
		}

		CEMSLogHelper::LogInfoMsg( EMS_CALIB406_SERVICE_STOP );
	}

	CTSiDebugTrace::Log("StartEMSService: calling _ReleaseObjects");
	_ReleaseObjects();
	CTSiDebugTrace::LogHR("StartEMSService: exit", hr);
	return hr;
}


static LONG WINAPI _SEHCrashHandler(EXCEPTION_POINTERS* pEI)
{
    DWORD dwCode = pEI->ExceptionRecord->ExceptionCode;
    // Skip C++ exceptions (0xE06D7363) and single-step/breakpoint — only log native faults
    if (dwCode == 0xE06D7363 || dwCode == EXCEPTION_SINGLE_STEP || dwCode == EXCEPTION_BREAKPOINT)
        return EXCEPTION_CONTINUE_SEARCH;
    PVOID pvAddr  = pEI->ExceptionRecord->ExceptionAddress;
    DWORD_PTR dwInfo0 = pEI->ExceptionRecord->NumberParameters > 0
                        ? pEI->ExceptionRecord->ExceptionInformation[0] : 0;
    DWORD_PTR dwInfo1 = pEI->ExceptionRecord->NumberParameters > 1
                        ? pEI->ExceptionRecord->ExceptionInformation[1] : 0;
    CTSiDebugTrace::LogFmtAlways(
        "*** SEH EXCEPTION CAUGHT: code=0x%08X addr=%p TID=%lu info=[%p %p] ***",
        dwCode, pvAddr, GetCurrentThreadId(),
        (PVOID)dwInfo0, (PVOID)dwInfo1);
    return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char* argv[])
{
	CTSiDebugTrace::Open();
	CTSiDebugTrace::Log("main: entry");
	atexit([]() {
		CTSiDebugTrace::Log("atexit: CALLED - process exiting via ExitProcess()/exit(), NOT TerminateProcess()");
	});
	CTSiDebugTrace::Log("main: atexit handler registered");
	AddVectoredExceptionHandler(1, _SEHCrashHandler);  // catch SEH before any C++ handler
	CTSiDebugTrace::LogFmt("main: SEH vectored handler installed (TID=%lu)", GetCurrentThreadId());

	EMS_RESULT hr = CoInitializeEx( 0, COINIT_MULTITHREADED );
	CTSiDebugTrace::LogHR("main: CoInitializeEx(COINIT_MULTITHREADED)", hr);

	xercesc::XMLPlatformUtils::Initialize();
	CTSiDebugTrace::Log("main: XMLPlatformUtils::Initialize() done");

	if (( S_OK == hr ) || ( S_FALSE == hr ) )
	{
		CTSiDebugTrace::Log("main: CoInitializeEx succeeded, creating service object");
		{
			try
			{
				// want nesting so DTOR happens before CoUninitialize
				CHGTCalibrateService calibrationService;
				CTSiDebugTrace::Log("main: CHGTCalibrateService constructed");

				if ( (argc > 1) && (*argv[1] == 'd') )
				{
					CTSiDebugTrace::Log("main: argc>1 and argv[1]='d' - calling RunAsService(FALSE) [debug mode]");
					calibrationService.RunAsService( FALSE );
				}

				CTSiDebugTrace::LogFmt("main: calling Run(\"%s\") - argc=%d", c_szServiceName, argc);
				calibrationService.Run( c_szServiceName, argc, argv );
				CTSiDebugTrace::Log("main: Run() returned");
			}
			catch( ... )
			{
				CTSiDebugTrace::Log("main: *** caught unhandled exception in service block ***");
			}
		}
		CTSiDebugTrace::Log("main: calling CoUninitialize");
		CoUninitialize();
	}
	else
	{
		CTSiDebugTrace::Log("main: CoInitializeEx failed - service not started");
	}

	CTSiDebugTrace::Log("main: XMLPlatformUtils::Terminate()");
	xercesc::XMLPlatformUtils::Terminate();

	CTSiDebugTrace::Log("main: exit");
	CTSiDebugTrace::Close();
	return 0;
}

