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
#include <vector>
#include <map>
#include <set>

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

const ULONG CHGTLPCalibrateEngine::ms_culTimeout = 100;		// 100 millisecs

// Cross-channel (cross-antenna) duplicate-detection buffer: every incoming LP
// record is held for this long before being routed onward, so a duplicate
// arriving on a later processing tick still gets a chance to be matched
// against it. Two records for the same LUT+beacon ID, whose receive time and
// frequency are within the tolerances below, are treated as the same physical
// detection seen by more than one antenna -- whichever has the higher CNR
// (carrier power) is kept, the other is dropped and never reaches a
// per-channel calibration object. Tune all three based on observed data:
// the hold window trades detection latency for how late a duplicate can
// still arrive and be caught; the tolerances trade false-negative risk
// (missed duplicates) against false-positive risk (distinct beacons wrongly
// merged).
static const INT64  c_i64CrossChannelBufferNanos                = 30000000000;  // 30 s hold window
static const INT64  c_i64CrossChannelReceiveTimeToleranceNanos  = 2000000;       // 2 ms leeway
static const double c_dCrossChannelFrequencyToleranceHz         = 5.0;           // 5 Hz leeway

// A duplicate whose two copies reach the engine more than the 30s hold window
// apart (e.g. one took a slower path upstream) would otherwise be missed
// entirely: the first copy ages out and dispatches as an "unmatched unique"
// before the second ever arrives to compare against. This extended lookback
// remembers what's already been dispatched so a late straggler can still be
// recognized and dropped, rather than being sent through as a second
// "unique" detection of the same event. Unlike the live buffer, a match here
// always drops the late arrival outright (the earlier copy already went out
// and can't be recalled, so there's no "keep the higher CNR" choice left to
// make) - the tradeoff is a small risk of keeping the lower-CNR copy on the
// rare occasion the better one is the late arrival.
static const INT64  c_i64RecentDispatchHistoryNanos             = 300000000000; // 5 min lookback

namespace
{
	struct _LpDupCandidate
	{
		CEMSRawLpCalibObj* pObj;
		ULONG   ulLutId;
		WORD    wAntId;
		INT64   i64BcnId;
		INT64   i64ReceiveTimeNanos;
		double  dFrequency;
		double  dCarrierPower;
		EMSTIME timeStored;
	};

	size_t _UfFind( std::vector<size_t>& vecParent, size_t x )
	{
		while( vecParent[x] != x )
		{
			vecParent[x] = vecParent[ vecParent[x] ];
			x = vecParent[x];
		}
		return x;
	}

	void _UfUnion( std::vector<size_t>& vecParent, size_t a, size_t b )
	{
		size_t ra = _UfFind( vecParent, a );
		size_t rb = _UfFind( vecParent, b );
		if( ra != rb )
			vecParent[ra] = rb;
	}
}


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
CHGTLPCalibrateEngine::_AddToCrossChannelBuffer( CEMSPointerList<CEMSRawLpCalibObj>&  lstNewCalibObj )
{
	CEMSRawLpCalibObj* pObj = NULL;

	try
	{
		if( lstNewCalibObj.Count() == 0 )
			return;

		lstNewCalibObj.MoveFirst();
		while( pObj = lstNewCalibObj.GetNext() )
		{
			// Stamp with the time it entered the hold buffer (reusing the same
			// GetTimeStored()/SetTimeStored() bookkeeping the per-channel
			// retransmit-history map already uses) so its 30s window can be
			// aged against the wall clock later.
			pObj->SetTimeStored( CEMSSystemClock::GetTime() );
			m_lstCrossChannelBuffer.Add( pObj );

			lstNewCalibObj.RemoveCurrent();
			pObj->Release();
			pObj = NULL;
		}
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _AddToCrossChannelBuffer ***");
		if( pObj )
		{
			pObj->Release();
			pObj = NULL;
		}
		throw;
	}
}

