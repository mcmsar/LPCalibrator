/*********************************************************************
*	              Copyright (c) 2006 by EMS Technologies, Inc.,
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

#ifndef __HGT_CHANNEL_CALIBRATION_H__
#define __HGT_CHANNEL_CALIBRATION_H__

#pragma warning(disable : 4786)

#include "aobjbase.h"
#include "emstypes.h"
#include "emserror.h"
#include "PointerList.h"
#include <set>
#include "HGTCalibrationObj.h"
#include "RefBcns.h"
#include <string>

class CHGTChannelCalibrationObj :	public CApiObjBase
{			
	public:
		CHGTChannelCalibrationObj();
		CHGTChannelCalibrationObj( const CHGTChannelCalibrationObj& Obj );
		~CHGTChannelCalibrationObj();
	
		EMS_RESULT Initialize(RefBcn* pRefBcns);
		EMS_RESULT Reset();

		void AddRawSpCalibObj(CEMSRawSpCalibObj*  pRawCalibObj);
		void PerformSpCalibration( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalibs,
									CEMSPointerList<CEMSSarrCalibObj>& rolstSarrCalib);


		void AddRawLpCalibObj(CEMSRawLpCalibObj*  pRawCalibObj);
		void PerformLpCalibration( CEMSPointerList<CEMSRawLpCalibObj>& rolstCalibs,
										CEMSPointerList<CEMSSarrCalibObj>& rolstSarrCalib);


	protected:
			
			bool _CheckFrameSynchBitErrors( CEMSRawSpCalibObj* pRawSpCalibObj );
			std::string _FormatInputTime( EMSTIME time );
			std::string _CreateKey( CEMSRawSpCalibObj* pRawSpCalibObj );
		
			bool _IsDuplicate( CEMSRawSpCalibObj* pRawSpCalibObj );
			void _SaveInputHistory( CEMSRawSpCalibObj* pCalibObj );

	private:

		RefBcn					*m_pRefBcns;
		//CEMSPointerList<CEMSRawSpCalibObj>  m_lstRawSpCalib;
		CEMSPointerList<CEMSRawLpCalibObj>  m_lstRawLpCalib;

		CEMSCriticalSection     ms_mtxInSpCalibData;

		CEMSPointerMap<std::string, CEMSRawSpCalibObj> m_omapInputHistory;

};

#endif