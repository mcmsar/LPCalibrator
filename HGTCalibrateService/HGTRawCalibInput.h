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

#ifndef __HGT_RAW_CALIB_INPUT_H__
#define __HGT_RAW_CALIB_INPUT_H__

#include "emsthread.h"
#include "emspipe.h"			// Pipe and CmdChannel Objects
#include "pksink.h"				// CEMSPacketSink
#include "pointerlist.h"
#include "emsexcpt.h" 			// CEMSException
#include "RawSpCalibObj.h"
#include "RawLpCalibObj.h"
#include <string>

//For testing
#include "PointerMap.h"

//! Collects Raw Calib406 data.
class CHGTRawCalibInput : public CEMSThread
{
	public:
		CHGTRawCalibInput();
		CHGTRawCalibInput( const CHGTRawCalibInput& x );
		virtual ~CHGTRawCalibInput();

		//! Start processing data.
		EMS_RESULT Start();

		//! Stop processing data.
		void Stop();

		//! Determine whether the server is running.
		bool IsRunning();

		void Init();
		void GetRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstSpCalibObj );
		void GetRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstLpCalibObj );

	protected:
		virtual void run();


		EMS_RESULT _PopulateInputSpCalibData();
		EMS_RESULT _PopulateInputLpCalibData();

		EMS_RESULT _RegisterInputChannelForSPCalib(); 
		EMS_RESULT _RegisterInputChannelForLPCalib(); //TOAFOA
		EMS_RESULT _CreateObjects( void );

		void _UnRegisterInputChannelForSPCalib();
		void _UnRegisterInputChannelForLPCalib(); //TOAFOA
		void _ReleaseObjects( void );

		void _AddRawSpCalibObjList( CEMSPointerList<CEMSRawSpCalibObj>&  lstCalibObj );
		void _AddRawLpCalibObjList( CEMSPointerList<CEMSRawLpCalibObj>&  lstCalibObj );

		std::string _FormatInputTime( EMSTIME time );

	private:

		HANDLE					m_hEventSignal;
		LPEMSDATAPIPELINE		m_lpDataPipeline;
		CEMSPacketSink			*m_lpSPCalibInputData;
		CEMSPacketSink			*m_lpLPCalibInputData;

		bool					m_bRunning;

		CEMSException			m_exception;
		CEMSCriticalSection     ms_mtxSpCalibData;
		CEMSCriticalSection		ms_mtxLpCalibData;

		CEMSPointerList<CEMSRawSpCalibObj> m_lstInputSpCalib;
		CEMSPointerList<CEMSRawLpCalibObj> m_lstInputLpCalib;


	private:	
		static const ULONG		ms_culTimeout;
};

#endif