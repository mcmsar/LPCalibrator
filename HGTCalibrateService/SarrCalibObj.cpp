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


#include "SarrCalibObj.h"
#include <math.h>

CEMSSarrCalibObj::CEMSSarrCalibObj():  CApiObjBase( TEXT("CEMSSarrCalibObj") )
{
	memset( &m_rawSarrCalibData, 0, sizeof( EMSSARRCALIBDATA ) );
}

CEMSSarrCalibObj::CEMSSarrCalibObj( const CEMSSarrCalibObj& rawObj )
{
	memcpy( &m_rawSarrCalibData, &rawObj.m_rawSarrCalibData, sizeof( EMSSARRCALIBDATA ) );
}

CEMSSarrCalibObj::CEMSSarrCalibObj( const EMSSARRCALIBDATA& rawData )
{
	memcpy( &m_rawSarrCalibData, &rawData, sizeof( EMSSARRCALIBDATA ) );
}

CEMSSarrCalibObj::~CEMSSarrCalibObj()
{
}

EMS_RESULT 
CEMSSarrCalibObj::Initialize( void )
{
	return EMS_OK;
}

EMS_RESULT 
CEMSSarrCalibObj::Reset( void )
{
	memset( &m_rawSarrCalibData, 0, sizeof( EMSSARRCALIBDATA ) );
	return EMS_OK;
}



