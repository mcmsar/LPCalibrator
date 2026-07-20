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


#define INITGUID
#include <initguid.h>
#include "LogHelper.h"
#include "EMSCLOCK.H"
#include "HGTSPCalibrateEngine.h"
#include "TSiDebugTrace.h"

#include "calib406msg.h"
#include "ccbootstrap.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

const ULONG CHGTSPCalibrateEngine::ms_culTimeout = 100;		// 100 millisecs


CHGTSPCalibrateEngine::CHGTSPCalibrateEngine() :  m_bRunning(false)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}

CHGTSPCalibrateEngine::CHGTSPCalibrateEngine( const CHGTSPCalibrateEngine& x ) :   m_bRunning(false)
{
	m_hEventSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
}


CHGTSPCalibrateEngine::~CHGTSPCalibrateEngine()
{
	Stop();
}

void CHGTSPCalibrateEngine::_ReleaseObjects()
{
	if ( m_hEventSignal != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hEventSignal );
		m_hEventSignal = INVALID_HANDLE_VALUE;
	}
}

EMS_RESULT
CHGTSPCalibrateEngine::_CreateObjects( void )
{
	EMS_RESULT hr = EMS_OK;

	return hr;
}


EMS_RESULT 
CHGTSPCalibrateEngine::Start()
{
	EMS_RESULT hr = EMS_OK;

	//_CreateObjects();

	if( !m_bRunning )
		start();

	return hr;
}

void 
CHGTSPCalibrateEngine::Stop()
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
CHGTSPCalibrateEngine::IsRunning()
{
	return m_bRunning;
}
		
void 
CHGTSPCalibrateEngine::Init()
{
}


