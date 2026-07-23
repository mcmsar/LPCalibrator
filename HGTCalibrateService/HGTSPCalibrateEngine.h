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

#ifndef __SP_CALIBRATE_ENGINE_H__
#define __SP_CALIBRATE_ENGINE_H__

#include "emsthread.h"
#include "calib406ext.h"		// EMSCALIB406DATAEXT
#include "objectmap.h"
#include "RawLpCalibObj.h"
#include "SarrCalibObj.h"
#include "pointerlist.h"
#include "emsexcpt.h" 			// CEMSException
#include "RawSpCalibObj.h"
#include "PointerMap.h"
#include "HGTChannelCalibrationObj.h"
#include <string>
#include <vector>

//! Engine to processes SP Calib406 data.
class CHGTSPCalibrateEngine : public CEMSThread
{
	public:
		CHGTSPCalibrateEngine();
		CHGTSPCalibrateEngine( const CHGTSPCalibrateEngine& x );
		virtual ~CHGTSPCalibrateEngine();

		//! Start processing data.
		EMS_RESULT Start();

		//! Stop processing data.
		void Stop();

		//! Determine whether the server is running.
		bool IsRunning();

		void Init();

		void InitRefBcns(LPEMSREFBEACONDATAEX lpRefBeaconData, int nRefBeaconCount)
		{
			m_oRefBcnData.SetRefBcnData(lpRefBeaconData); 
			m_oRefBcnData.SetRefBcnCount(nRefBeaconCount);
		}

		void AddSpCalib( CEMSPointerList<CEMSRawSpCalibObj>&  lstSpCalibData );


		void GetRawLPCalib( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalib );
		void GetSARRCalib( CEMSPointerList<CEMSSarrCalibObj>& rolstFcal );

	protected:
		virtual void run();
		void _DoIdle();

		EMS_RESULT _CreateObjects( void );
		void _ReleaseObjects( void );
		
		std::string _CreateMapKey( ULONG ulLutId, WORD wAntId, ULONG ulConstellation );
		void _PopulateRawSpCalibObj();
		void _PopulateRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj );
		void _AddToCrossChannelBuffer( CEMSPointerList<CEMSRawSpCalibObj>&  lstNewCalibObj );
		void _ResolveCrossChannelBuffer( CEMSPointerList<CEMSRawSpCalibObj>&  lstReadyCalibObj );
		void _RecordDispatch( ULONG ulLutId, ULONG ulSatId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency );
		bool _IsLateArrivalDuplicate( ULONG ulLutId, ULONG ulSatId, INT64 i64BcnId, INT64 i64ReceiveTimeNanos, double dFrequency );
		void _PurgeRecentDispatchHistory();
		void _PopulateChannelCalibObj( CEMSRawSpCalibObj*  pCalibObj );
		void _PerformSpCalibration();
		void _OutputCalibratedData(CEMSPointerList<CEMSRawLpCalibObj>& olstCalib);
		void _OutputSarrCalibData( CEMSPointerList<CEMSSarrCalibObj>& olstSarrCalib );
		ULONG _GetConstellation( ULONG ulSatId );


	private:

		HANDLE					m_hEventSignal;

		bool					m_bRunning;

		RefBcn					m_oRefBcnData;
		CEMSException			m_exception;
		CEMSCriticalSection     ms_mtxLpCalibData;
		CEMSCriticalSection		ms_mtxFcalData;
		CEMSCriticalSection		ms_mtxSpCalibData;

		//
		CEMSPointerList<CEMSRawSpCalibObj> m_lstInputSpCalib;

		// Cross-channel (cross-antenna) duplicate-detection hold buffer: every
		// incoming record waits here up to c_i64CrossChannelBufferNanos before
		// being routed to a per-channel calibration object, so a duplicate
		// arriving on a later tick still has a chance to be matched. See
		// _ResolveCrossChannelBuffer.
		CEMSPointerList<CEMSRawSpCalibObj> m_lstCrossChannelBuffer;

		// Extended lookback for late-arriving duplicates that miss the live
		// buffer entirely (e.g. one copy took a slower path to the engine and
		// showed up more than the 30s hold window after its partner was
		// already dispatched). Plain value-typed bookkeeping, not the raw
		// ref-counted calib objects - only the fields needed to recognize a
		// late match are kept. See _RecordDispatch/_IsLateArrivalDuplicate.
		struct _RecentDispatch
		{
			ULONG   ulLutId;
			ULONG   ulSatId;
			INT64   i64BcnId;
			INT64   i64ReceiveTimeNanos;
			double  dFrequency;
			EMSTIME timeDispatched;
		};
		std::vector<_RecentDispatch> m_vecRecentDispatchHistory;

		CEMSPointerMap<std::string, CHGTChannelCalibrationObj> m_mapCalibrateChannels;
		CEMSPointerList<CEMSRawLpCalibObj>             m_lstOutputCalibData;
		CEMSPointerList<CEMSSarrCalibObj>            m_lstOutputFcalData;

	private:	
		static const ULONG		ms_culTimeout;
};

#endif