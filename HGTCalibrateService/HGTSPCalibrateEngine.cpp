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
#include <vector>
#include <map>
#include <set>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

const ULONG CHGTSPCalibrateEngine::ms_culTimeout = 100;		// 100 millisecs

// Cross-channel (cross-antenna) duplicate-detection buffer. See
// CHGTLPCalibrateEngine::_ResolveCrossChannelBuffer for the LP-side
// counterpart and the full rationale - same rule, applied one stage
// earlier so cross-channel duplicates never reach a per-channel
// calibration object on the SP side either.
static const INT64  c_i64CrossChannelBufferNanos                = 30000000000;  // 30 s hold window

// The duplicate rule: two detections are the same detection when they share
// a LutId and a BeaconID, their receive times are within 0.5 s of each other,
// and their frequencies are within 2 Hz of each other.
//
// Note these are NOT the same quantity as the hold window above. The hold
// window is wall-clock time a record spends waiting in the buffer for a
// partner to show up; the tolerances below compare the detections' own
// reported receive time and frequency against each other.
//
// These are tight on purpose: live data shows real cross-antenna duplicates
// landing at dT=0.000s / dF=0.000-0.091Hz, so this window catches them with
// room to spare while staying far away from the ~8 Hz per 30 s Doppler drift
// that separates one beacon burst from the next.
static const INT64  c_i64DuplicateReceiveTimeToleranceNanos     = 500000000;   // 0.5 s
static const double c_dDuplicateFrequencyToleranceHz            = 2.0;         // 2 Hz

// See CHGTLPCalibrateEngine::_IsLateArrivalDuplicate for the LP-side
// counterpart and the full rationale: a duplicate whose two copies reach the
// engine more than the 30s hold window apart would otherwise be missed
// entirely, since the first ages out and dispatches before the second ever
// arrives to compare against. This extended lookback remembers what's
// already been dispatched so a late straggler can still be recognized and
// dropped (always dropped outright on a match here - the earlier copy
// already went out and can't be recalled, so there's no CNR comparison left
// to make).
static const INT64  c_i64RecentDispatchHistoryNanos             = 300000000000; // 5 min lookback

namespace
{
	struct _SpDupCandidate
	{
		CEMSRawSpCalibObj* pObj;
		ULONG   ulLutId;
		WORD    wAntId;
		INT64   i64BcnId;
		INT64   i64ReceiveTimeNanos;
		double  dFrequency;
		double  dCarrierPower;
		EMSTIME timeStored;
	};

	// The duplicate rule in one place, so the live-buffer pass and the
	// late-arrival lookback can never drift apart.
	bool _IsDuplicatePair( ULONG ulLutIdA, INT64 i64BcnIdA, INT64 i64RxA, double dFreqA,
	                       ULONG ulLutIdB, INT64 i64BcnIdB, INT64 i64RxB, double dFreqB )
	{
		if( ulLutIdA != ulLutIdB || i64BcnIdA != i64BcnIdB )
			return false;

		INT64 i64TimeDiff = i64RxA - i64RxB;
		if( i64TimeDiff < 0 )
			i64TimeDiff = -i64TimeDiff;

		double dFreqDiff = dFreqA - dFreqB;
		if( dFreqDiff < 0.0 )
			dFreqDiff = -dFreqDiff;

		return ( i64TimeDiff <= c_i64DuplicateReceiveTimeToleranceNanos &&
		         dFreqDiff  <= c_dDuplicateFrequencyToleranceHz );
	}

	bool _IsDuplicatePair( const _SpDupCandidate& a, const _SpDupCandidate& b )
	{
		return _IsDuplicatePair( a.ulLutId, a.i64BcnId, a.i64ReceiveTimeNanos, a.dFrequency,
		                         b.ulLutId, b.i64BcnId, b.i64ReceiveTimeNanos, b.dFrequency );
	}

	// Orders candidate indices oldest-received first, so clustering grows
	// forward in time and doesn't depend on the order records happened to
	// land in the hold buffer.
	struct _ByReceiveTime
	{
		const std::vector<_SpDupCandidate>* pVec;

		_ByReceiveTime( const std::vector<_SpDupCandidate>& vec ) : pVec( &vec ) {}