void
CHGTLPCalibrateEngine::_ResolveCrossChannelBuffer( CEMSPointerList<CEMSRawLpCalibObj>&  lstReadyCalibObj )
{
	// Best-effort: a failure here must never take the whole LP path down with
	// it, so exceptions are logged and swallowed rather than propagated -
	// worst case, nothing ages out of the buffer this tick and it's retried
	// on the next one.
	try
	{
		if( m_lstCrossChannelBuffer.Count() == 0 )
			return;

		// Snapshot the current buffer contents for matching. GetNext() AddRef()s
		// each object; the buffer itself still holds its own reference, so it's
		// safe to Release() our extra ref right after copying the fields out.
		std::vector<_LpDupCandidate> vecCandidates;
		CEMSRawLpCalibObj* pObj = NULL;

		m_lstCrossChannelBuffer.MoveFirst();
		while( pObj = m_lstCrossChannelBuffer.GetNext() )
		{
			_LpDupCandidate cand;
			cand.pObj               = pObj;
			cand.ulLutId            = pObj->GetLutId();
			cand.wAntId             = pObj->GetAntennaId();
			cand.i64BcnId           = pObj->GetBcnId();
			cand.i64ReceiveTimeNanos = pObj->GetTimeMsg().intTime;
			cand.dFrequency         = pObj->GetFrequency();
			cand.dCarrierPower      = pObj->GetCarrierPower();
			cand.timeStored         = pObj->GetTimeStored();

			vecCandidates.push_back( cand );

			pObj->Release();
			pObj = NULL;
		}

		// Group by LutId+BeaconID ("matching beaconID") first - both are exact
		// matches, so this cheaply narrows down the pairs that then need the
		// receive-time/frequency closeness check. Deliberately NOT keying on
		// SatId: at this point in the pipeline (before _GetDBFSatellite has
		// run) GetSatId() is only a per-antenna provisional tag, not the
		// resolved satellite - it can differ across antennas for the exact
		// same physical detection (confirmed from live data: one beacon, one
		// instant, one frequency, four antennas, four different SatId
		// values). Keying on it here silently split genuine duplicates into
		// separate "unique" buckets instead of collapsing them. Cross-
		// satellite collisions (different satellites relaying the same
		// beacon ID) are still guarded against by the receive-time/frequency
		// tolerances below - different satellites impart different Doppler
		// shifts on the same 406 MHz beacon, so their raw frequencies
		// generally differ by far more than the 5 Hz tolerance.
		std::map<std::string, std::vector<size_t> > mapGroups;
		char szKey[128];
		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			sprintf( szKey, "%lu_%I64X", vecCandidates[i].ulLutId, vecCandidates[i].i64BcnId );
			mapGroups[ std::string(szKey) ].push_back( i );
		}

		// Union-find over candidate indices, so chains of near-identical
		// detections (A close to B, B close to C) collapse into one cluster
		// instead of being resolved as inconsistent pairwise decisions.
		std::vector<size_t> vecParent( vecCandidates.size() );
		for( size_t i = 0; i < vecParent.size(); i++ )
			vecParent[i] = i;

		for( std::map<std::string, std::vector<size_t> >::iterator itGroup = mapGroups.begin(); itGroup != mapGroups.end(); ++itGroup )
		{
			std::vector<size_t>& vecIdx = itGroup->second;
			if( vecIdx.size() < 2 )
				continue;

			for( size_t a = 0; a < vecIdx.size(); a++ )
			{
				for( size_t b = a + 1; b < vecIdx.size(); b++ )
				{
					const _LpDupCandidate& ca = vecCandidates[ vecIdx[a] ];
					const _LpDupCandidate& cb = vecCandidates[ vecIdx[b] ];

					INT64 i64TimeDiff = ca.i64ReceiveTimeNanos - cb.i64ReceiveTimeNanos;
					if( i64TimeDiff < 0 )
						i64TimeDiff = -i64TimeDiff;

					double dFreqDiff = ca.dFrequency - cb.dFrequency;
					if( dFreqDiff < 0.0 )
						dFreqDiff = -dFreqDiff;

					if( i64TimeDiff <= c_i64CrossChannelReceiveTimeToleranceNanos &&
						dFreqDiff  <= c_dCrossChannelFrequencyToleranceHz )
					{
						_UfUnion( vecParent, vecIdx[a], vecIdx[b] );
					}
				}
			}
		}

		// Best (highest-CNR) candidate per cluster.
		std::map<size_t, size_t> mapBestInCluster; // root index -> best index
		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			size_t root = _UfFind( vecParent, i );
			std::map<size_t, size_t>::iterator itBest = mapBestInCluster.find( root );
			if( itBest == mapBestInCluster.end() )
			{
				mapBestInCluster[root] = i;
			}
			else if( vecCandidates[i].dCarrierPower > vecCandidates[ itBest->second ].dCarrierPower )
			{
				itBest->second = i;
			}
		}

		// A cluster is ready to resolve once ANY of its members has been sitting
		// in the buffer for the full 30s hold window - once a match is confirmed
		// there's no reason to wait out the rest. An unmatched (singleton)
		// candidate is ready once it alone reaches 30s old ("after 30s, if there
		// are no duplicates, it is safe to move the detection on").
		EMSTIME timeNow = CEMSSystemClock::GetTime();
		std::set<size_t> setClustersReady;
		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			INT64 i64AgeNanos = timeNow.intTime - vecCandidates[i].timeStored.intTime;
			if( i64AgeNanos >= c_i64CrossChannelBufferNanos )
				setClustersReady.insert( _UfFind( vecParent, i ) );
		}

		if( setClustersReady.empty() )
		{
			_PurgeRecentDispatchHistory();
			return;
		}

		std::set<CEMSRawLpCalibObj*> setSurvivors;
		std::set<CEMSRawLpCalibObj*> setToDrop;

		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			size_t root = _UfFind( vecParent, i );
			if( setClustersReady.find( root ) == setClustersReady.end() )
				continue; // this cluster hasn't reached its 30s window yet

			size_t best = mapBestInCluster[root];
			if( i == best )
			{
				// Before treating this as a fresh detection, check it isn't a
				// late straggler duplicating something already dispatched
				// minutes ago - that pair would never have been in the live
				// buffer together to be caught above.
				if( _IsLateArrivalDuplicate( vecCandidates[i].ulLutId, vecCandidates[i].i64BcnId,
				                              vecCandidates[i].i64ReceiveTimeNanos, vecCandidates[i].dFrequency ) )
				{
					setToDrop.insert( vecCandidates[i].pObj );

					CTSiDebugTrace::LogFmtAlways(
						"LP LATE-ARRIVAL DUP: dropping BcnId=%016I64X LutId=%lu AntId=%u Freq=%.3f CNR=%.2f "
						"(matches a detection already dispatched earlier - missed the live 30s buffer window)",
						vecCandidates[i].i64BcnId, vecCandidates[i].ulLutId,
						(unsigned)vecCandidates[i].wAntId, vecCandidates[i].dFrequency, vecCandidates[i].dCarrierPower );
				}
				else
				{
					setSurvivors.insert( vecCandidates[i].pObj );
				}
			}
			else
			{
				setToDrop.insert( vecCandidates[i].pObj );

				CTSiDebugTrace::LogFmtAlways(
					"LP CROSS-CHANNEL DUP: dropping BcnId=%016I64X LutId=%lu AntId=%u Freq=%.3f CNR=%.2f "
					"(kept AntId=%u Freq=%.3f CNR=%.2f)",
					vecCandidates[i].i64BcnId, vecCandidates[i].ulLutId,
					(unsigned)vecCandidates[i].wAntId, vecCandidates[i].dFrequency, vecCandidates[i].dCarrierPower,
					(unsigned)vecCandidates[best].wAntId, vecCandidates[best].dFrequency, vecCandidates[best].dCarrierPower );
			}
		}

		// Pull every resolved cluster's members out of the buffer; hand the
		// survivors back to the caller for dispatch (recording each one so a
		// still-later straggler can be caught too), drop the rest.
		if( !setSurvivors.empty() || !setToDrop.empty() )
		{
			m_lstCrossChannelBuffer.MoveFirst();
			while( pObj = m_lstCrossChannelBuffer.GetNext() )
			{
				if( setSurvivors.find( pObj ) != setSurvivors.end() )
				{
					_RecordDispatch( pObj->GetLutId(), pObj->GetBcnId(), pObj->GetTimeMsg().intTime, pObj->GetFrequency() );
					lstReadyCalibObj.Add( pObj );
					m_lstCrossChannelBuffer.RemoveCurrent();
				}
				else if( setToDrop.find( pObj ) != setToDrop.end() )
				{
					m_lstCrossChannelBuffer.RemoveCurrent();
				}
				pObj->Release();
				pObj = NULL;
			}
		}

		_PurgeRecentDispatchHistory();
	}
	catch( ... )
	{
		CTSiDebugTrace::LogAlways("*** EXCEPTION in _ResolveCrossChannelBuffer ***");
	}
}

