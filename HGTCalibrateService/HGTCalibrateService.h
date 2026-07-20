// HGTCalibrateService.h

#ifndef INC_HGTCALIBRATESERVICE
#define INC_HGTCALIBRATESERVICE

#include <stdio.h>
#include "EService.h"			// for CEMSService class
#include "calib406ext.h"		// EMSCALIB406DATAEXT
#include "emsexcpt.h" 			// CEMSException
#include "emsbeacn.h"			// IEMSRefBeaconInfo

#include "HGTRawCalibInput.h"
#include "HGTSPCalibrateEngine.h"
#include "HGTLPCalibrateEngine.h"
#include "HGTLPCalibOutput.h"

class CHGTCalibrateService : public CEMSService
{
public:
	CHGTCalibrateService();
	virtual ~CHGTCalibrateService();

protected:
	virtual EMS_RESULT StartEMSService( DWORD dwArgc, LPTSTR *lpszArgv );
	virtual EMS_RESULT StopEMSService( void );
	virtual EMS_RESULT PauseEMSService( void );
	virtual EMS_RESULT ContinueEMSService( void );

private: // methods

	EMS_RESULT _CreateObjects( void );
	EMS_RESULT _ServiceProcessLoop( void );

	EMS_RESULT _LoadRefBeaconData( void );
	void _ReleaseObjects( void );
	EMS_RESULT _Initialize( void );
	EMS_RESULT _DoIdle();
	bool _IsTimeToSaveSarrData();
	bool _IsTimeToCalculateSA();

private: // data
	HANDLE					m_hEventStop;
	HANDLE					m_hEventSignal;
	BOOL					m_bPaused;

	CEMSException			m_exception;
	CEMSCriticalSection     ms_mtxOutputData;
	IEMSRefBeaconInfo	 	*m_lpRefBeaconObj;
	LONG					m_lRefBeaconCount;
	EMSREFBEACONDATAEX		*m_lpRefBeaconData;
	EMSTIME					m_timeLastSave;
	EMSTIME					m_timeLastSA;

	CHGTRawCalibInput		m_oRawCalibInput;
	CHGTSPCalibrateEngine	m_oSPCalibrateEngine;
	CHGTLPCalibrateEngine	m_oLPCalibrateEngine;
	CHGTLPCalibOutput		m_oLPCalibOutput;
};


#endif