void
CHGTSPCalibrateEngine::AddSpCalib( CEMSPointerList<CEMSRawSpCalibObj>&  lstSpCalib )
{
	
	try
	{
		if( lstSpCalib.Count() > 0 )
		{
			ms_mtxSpCalibData.Enter();

			m_lstInputSpCalib += lstSpCalib;
				
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
CHGTSPCalibrateEngine::_CreateMapKey( ULONG ulLutId, WORD wAntId, ULONG ulConstellation )
{
	// Create the key.
	char szKey[ 128 ];

	sprintf( szKey, "%d_%d_%d", ulLutId, wAntId, ulConstellation);

	std::string oszKey( szKey );
	return oszKey;
}

ULONG
CHGTSPCalibrateEngine::_GetConstellation( ULONG ulSatId )
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
CHGTSPCalibrateEngine::_PopulateChannelCalibObj( CEMSRawSpCalibObj*  pCalibData )
{
	if( !pCalibData )
		return;


	CHGTChannelCalibrationObj* pCalibChannel = NULL;

	try
	{
		WORD wAntID = pCalibData->GetAntennaId();
		ULONG ulConstellation = _GetConstellation( pCalibData->GetSatId() );
		ULONG ulLutID = pCalibData->GetLutId();

		// Check if DBF channel
		if ( wAntID > 99 )
		{
			//WORD wBeamID = wAntID % 100;
			wAntID = (wAntID/100)*100;
			//pCalibData->SetAntennaId( (wAntID);
			ulConstellation = pCalibData->GetSatId();
		}

		std::string szKey = _CreateMapKey( ulLutID, wAntID, ulConstellation );

		if(m_mapCalibrateChannels.IsInMap(szKey))
		{
			pCalibChannel = m_mapCalibrateChannels.Get( szKey );

			pCalibChannel->AddRawSpCalibObj( pCalibData );

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

			pCalibChannel->Initialize( &m_oRefBcnData);

			m_mapCalibrateChannels.Add( szKey, pCalibChannel );

			CTSiDebugTrace::LogFmt("SP ENGINE: new channel created key=%s - adding first record", szKey.c_str());
			pCalibChannel->AddRawSpCalibObj( pCalibData );
			// Do NOT Release here - Add() took ownership of pCalibChannel
		}

	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PopulateChannelCalibObj ***");
		if( pCalibChannel )
		{
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}
		throw;
	}
}

void
CHGTSPCalibrateEngine::_PopulateRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj )
{
	CEMSRawSpCalibObj* pCalibObject = NULL;
	
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
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PopulateRawSpCalibObjList - SP record(s) dropped ***");
		if( pCalibObject )
		{
			pCalibObject->Release();
			pCalibObject = NULL;
		}
	}
}

void
CHGTSPCalibrateEngine::_PopulateRawSpCalibObj()
{
	CEMSPointerList<CEMSRawSpCalibObj>   lstInputCalib;
	try
	{
		ms_mtxSpCalibData.Enter();
		lstInputCalib = m_lstInputSpCalib;
		m_lstInputSpCalib.Clear();
		ms_mtxSpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxSpCalibData.Leave();
		throw;
	}

	_PopulateRawSpCalibObjList( lstInputCalib );
}

void
CHGTSPCalibrateEngine::GetSARRCalib( CEMSPointerList<CEMSSarrCalibObj>& rolstFcalRecs )
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
CHGTSPCalibrateEngine::GetRawLPCalib( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalib )
{
	try
	{
		ms_mtxLpCalibData.Enter();

		rolstCalib = m_lstOutputCalibData;

		m_lstOutputCalibData.Clear();

		ms_mtxLpCalibData.Leave();
	}
	catch( ... )
	{
		ms_mtxLpCalibData.Leave();
		throw;
	}
}


void 
CHGTSPCalibrateEngine::_OutputSarrCalibData( CEMSPointerList<CEMSSarrCalibObj>& olstSarrCalib ) 
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

void 
CHGTSPCalibrateEngine::_OutputCalibratedData( CEMSPointerList<CEMSRawLpCalibObj>& olstCalib ) 
{
	try
	{

		if ( olstCalib.Count() > 0 )
		{
			ms_mtxLpCalibData.Enter();

			m_lstOutputCalibData += olstCalib;

			ms_mtxLpCalibData.Leave();

		}
	}
	catch(...)
	{
		ms_mtxLpCalibData.Leave();
		throw;
	}

}

void
CHGTSPCalibrateEngine::_PerformSpCalibration()
{
	CHGTChannelCalibrationObj* pCalibChannel = NULL;
	try
	{
		CEMSPointerList<CEMSRawLpCalibObj> olstCalib;
		CEMSPointerList<CEMSSarrCalibObj> olstSarrCalib;

		m_mapCalibrateChannels.MoveFirst();
		while( pCalibChannel = m_mapCalibrateChannels.GetNext() )
		{
			CTSiDebugTrace::LogFmt("SP ENGINE: calling channel->PerformSpCalibration ptr=%p", pCalibChannel);
			pCalibChannel->PerformSpCalibration(olstCalib, olstSarrCalib);
			CTSiDebugTrace::LogFmt("SP ENGINE: PerformSpCalibration returned, about to Release channel ptr=%p", pCalibChannel);
			pCalibChannel->Release();
			CTSiDebugTrace::Log("SP ENGINE: channel Release complete");
			pCalibChannel = NULL;
		}
		CTSiDebugTrace::Log("SP ENGINE: channel iteration loop complete");

		_OutputCalibratedData( olstCalib );

		_OutputSarrCalibData( olstSarrCalib );

	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _PerformSpCalibration ***");
		if( pCalibChannel )
		{
			pCalibChannel->Release();
			pCalibChannel = NULL;
		}
		throw;
	}
}


void
CHGTSPCalibrateEngine::_DoIdle()
{
	try
	{

		_PopulateRawSpCalibObj();

		_PerformSpCalibration();

	}
	catch( ... )
	{
		throw;
	}
}

void 
CHGTSPCalibrateEngine::run()
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
								m_exception.SetMark( __LINE__, __FILE__ );
								_DoIdle();
							}
							break;
					}
				}
				catch( CEMSException& e )
				{
					CTSiDebugTrace::LogFmtAlways("*** EXCEPTION in SPCalibrateEngine loop (CEMSException hr=0x%08X) - _ReleaseObjects called ***",
						(unsigned long)e.GetErrCode());
					_ReleaseObjects();
					CEMSLogHelper::LogException( e );
				}
				catch( ... )
				{
					CTSiDebugTrace::LogAlways("*** EXCEPTION in SPCalibrateEngine loop (unknown) - _ReleaseObjects called ***");
					_ReleaseObjects();
					throw;
				}
			}
		}
		catch( CEMSException& e )
		{
			CTSiDebugTrace::LogFmtAlways("*** FATAL EXCEPTION in SPCalibrateEngine run() (CEMSException hr=0x%08X) - thread exiting ***",
				(unsigned long)e.GetErrCode());
			_ReleaseObjects();
			CEMSLogHelper::LogException( e );

			// No point in throwing the exception again.  There's nothing available to catch it.

		}
		catch( ... )
		{
			CTSiDebugTrace::LogAlways("*** FATAL EXCEPTION in SPCalibrateEngine run() (unknown) - thread exiting ***");
			_ReleaseObjects();
			// No point in throwing the exception again.  There's nothing available to catch it.
		}

		m_bRunning = false;

		_ReleaseObjects();
		CoUninitialize();
	}
}
