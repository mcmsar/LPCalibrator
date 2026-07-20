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
#include "HGTLPCalibrateEngine.h"
#include "TSiDebugTrace.h"
#include "calib406msg.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

const ULONG CHGTLPCalibrateEngine::ms_culTimeout = 100;		// 100 millisecs


CHGTLPCalibrateEngine::CHGTLPCalibrateEngine() :  m_bRunning(false)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}

CHGTLPCalibrateEngine::CHGTLPCalibrateEngine( const CHGTLPCalibrateEngine& x ) :   m_bRunning(false)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}


CHGTLPCalibrateEngine::~CHGTLPCalibrateEngine()
{
	Stop();
}

void CHGTLPCalibrateEngine::_ReleaseObjects()
{
	if ( m_hEventSignal != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventSignal );
		m_hEventSignal = INVALID_HANDLE_VALUE;
	}
}

EMS_RESULT
CHGTLPCalibrateEngine::_CreateObjects( void )
{
	EMS_RESULT hr = EMS_OK;

	return hr;
}


EMS_RESULT 
CHGTLPCalibrateEngine::Start()
{
	EMS_RESULT hr = EMS_OK;

	//_CreateObjects();

	if( !m_bRunning )
		start();

	return hr;
}

void 
CHGTLPCalibrateEngine::Stop()
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
CHGTLPCalibrateEngine::IsRunning()
{
	return m_bRunning;
}
		
void 
CHGTLPCalibrateEngine::Init()
{
}


void
CHGTLPCalibrateEngine::AddLpCalib( CEMSPointerList<CEMSRawLpCalibObj>&  lstLpCalib )
{
	
	try
	{
		if( lstLpCalib.Count() > 0 )
		{
			ms_mtxInLpCalibData.Enter();

			m_lstInputLpCalib += lstLpCalib;
				
			ms_mtxInLpCalibData.Leave();
		}
	}
	catch( ... )
	{
		ms_mtxInLpCalibData.Leave();
		throw;
	}
}

std::string
CHGTLPCalibrateEngine::_CreateMapKey( ULONG ulLutId, WORD wAntId, ULONG ulConstellation )
{
	// Create the key.
	char szKey[ 128 ];

	sprintf( szKey, "%d_%d_%d", ulLutId, wAntId, ulConstellation);

	std::string oszKey( szKey );
	return oszKey;
}

void
CHGTLPCalibrateEngine::_PopulateChannelCalibObj( CEMSRawLpCalibObj*  pCalibData )
{
	if( !pCalibData )
		return;


	CHGTChannelCalibrationObj* pCalibChannel = NULL;

	try
	{
		ULONG ulConstellation = _GetConstellation( pCalibData->GetSatId() );	
		std::string szKey = _CreateMapKey( pCalibData->GetLutId(), pCalibData->GetAntennaId(), ulConstellation );

		if(m_mapCalibrateChannels.IsInMap(szKey))
		{
			pCalibChannel = m_mapCalibrateChannels.Get( szKey );
			pCalibChannel->AddRawLpCalibObj( pCalibData );
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}
		else
		{
			pCalibChannel = new CHGTChannelCalibrationObj();
			if( !pCalibChannel )
			{
				THROW_NOMEMORY_EXCEPTION();
			}

			pCalibChannel->Initialize(&m_oRefBcnData);

			m_mapCalibrateChannels.Add( szKey, pCalibChannel );
		}

			//pCalibChannel->Release();
			//pCalibChannel = NULL;

	}
	catch( ... )
	{
		if( pCalibChannel )
		{
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}
		throw;
	}
}

void
CHGTLPCalibrateEngine::_PopulateRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstCalibObj )
{
	CEMSRawLpCalibObj* pCalibObject = NULL;
	
	try
	{
		if( lstCalibObj.Count() > 0 )
		{
			lstCalibObj.MoveFirst();
			while( pCalibObject = lstCalibObj.GetNext() )
			{
				_PopulateChannelCalibObj( pCalibObject );

				lstCalibObj.RemoveCurrent();
				
				pCalibObject->Release();
				pCalibObject = NULL;
			}
		}
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PopulateRawLpCalibObjList - LP record(s) dropped ***");
		if( pCalibObject )
		{
			pCalibObject->Release();
			pCalibObject = NULL;
		}
	}
}

void
CHGTLPCalibrateEngine::_PopulateRawLpCalibObj()
{
	CEMSPointerList<CEMSRawLpCalibObj>   lstInputCalib;
	try
	{
		ms_mtxInLpCalibData.Enter();
		lstInputCalib = m_lstInputLpCalib;
		m_lstInputLpCalib.Clear();
		ms_mtxInLpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxInLpCalibData.Leave();
		throw;
	}

	_PopulateRawLpCalibObjList( lstInputCalib );
}

void
CHGTLPCalibrateEngine::GetSARRCalib( CEMSPointerList<CEMSSarrCalibObj>& rolstFcalRecs )
{
	try
	{
		ms_mtxFcalData.Enter();

		rolstFcalRecs = m_lstOutputFcalData;

		m_lstOutputFcalData.Clear();

		ms_mtxFcalData.Leave();
	}
	catch( ... )
	{
		ms_mtxFcalData.Leave();
		throw;
	}
}

