#include "RefBcns.h"

RefBcn::RefBcn(): m_lpRefBeaconData(NULL), m_ulRefBeaconCount(0L)
{
}

RefBcn::RefBcn( EMSREFBEACONDATAEX* pRefData, LONG ulCount )
{
	m_lpRefBeaconData = pRefData;
	m_ulRefBeaconCount = ulCount;
}

bool
RefBcn::IsRefBcn( INT64 i64BeaconID, EMSREFBEACONDATAEX*& pRefBcnData )
{
	bool bRefBcn = false;
	pRefBcnData = NULL;

	if( m_lpRefBeaconData && (m_ulRefBeaconCount > 0 ) )
	{
		for( LONG i = 0; i < m_ulRefBeaconCount; i++ )
		{
			if( m_lpRefBeaconData[i].i64BeaconID == i64BeaconID )
			{
				pRefBcnData = &m_lpRefBeaconData[i];
				bRefBcn = true;
				break;
			}
		}
	}
	return bRefBcn;
}
