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

#ifndef __HGT_LP_CALIB_OUTPUT_H__
#define __HGT_LP_CALIB_OUTPUT_H__

#include "emsthread.h"
#include "emsexcpt.h" 			// CEMSException
#include "CriticalSection.h"
#include "RawLpCalibObj.h"
#include "pointerlist.h"
#include "emspipe.h"			// Pipe and CmdChannel Objects
#include "pksink.h"				// CEMSPacketSink
#include "pksource.h"			// CEMSPacketSource
#include "sarrcalib.h"
#include "SarrCalibObj.h"

//! Engine to Output LP Calib406 data.
class CHGTLPCalibOutput : public CEMSThread
{
	public:
		CHGTLPCalibOutput();
		CHGTLPCalibOutput( const CHGTLPCalibOutput& x );
		virtual ~CHGTLPCalibOutput();

		//! Start processing data.
		EMS_RESULT Start();

		//! Stop processing data.
		void Stop();

		//! Determine whether the server is running.
		bool IsRunning();

		void Init();

		void AddRawLPCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  rolstRawLPCalib );
		void AddSARRCalibObjList( CEMSPointerList<CEMSSarrCalibObj>&  rolstSarrCalib );


	protected:
		virtual void run();
		void _DoIdle();

		EMS_RESULT _CreateObjects( void );
		void _ReleaseObjects( void );

		// Test-only override for the FOA output gate in _OutputRawLPCalibData.
		// Read once at Start() from a marker file (see .cpp for path/format) so
		// it can't be silently reverted by the periodic lscalibdata.csv
		// round-trip (SaveSarrData/_Serialize rewrites that file from
		// in-memory state on a timer and never re-reads it, so hand edits to
		// the CSV while the service is running get overwritten within a
		// couple of minutes). Delete the marker file and restart to go back
		// to the normal per-antenna configured threshold.
		static void _LoadFoaThresholdOverride();
		
		void _GetRawLPCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>& rolstRawLPCalib );
		void _GetSARRCalibObjList( CEMSPointerList<CEMSSarrCalibObj>& rolstSarrCalib );

		void _OutputRawLPCalibData(CEMSPointerList<CEMSRawLpCalibObj>& olstCalib);
		void _OutputSARRCalibData(CEMSPointerList<CEMSSarrCalibObj>& olstCalib);

		EMS_RESULT _SendLPCalibDataToPipeline(CEMSRawLpCalibObj* pCal);
		EMS_RESULT _SendSarrCalibDataToPipeline(CEMSSarrCalibObj* pFcal);

	private:

		HANDLE					m_hEventSignal;
		bool					m_bRunning;
		bool					m_bLastSendSuccessLpCalib;
		bool					m_bLastSendSuccessSarrCalib;

		CEMSException			m_exception;
		CEMSCriticalSection     ms_mtxLpcalibData;
		CEMSCriticalSection     ms_mtxSarrCalibData;
		LPEMSDATAPIPELINE		m_lpDataPipeline;
		CEMSPacketSource		*m_lpCalib406ExOutputData;
		CEMSPacketSource		*m_lpSarrCalibData;
		CEMSPointerList<CEMSRawLpCalibObj> m_olstLpRawCalib;
		CEMSPointerList<CEMSSarrCalibObj>  m_olstSarrCalib;

	private:	
		static const ULONG		ms_culTimeout;
		static const ULONG		ms_culsleepTime;
		static const ULONG		ms_culMaxRetries;

		static bool				ms_bFoaThresholdOverrideEnabled;
		static double			ms_dFoaThresholdOverride;
};

#endif