void
CHGTLPCalibrateEngine::_RecordDispatch( ULONG ulLutId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency )
{
	_RecentDispatch entry;
	entry.ulLutId            = ulLutId;
	entry.i64BcnId           = i64BcnId;
	entry.i64ReceiveTimeNanos = i64ReceiveTimeNanos;
	entry.dFrequency         = dFrequency;
	entry.timeDispatched     = CEMSSystemClock::GetTime();

	m_vecRecentDispatchHistory.push_back( entry );
}

bool
CHGTLPCalibrateEngine::_IsLateArrivalDuplicate( ULONG ulLutId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency )
{
	for( size_t i = 0; i < m_vecRecentDispatchHistory.size(); i++ )
	{
		const _RecentDispatch& hist = m_vecRecentDispatchHistory[i];

		if( hist.ulLutId != ulLutId || hist.i64BcnId != i64BcnId )
			continue;

		INT64 i64TimeDiff = i64ReceiveTimeNanos - hist.i64ReceiveTimeNanos;
		if( i64TimeDiff < 0 )
			i64TimeDiff = -i64TimeDiff;

		double dFreqDiff = dFrequency - hist.dFrequency;
		if( dFreqDiff < 0.0 )
			dFreqDiff = -dFreqDiff;

		if( i64TimeDiff <= c_i64CrossChannelReceiveTimeToleranceNanos &&
			dFreqDiff  <= c_dCrossChannelFrequencyToleranceHz )
		{
			return true;
		}
	}

	return false;
}

