
#ifndef __REF_BCN_H__
#define __REF_BCN_H__

#include "emstypes.H"
#include "EMSBEACN.H"

class RefBcn
{
public:
	RefBcn();
	RefBcn( EMSREFBEACONDATAEX* pRefData, LONG ulCount );
	virtual ~RefBcn(){}

	void SetRefBcnData( EMSREFBEACONDATAEX* pRefData ){ m_lpRefBeaconData = pRefData; }
	void SetRefBcnCount( ULONG ulCount ) { m_ulRefBeaconCount = ulCount;}

	bool IsRefBcn( INT64 i64BeaconID, EMSREFBEACONDATAEX*& pRefBcnData );

	static void SetLUTID( const ULONG ulLutID ){ ms_ulLutID = ulLutID; }
	static ULONG GetLUTID() {return ms_ulLutID;}

	EMSREFBEACONDATAEX* GetRefBcnData(){return m_lpRefBeaconData;}
	ULONG GetRefBcnCount(){return m_ulRefBeaconCount;}

private:

	EMSREFBEACONDATAEX*		m_lpRefBeaconData;
	ULONG					m_ulRefBeaconCount;

	static ULONG			ms_ulLutID;


};

#endif