void 
CHGTLPCalibrateEngine::_OutputSarrCalibData( CEMSPointerList<CEMSSarrCalibObj>& olstSarrCalib ) 
{
	try
	{

		if ( olstSarrCalib.Count() > 0 )
		{
			ms_mtxFcalData.Enter();

			m_lstOutputFcalData += olstSarrCalib;

			ms_mtxFcalData.Leave();

		}
	}
	catch(...)
	{
		ms_mtxFcalData.Leave();
		throw;
	}

}

ULONG
CHGTLPCalibrateEngine::_GetConstellation( ULONG ulSatId )
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

void
CHGTLPCalibrateEngine::GetRawLPCalib( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalib )
{
	try
	{
		ms_mtxOutLpCalibData.Enter();

		rolstCalib = m_lstOutputCalibData;

		m_lstOutputCalibData.Clear();

		ms_mtxOutLpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxOutLpCalibData.Leave();
		throw;
	}
}


void 
CHGTLPCalibrateEngine::_OutputCalibratedData( CEMSPointerList<CEMSRawLpCalibObj>& olstCalib ) 
{
	try
	{

		if ( olstCalib.Count() > 0 )
		{
			ms_mtxOutLpCalibData.Enter();

			m_lstOutputCalibData += olstCalib;

			ms_mtxOutLpCalibData.Leave();

		}
	}
	catch(...)
	{
		ms_mtxOutLpCalibData.Leave();
		throw;
	}

}

void
CHGTLPCalibrateEngine::_PerformLpCalibration()
{
	CHGTChannelCalibrationObj* pCalibChannel = NULL;
	try
	{
		CEMSPointerList<CEMSRawLpCalibObj> olstCalib;
		CEMSPointerList<CEMSSarrCalibObj> olstSarrCalib;

		ULONG ulChannelCount = m_mapCalibrateChannels.Count();
		if(ulChannelCount > 0)
			CTSiDebugTrace::LogFmt("LPEng: _PerformLpCalibration channels=%lu", ulChannelCount);

		m_mapCalibrateChannels.MoveFirst();
		while( pCalibChannel = m_mapCalibrateChannels.GetNext() )
		{
			CTSiDebugTrace::LogFmt("LPEng: PerformLpCalibration ptr=%p", pCalibChannel);
			pCalibChannel->PerformLpCalibration(olstCalib, olstSarrCalib);
			CTSiDebugTrace::LogFmt("LPEng: PerformLpCalibration done, releasing ptr=%p", pCalibChannel);
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}

		_OutputCalibratedData( olstCalib );

		_OutputSarrCalibData( olstSarrCalib );

	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PerformLpCalibration ***");
		if( pCalibChannel )
		{
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}
		throw;
	}
}


void
CHGTLPCalibrateEngine::_DoIdle()
{
	try
	{
		CTSiDebugTrace::Log("LPEng:[a] _PopulateRawLpCalibObj");
		_PopulateRawLpCalibObj();
		CTSiDebugTrace::Log("LPEng:[b] _PerformLpCalibration");
		_PerformLpCalibration();
		CTSiDebugTrace::Log("LPEng:[done]");
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in LPCalibrateEngine::_DoIdle ***");
		throw;
	}
}

void 
CHGTLPCalibrateEngine::run()
{
	EMS_RESULT hr = CoInitializeEx( 0, COINIT_MULTITHREADED );

	CEMSLogHelper::Init( 0, EMSLOGCAT_Calib406 );

	if( EMS_OK == hr )
	{
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
			static long s_nLPEngCount = 0;

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
								++s_nLPEngCount;
								if(s_nLPEngCount % 10 == 0)
									CTSiDebugTrace::LogFmt("LPCalibrateEngine: alive iteration=%ld (%.0f sec)", s_nLPEngCount, s_nLPEngCount * 0.1);
								m_exception.SetMark( __LINE__, __FILE__ );
								_DoIdle();
							}
							break;
					}
				}
				catch( CEMSException& e )
				{
					CTSiDebugTrace::LogFmtAlways("*** EXCEPTION in LPCalibrateEngine loop (CEMSException hr=0x%08X) - _ReleaseObjects called ***",
						(unsigned long)e.GetErrCode());
					_ReleaseObjects();
					CEMSLogHelper::LogException( e );
				}
				catch( ... )
				{
					CTSiDebugTrace::LogAlways("*** EXCEPTION in LPCalibrateEngine loop (unknown) - _ReleaseObjects called ***");
					_ReleaseObjects();
					throw;
				}
			}
		}
		catch( CEMSException& e )
		{
			CTSiDebugTrace::LogFmtAlways("*** FATAL EXCEPTION in LPCalibrateEngine run() (CEMSException hr=0x%08X) - thread exiting ***",
				(unsigned long)e.GetErrCode());
			_ReleaseObjects();
			CEMSLogHelper::LogException( e );

			// No point in throwing the exception again.  There's nothing available to catch it.

		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** FATAL EXCEPTION in LPCalibrateEngine run() (unknown) - thread exiting ***");
			_ReleaseObjects();
			// No point in throwing the exception again.  There's nothing available to catch it.
		}

		m_bRunning = false;

		_ReleaseObjects();
		CoUninitialize();
	}
}