		bool operator()( size_t a, size_t b ) const
		{
			return (*pVec)[a].i64ReceiveTimeNanos < (*pVec)[b].i64ReceiveTimeNanos;
		}
	};
}


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
CHGTSPCalibrateEngine::_AddToCrossChannelBuffer( CEMSPointerList<CEMSRawSpCalibObj>&  lstNewCalibObj )
{
	CEMSRawSpCalibObj* pObj = NULL;

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
CHGTSPCalibrateEngine::_ResolveCrossChannelBuffer( CEMSPointerList<CEMSRawSpCalibObj>&  lstReadyCalibObj )
{
	// Best-effort: a failure here must never take the whole SP path down with
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
		std::vector<_SpDupCandidate> vecCandidates;
		CEMSRawSpCalibObj* pObj = NULL;

		m_lstCrossChannelBuffer.MoveFirst();
		while( pObj = m_lstCrossChannelBuffer.GetNext() )
		{
			_SpDupCandidate cand;
			cand.pObj               = pObj;
			cand.ulLutId            = pObj->GetLutId();
			cand.wAntId             = pObj->GetAntennaId();
			cand.i64BcnId           = pObj->GetBeaconID();
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
		// generally differ by far more than the 2 Hz tolerance.
		std::map<std::string, std::vector<size_t> > mapGroups;
		char szKey[128];
		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			sprintf( szKey, "%lu_%I64X", vecCandidates[i].ulLutId, vecCandidates[i].i64BcnId );
			mapGroups[ std::string(szKey) ].push_back( i );
		}

		// Cluster the candidates inside each group. A candidate joins an
		// existing cluster only if it is a duplicate of EVERY member already
		// in it (complete linkage), so every pair inside a cluster genuinely
		// satisfies the rule.
		//
		// Deliberately not a transitive/union-find merge: chaining A-B and
		// B-C would put A and C in one cluster even when they are two
		// tolerance-widths apart and so not duplicates of each other, and a
		// long enough chain could swallow genuinely distinct bursts of the
		// same beacon and throw away real detections. Complete linkage keeps
		// that impossible regardless of how the tolerances are later tuned.
		std::vector<size_t> vecClusterId( vecCandidates.size(), 0 );
		std::vector< std::vector<size_t> > vecClusterMembers;

		for( std::map<std::string, std::vector<size_t> >::iterator itGroup = mapGroups.begin(); itGroup != mapGroups.end(); ++itGroup )
		{
			std::vector<size_t>& vecIdx = itGroup->second;

			std::sort( vecIdx.begin(), vecIdx.end(), _ByReceiveTime( vecCandidates ) );

			std::vector<size_t> vecGroupClusters;   // cluster ids opened for this group

			for( size_t a = 0; a < vecIdx.size(); a++ )
			{
				const size_t idx = vecIdx[a];
				bool bPlaced = false;

				for( size_t c = 0; c < vecGroupClusters.size() && !bPlaced; c++ )
				{
					std::vector<size_t>& vecMembers = vecClusterMembers[ vecGroupClusters[c] ];

					bool bMatchesAll = true;
					for( size_t m = 0; m < vecMembers.size() && bMatchesAll; m++ )
					{
						if( !_IsDuplicatePair( vecCandidates[idx], vecCandidates[ vecMembers[m] ] ) )
							bMatchesAll = false;
					}

					if( bMatchesAll )
					{
						vecClusterId[idx] = vecGroupClusters[c];
						vecMembers.push_back( idx );
						bPlaced = true;
					}
				}

				if( !bPlaced )
				{
					vecClusterId[idx] = vecClusterMembers.size();
					vecGroupClusters.push_back( vecClusterMembers.size() );
					vecClusterMembers.push_back( std::vector<size_t>( 1, idx ) );
				}
			}
		}

		// Best (highest-CNR) candidate per cluster.
		std::map<size_t, size_t> mapBestInCluster; // cluster id -> best index
		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			size_t cluster = vecClusterId[i];
			std::map<size_t, size_t>::iterator itBest = mapBestInCluster.find( cluster );
			if( itBest == mapBestInCluster.end() )
			{
				mapBestInCluster[cluster] = i;
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
				setClustersReady.insert( vecClusterId[i] );
		}

		if( setClustersReady.empty() )
		{
			_PurgeRecentDispatchHistory();
			return;
		}

		std::set<CEMSRawSpCalibObj*> setSurvivors;
		std::set<CEMSRawSpCalibObj*> setToDrop;

		for( size_t i = 0; i < vecCandidates.size(); i++ )
		{
			size_t cluster = vecClusterId[i];
			if( setClustersReady.find( cluster ) == setClustersReady.end() )
				continue; // this cluster hasn't reached its 30s window yet

			size_t best = mapBestInCluster[cluster];
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
						"SP LATE-ARRIVAL DUP: dropping BcnId=%016I64X LutId=%lu AntId=%u Freq=%.3f CNR=%.2f "
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

				INT64 i64TimeDelta = vecCandidates[i].i64ReceiveTimeNanos - vecCandidates[best].i64ReceiveTimeNanos;

				CTSiDebugTrace::LogFmtAlways(
					"SP CROSS-CHANNEL DUP: dropping BcnId=%016I64X LutId=%lu AntId=%u Freq=%.3f CNR=%.2f "
					"(kept AntId=%u Freq=%.3f CNR=%.2f; dT=%.3fs dF=%.3fHz)",
					vecCandidates[i].i64BcnId, vecCandidates[i].ulLutId,
					(unsigned)vecCandidates[i].wAntId, vecCandidates[i].dFrequency, vecCandidates[i].dCarrierPower,
					(unsigned)vecCandidates[best].wAntId, vecCandidates[best].dFrequency, vecCandidates[best].dCarrierPower,
					(double)i64TimeDelta * 1e-9, vecCandidates[i].dFrequency - vecCandidates[best].dFrequency );
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
					_RecordDispatch( pObj->GetLutId(), pObj->GetBeaconID(), pObj->GetTimeMsg().intTime, pObj->GetFrequency() );
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
CHGTSPCalibrateEngine::_RecordDispatch( ULONG ulLutId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency )
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
CHGTSPCalibrateEngine::_IsLateArrivalDuplicate( ULONG ulLutId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency )
{
	for( size_t i = 0; i < m_vecRecentDispatchHistory.size(); i++ )
	{
		const _RecentDispatch& hist = m_vecRecentDispatchHistory[i];

		if( _IsDuplicatePair( ulLutId, i64BcnId, i64ReceiveTimeNanos, dFrequency,
		                      hist.ulLutId, hist.i64BcnId, hist.i64ReceiveTimeNanos, hist.dFrequency ) )
		{
			return true;
		}
	}

	return false;
}

void
CHGTSPCalibrateEngine::_PurgeRecentDispatchHistory()
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
CHGTSPCalibrateEngine::_PopulateRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj )
{
	CEMSRawSpCalibObj* pCalibObject = NULL;

	try
	{
		_AddToCrossChannelBuffer( lstCalibObj );

		CEMSPointerList<CEMSRawSpCalibObj> lstReadyCalibObj;
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
