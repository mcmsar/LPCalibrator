// TSiDebugTrace.h - Execution trace for HGTCalibrateService debugging

#ifndef __TSI_DEBUG_TRACE_H__
#define __TSI_DEBUG_TRACE_H__

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <share.h>

class CTSiDebugTrace
{
public:
    // Verbose tier (idle-loop/per-message chatter): only written when a marker
    // file "<szPath>.debug" exists at startup. Normal operation - no marker
    // file - suppresses these entirely so the trace file only carries the
    // curated "Always" events below.
    static void Open(const char* szPath = "C:\\TSiDebugTrace.txt")
    {
        InitializeCriticalSection(&m_cs);

        strncpy(m_szFilePath, szPath, sizeof(m_szFilePath) - 1);
        m_szFilePath[sizeof(m_szFilePath) - 1] = '\0';
        m_ullLastPruneTick = GetTickCount64();

        char szDebugFlagPath[MAX_PATH + 8];
        _snprintf(szDebugFlagPath, sizeof(szDebugFlagPath) - 1, "%s.debug", m_szFilePath);
        szDebugFlagPath[sizeof(szDebugFlagPath) - 1] = '\0';
        m_bDebugEnabled = (GetFileAttributesA(szDebugFlagPath) != INVALID_FILE_ATTRIBUTES);

        // Verbose logging is high-volume, so it gets a tighter retention window;
        // with only the curated "Always" events, a week stays manageable.
        m_i64RetentionSeconds = m_bDebugEnabled ? (12 * 3600) : (7 * 24 * 3600);

        // Append rather than truncate, so a service restart doesn't discard
        // whatever was logged before it - retention pruning (_MaybePruneLocked)
        // is what keeps the file bounded, not a fresh start on every Open().
        // Use _fsopen with _SH_DENYNO so a second instance can still write the
        // file even if a previous instance crashed while holding it open.
        m_lpFile = _fsopen(szPath, "a", _SH_DENYNO);
        if (m_lpFile)
        {
            _WriteHeaderLocked();
        }
    }

    static void Close()
    {
        EnterCriticalSection(&m_cs);
        if (m_lpFile)
        {
            _WriteLogLocked("=== Debug Trace closed ===", true);
            fclose(m_lpFile);
            m_lpFile = NULL;
        }
        LeaveCriticalSection(&m_cs);
        DeleteCriticalSection(&m_cs);
    }

    // --- Verbose tier: needs the ".debug" marker file to produce output ---

    static void Log(const char* msg)
    {
        if (!m_bDebugEnabled) return;
        EnterCriticalSection(&m_cs);
        _WriteLogLocked(msg, false);
        LeaveCriticalSection(&m_cs);
    }

    static void LogHR(const char* context, long hr)
    {
        if (!m_bDebugEnabled) return;
        _LogHRImpl(context, hr, false);
    }

    static void LogFmt(const char* fmt, ...)
    {
        if (!m_bDebugEnabled) return;
        if (!m_lpFile) return;
        char buf[2048];
        va_list args;
        va_start(args, fmt);
        _vsnprintf(buf, sizeof(buf)-1, fmt, args);
        va_end(args);
        buf[sizeof(buf)-1] = '\0';

        EnterCriticalSection(&m_cs);
        _WriteLogLocked(buf, false);
        LeaveCriticalSection(&m_cs);
    }

    // --- Always tier: relevant-in-normal-operation events (ref beacon hits,
    // bias updates, DBF/candidate failures, exceptions, CSV persistence
    // summaries) - written regardless of the debug flag. ---

    static void LogAlways(const char* msg)
    {
        EnterCriticalSection(&m_cs);
        _WriteLogLocked(msg, true);
        LeaveCriticalSection(&m_cs);
    }

    static void LogHRAlways(const char* context, long hr)
    {
        _LogHRImpl(context, hr, true);
    }

    static void LogFmtAlways(const char* fmt, ...)
    {
        if (!m_lpFile) return;
        char buf[2048];
        va_list args;
        va_start(args, fmt);
        _vsnprintf(buf, sizeof(buf)-1, fmt, args);
        va_end(args);
        buf[sizeof(buf)-1] = '\0';

        EnterCriticalSection(&m_cs);
        _WriteLogLocked(buf, true);
        LeaveCriticalSection(&m_cs);
    }

private:
    static void _LogHRImpl(const char* context, long hr, bool bAlways)
    {
        if (!m_lpFile) return;
        const char* status;
        if (hr == 0)      status = "OK";
        else if (hr == 1) status = "EMS_FALSE/S_FALSE";
        else              status = "*** FAILED ***";

        char buf[512];
        _snprintf(buf, sizeof(buf)-1, "%-62s hr=0x%08X (%ld) [%s]",
            context, (unsigned long)hr, hr, status);
        buf[sizeof(buf)-1] = '\0';

        EnterCriticalSection(&m_cs);
        _WriteLogLocked(buf, bAlways);
        LeaveCriticalSection(&m_cs);
    }

    static void _WriteHeaderLocked()
    {
        if (!m_lpFile) return;

        // In append mode the file may already hold entries from a prior run;
        // a leading blank line keeps this session's banner visually separated
        // from whatever was written before the restart. _ftelli64 reports the
        // current (post-open) size, so it's 0 only for a brand-new file.
        if (_ftelli64(m_lpFile) > 0)
            fprintf(m_lpFile, "\n");

        SYSTEMTIME st;
        GetLocalTime(&st);
        fprintf(m_lpFile,
            "=== HGTCalibrateService Debug Trace ===\n"
            "Opened:   %04d-%02d-%02d %02d:%02d:%02d\n"
            "CSV file: C:\\Program Files (x86)\\EMS Technologies\\LUT Server\\Config\\lscalibdata.csv\n"
            "hr legend: 0x00000000=S_OK/EMS_OK  0x00000001=S_FALSE/EMS_FALSE  negative=error\n"
            "Verbose logging: %s (create/delete \"%s.debug\" and restart to toggle)\n"
            "Retention: entries older than %d hours are pruned automatically\n"
            "========================================\n\n",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
            m_bDebugEnabled ? "ON" : "OFF (only relevant/Always events are logged)",
            m_szFilePath,
            (int)(m_i64RetentionSeconds / 3600));
        fflush(m_lpFile);
    }

