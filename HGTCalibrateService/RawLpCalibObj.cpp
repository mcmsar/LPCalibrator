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


#include "RawLpCalibObj.h"
#include <math.h>

CEMSRawLpCalibObj::CEMSRawLpCalibObj():  CApiObjBase( TEXT("CEMSRawLpCalibObj") )
{
	memset( &m_rawCalibData, 0, sizeof( EMSCALIB406DATAEXT ) );
}

CEMSRawLpCalibObj::CEMSRawLpCalibObj( const CEMSRawLpCalibObj& rawObj )
{
	memcpy( &m_rawCalibData, &rawObj.m_rawCalibData, sizeof( EMSCALIB406DATAEXT ) );
}

CEMSRawLpCalibObj::CEMSRawLpCalibObj( const EMSCALIB406DATAEXT& rawData )
{
	memcpy( &m_rawCalibData, &rawData, sizeof( EMSCALIB406DATAEXT ) );
}

CEMSRawLpCalibObj::~CEMSRawLpCalibObj()
{
}

EMS_RESULT 
CEMSRawLpCalibObj::Initialize( void )
{
	return EMS_OK;
}

EMS_RESULT 
CEMSRawLpCalibObj::Reset( void )
{
	memset( &m_rawCalibData, 0, sizeof( EMSCALIB406DATAEXT ) );
	return EMS_OK;
}

void 
CEMSRawLpCalibObj::ResetStatusFlags( void )
{
	m_rawCalibData.calib406.wStatus &= ~EMS_FLAG_HISTORICAL_MSG; // reset, since new
	m_rawCalibData.calib406.wStatus &= ~EMS_FLAG_PROCESSED_MSG; // reset, since new
}

void 
CEMSRawLpCalibObj::UpdatePassId()
{
	m_rawCalibData.calib406.ulPassID += m_rawCalibData.calib406.hdr.id.ulLutID * 10000;
}

