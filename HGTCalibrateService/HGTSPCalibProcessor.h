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

#ifndef __SP_CALIB_PROCESSOR_H__
#define __SP_CALIB_PROCESSOR_H__

#include "emsthread.h"
#include "emspipe.h"			// Pipe and CmdChannel Objects
#include "pksink.h"				// CEMSPacketSink
#include "pointerlist.h"
#include "emsexcpt.h" 			// CEMSException
#include "RawSpCalibObj.h"
#include "HGTSPCalibrateEngine.h"

//! Processes SP Calib406 data.
class CHGTSPCalibProcessor : public CEMSThread
{
	public:
		CHGTSPCalibProcessor();
		CHGTSPCalibProcessor( const CHGTSPCalibProcessor& x );
		virtual ~CHGTSPCalibProcessor();

		//! Start processing data.
		EMS_RESULT Start();

		//! Stop processing data.
		void Stop();

		//! Determine whether the server is running.
		bool IsRunning();

		void Init();
		void GetRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj );

	protected:
		virtual void run();


		EMS_RESULT _PopulateInputSpCalibData();

		EMS_RESULT _CreateObjects( void );
		void _ReleaseObjects( void );
		void _AddRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj );

	private:

		HANDLE					m_hEventSignal;
		LPEMSDATAPIPELINE		m_lpDataPipeline;
		CEMSPacketSink			*m_lpCalib4062InputData;

		bool					m_bRunning;

		CEMSException			m_exception;
		CEMSCriticalSection     ms_mtxInputData;
		CEMSCriticalSection		ms_mtxOutputData;
		CEMSPointerList<CEMSRawSpCalibObj> m_lstInputSpCalib;

	private:	
		static const ULONG		ms_culTimeout;
};

#endif