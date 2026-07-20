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


#include "RawSpCalibObj.h"
#include <math.h>

CEMSRawSpCalibObj::CEMSRawSpCalibObj():  CApiObjBase( TEXT("CEMSRawSpCalibObj") )
{
	memset( &m_rawSpCalibData, 0, sizeof( EMSCALIB406DATA2 ) );
}

CEMSRawSpCalibObj::CEMSRawSpCalibObj( const CEMSRawSpCalibObj& rawObj )
{
	memcpy( &m_rawSpCalibData, &rawObj.m_rawSpCalibData, sizeof( EMSCALIB406DATA2 ) );
}

CEMSRawSpCalibObj::CEMSRawSpCalibObj( const EMSCALIB406DATA2& rawData )
{
	memcpy( &m_rawSpCalibData, &rawData, sizeof( EMSCALIB406DATA2 ) );
}

CEMSRawSpCalibObj::~CEMSRawSpCalibObj()
{
}

EMS_RESULT 
CEMSRawSpCalibObj::Initialize( void )
{
	return EMS_OK;
}

EMS_RESULT 
CEMSRawSpCalibObj::Reset( void )
{
	memset( &m_rawSpCalibData, 0, sizeof( EMSCALIB406DATA2 ) );
	return EMS_OK;
}