    static void _WriteLogLocked(const char* msg, bool bAlways)
    {
        if (!m_bDebugEnabled && !bAlways)
            return;

        if (m_lpFile)
        {
            // Clear any prior stream error so a transient I/O fault doesn't
            // permanently silence the logger.
            if (ferror(m_lpFile))
                clearerr(m_lpFile);

            SYSTEMTIME st;
            GetLocalTime(&st);
            // Full date is included (not just time-of-day) so that lines remain
            // unambiguous across midnight/day boundaries for _MaybePruneLocked.
            fprintf(m_lpFile, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\n",
                st.wYear, st.wMonth, st.wDay,
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, msg);
            fflush(m_lpFile);

            _MaybePruneLocked();
        }
    }

    // Parses the "[YYYY-MM-DD HH:MM:SS.mmm]" prefix written by _WriteLogLocked
    // into a local-time epoch value. Returns false for lines that don't start
    // with a recognizable timestamp (e.g. the banner/header lines).
    static bool _TryParseLineEpoch(const char* szLine, __int64& i64Epoch)
    {
        int nYear = 0, nMonth = 0, nDay = 0, nHour = 0, nMin = 0, nSec = 0, nMs = 0;
        if (sscanf(szLine, "[%d-%d-%d %d:%d:%d.%d]",
                &nYear, &nMonth, &nDay, &nHour, &nMin, &nSec, &nMs) != 7)
            return false;

        struct tm tmLine;
        memset(&tmLine, 0, sizeof(tmLine));
        tmLine.tm_year = nYear - 1900;
        tmLine.tm_mon  = nMonth - 1;
        tmLine.tm_mday = nDay;
        tmLine.tm_hour = nHour;
        tmLine.tm_min  = nMin;
        tmLine.tm_sec  = nSec;
        tmLine.tm_isdst = -1;

        __time64_t t64 = _mktime64(&tmLine);
        if (t64 == (__time64_t)-1)
            return false;

        i64Epoch = (__int64)t64;
        return true;
    }

    // Rewrites the trace file in place, dropping any entries older than
    // m_i64RetentionSeconds. Checked at most once every PRUNE_CHECK_INTERVAL_MS
    // so a long-running service doesn't grow this file without bound, while
    // avoiding rescanning the whole file on every single log call. Must be
    // called with m_cs already held (i.e. only from _WriteLogLocked).
    static void _MaybePruneLocked()
    {
        ULONGLONG ullNow = GetTickCount64();
        if ((ullNow - m_ullLastPruneTick) < PRUNE_CHECK_INTERVAL_MS)
            return;
        m_ullLastPruneTick = ullNow;

        if (!m_lpFile || m_szFilePath[0] == '\0')
            return;

        fflush(m_lpFile);

        FILE* lpRead = fopen(m_szFilePath, "r");
        if (!lpRead)
            return;

        char szTmpPath[MAX_PATH + 16];
        _snprintf(szTmpPath, sizeof(szTmpPath) - 1, "%s.prune", m_szFilePath);
        szTmpPath[sizeof(szTmpPath) - 1] = '\0';

        FILE* lpTmp = fopen(szTmpPath, "w");
        if (!lpTmp)
        {
            fclose(lpRead);
            return;
        }

        __time64_t i64Cutoff = _time64(NULL) - m_i64RetentionSeconds;
        long lDroppedCount = 0;

        char szLine[2048];
        while (fgets(szLine, sizeof(szLine), lpRead))
        {
            __int64 i64LineEpoch = 0;
            bool bTimestamped = _TryParseLineEpoch(szLine, i64LineEpoch);

            // Always keep non-timestamped lines (banner/header) and anything
            // we can't confidently age off, plus anything inside the window.
            if (!bTimestamped || i64LineEpoch >= i64Cutoff)
            {
                fputs(szLine, lpTmp);
            }
            else
            {
                lDroppedCount++;
            }
        }
        fclose(lpRead);

        if (lDroppedCount > 0)
        {
            fprintf(lpTmp, "--- pruned %ld entries older than %d hours ---\n\n",
                lDroppedCount, (int)(m_i64RetentionSeconds / 3600));
        }
        fclose(lpTmp);

        if (lDroppedCount > 0)
        {
            fclose(m_lpFile);
            m_lpFile = NULL;

            DeleteFileA(m_szFilePath);
            MoveFileA(szTmpPath, m_szFilePath);

            m_lpFile = _fsopen(m_szFilePath, "a", _SH_DENYNO);
        }
        else
        {
            // Nothing aged off - discard the scratch copy and keep appending
            // to the file/handle we already have open.
            DeleteFileA(szTmpPath);
        }
    }

public:
    static FILE*            m_lpFile;
    static CRITICAL_SECTION m_cs;

private:
    static char      m_szFilePath[MAX_PATH];
    static ULONGLONG m_ullLastPruneTick;
    static bool      m_bDebugEnabled;
    static __int64   m_i64RetentionSeconds;

    static const DWORD PRUNE_CHECK_INTERVAL_MS = 60 * 60 * 1000;  // check hourly
};

#endif // __TSI_DEBUG_TRACE_H__