void
CHGTLPCalibrateEngine::_PurgeRecentDispatchHistory()
{
	if( m_vecRecentDispatchHistory.empty() )
		return;

	EMSTIME timeNow = CEMSSystemClock::GetTime();
	std::vector<_RecentDispatch> vecKept;
	vecKept.reserve( m_vecRecentDispatchHistory.size() );

	for( size_t i = 0; i < m_vecRecentDispatchHistory.size(); i++ )
	{
		INT64 i64AgeNanos = timeNow.intTime - m_vecRecentDispatchHistory[i].timeDispatched.intTime;
		if( i64AgeNanos < c_i64RecentDispatchHistoryNanos )
			vecKept.push_back( m_vecRecentDispatchHistory[i] );
	}

	m_vecRecentDispatchHistory.swap( vecKept );
}

void
CHGTLPCalibrateEngine::_PopulateRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstCalibObj )
{
	CEMSRawLpCalibObj* pCalibObject = NULL;

	try
	{
		_AddToCrossChannelBuffer( lstCalibObj );

		CEMSPointerList<CEMSRawLpCalibObj> lstReadyCalibObj;
		_ResolveCrossChannelBuffer( lstReadyCalibObj );

		if( lstReadyCalibObj.Count() > 0 )
		{
			lstReadyCalibObj.MoveFirst();
			while( pCalibObject = lstReadyCalibObj.GetNext() )
			{
				_PopulateChannelCalibObj( pCalibObject );

				lstReadyCalibObj.RemoveCurrent();

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
