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

#ifndef __HGTFileLogger_H__
#define __HGTFileLogger_H__

#include <vector>
#include <string>

class CHGTFileLogger
{
	public:
		static CHGTFileLogger* GetInstance(){static CHGTFileLogger obj;return &obj;}

		static FILE*	GetTraceFile()
		{
			if((m_ulRecCount == 0) || (m_ulRecCount > 50000))
			{
				char	szLogFile[ 256 ];
				memset( szLogFile,      0, sizeof( szLogFile ) );
				sprintf( szLogFile, "D:\\DBFSatTrace\\EMSTraceSat_%d.csv", m_ulSequence );
				if(m_lpTraceSatFile)
					fclose(m_lpTraceSatFile);

				m_lpTraceSatFile = fopen( szLogFile, "w+" );
				m_ulSequence++;
				m_ulRecCount = 0;
			}

			m_ulRecCount++;

			if(m_ulSequence > 100)
				m_ulSequence = 0;

			return m_lpTraceSatFile;
		}

		static void QueueDBFDetection(const std::string& strLine)
		{
			m_vecDBFQueue.push_back(strLine);
		}

		// Call once per PerformSpCalibration pass. Writes all queued detections to a
		// new per-second file when at least 1000 ms have elapsed since the last flush.
		// Cycles through 3600 files (one per second = one hour) before overwriting.
		static void FlushDBFDetections()
		{
			DWORD dwNow = GetTickCount();

			if(m_dwLastDBFFlush == 0)
			{
				m_dwLastDBFFlush = dwNow;
				return;
			}

			if((dwNow - m_dwLastDBFFlush) < 1000)
				return;

			if(!m_vecDBFQueue.empty())
			{
				SYSTEMTIME st;
				GetLocalTime(&st);

				CreateDirectoryA("D:\\DBFSatTrace\\EMSTraceDBFInput", NULL);

				char szLogFile[256];
				memset(szLogFile, 0, sizeof(szLogFile));
				sprintf(szLogFile, "D:\\DBFSatTrace\\EMSTraceDBFInput\\EMSTraceDBFInput_%02d_%02d.csv",
					st.wMinute, st.wSecond);

				FILE* fp = fopen(szLogFile, "w");
				if(fp)
				{
					for(size_t i = 0; i < m_vecDBFQueue.size(); i++)
						fprintf(fp, "%s", m_vecDBFQueue[i].c_str());
					fclose(fp);
				}
			}

			m_vecDBFQueue.clear();
			m_dwLastDBFFlush = dwNow;
		}

	private:
		CHGTFileLogger()
		{
			m_ulRecCount = 0;
			m_ulSequence = 0;
			m_lpTraceSatFile = NULL;
			m_dwLastDBFFlush = 0;
		}
		~CHGTFileLogger()
		{
			if(m_lpTraceSatFile)
				fclose(m_lpTraceSatFile);
		}

	private:
		static FILE			*m_lpTraceSatFile;
		static ULONG		m_ulRecCount;
		static ULONG		m_ulSequence;
		static DWORD		m_dwLastDBFFlush;
		static std::vector<std::string> m_vecDBFQueue;

};

#endif
