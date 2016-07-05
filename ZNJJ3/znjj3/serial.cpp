#include "serial.h"
#include <stdlib.h>
#include <stdio.h>
#include <QString>
#include <QDebug>
#include <WinBase.h>

Serial::Serial()
{
    m_ovRead.Offset = 0;
    m_ovRead.OffsetHigh = 0;
    m_ovWrite.Offset = 0;
    m_ovWrite.OffsetHigh = 0;

    // create events
    m_ovRead.hEvent = NULL;
    m_ovWrite.hEvent = NULL;

    m_hComm = INVALID_HANDLE_VALUE;
    m_OnSeriesRead = NULL;
    m_bOpened = 0;
}

Serial::~Serial()
{

}

bool Serial::OpenPort(void * pOwner,int portNo,int baud,int parity	,int databits,int stopbits)
{
    Q_UNUSED(parity);
#ifdef Q_WS_WIN
    DCB commParam;
    QString szPort;

    m_pOwner = pOwner;

    if (m_hComm != INVALID_HANDLE_VALUE)
    {
        return TRUE;
    }

    if ((portNo <=0) || (portNo >= 20))
    {
        return false;
    }

    if (m_ovRead.hEvent != NULL)
    {
        ResetEvent(m_ovRead.hEvent);
    }
    else
    {
        m_ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE,  TEXT("openport_read_event"));//).toStdWString());
    }

    // create event for overlapped write
    if (m_ovWrite.hEvent != NULL)
    {
        ResetEvent(m_ovWrite.hEvent);
    }
    else
    {
        m_ovWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("openport_write_event"));
    }

    szPort = QString("COM%1").arg(portNo);

    LPCWSTR lstr =reinterpret_cast<LPCWSTR>( szPort.utf16());
    m_hComm = CreateFile(
        lstr,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
        );

    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        return FALSE;

    }


    if (!GetCommState(m_hComm,&commParam))
    {
        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }


    commParam.BaudRate = baud;
    commParam.fBinary = TRUE;
    commParam.fParity = TRUE;
    commParam.ByteSize = databits;
    commParam.Parity = NOPARITY;
    commParam.StopBits = stopbits;

    commParam.fOutxCtsFlow = FALSE;				// No CTS output flow control
    commParam.fOutxDsrFlow = FALSE;				// No DSR output flow control
    commParam.fDtrControl = DTR_CONTROL_DISABLE;
    // DTR flow control type
    commParam.fDsrSensitivity = FALSE;			// DSR sensitivity
    commParam.fTXContinueOnXoff = TRUE;			// XOFF continues Tx
    commParam.fOutX = FALSE;					// No XON/XOFF out flow control
    commParam.fInX = FALSE;						// No XON/XOFF in flow control
    commParam.fErrorChar = FALSE;				// Disable error replacement
    commParam.fNull = FALSE;					// Disable null stripping
    commParam.fRtsControl = RTS_CONTROL_DISABLE;
    // RTS flow control
    commParam.fAbortOnError = FALSE;

    if (!SetCommState(m_hComm, &commParam))
    {
        qDebug()<<("SetCommState error");
        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    COMMTIMEOUTS tempCommTimeOuts;
    GetCommTimeouts (m_hComm, &tempCommTimeOuts);

    tempCommTimeOuts.ReadIntervalTimeout = 100;
    tempCommTimeOuts.ReadTotalTimeoutMultiplier = 100;
    tempCommTimeOuts.ReadTotalTimeoutConstant = 1000;

    tempCommTimeOuts.WriteTotalTimeoutMultiplier = 100;
    tempCommTimeOuts.WriteTotalTimeoutConstant = 1000;

    if(!SetCommTimeouts( m_hComm, &tempCommTimeOuts ))
    {
        qDebug()<<QStringLiteral("SetCommTimeouts 返回错误");

        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    SetCommMask (m_hComm, EV_RXCHAR);

    SetupComm(m_hComm,5120,512);

    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    m_hReadCloseEvent = CreateEvent(NULL,FALSE,FALSE,TEXT("Com_ReadCloseEvent"));

    m_hReadThread = CreateThread(NULL,0,ReadThreadFunc,this,0,&m_dwReadThreadID);

    SetThreadPriority(m_hReadThread,THREAD_PRIORITY_TIME_CRITICAL);

    //qDebug()<<QStringLiteral("串口打开成功\n");
    m_bOpened = TRUE;
    return TRUE;
#endif
}

bool Serial::OpenPortEx(void * pOwner,int portNo,int baud,int parity,int databits,int stopbits)
{
    Q_UNUSED(parity);
    #ifdef Q_WS_WIN
    DCB commParam;
    QString szPort;

    m_pOwner = pOwner;

    if (m_hComm != INVALID_HANDLE_VALUE)
    {
        return TRUE;
    }

    if ((portNo <=0) || (portNo >= 20))
    {
        return false;
    }

    if (m_ovRead.hEvent != NULL)
    {
        ResetEvent(m_ovRead.hEvent);
    }
    else
    {
        m_ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("openport_ex_read_event"));
    }

    if (m_ovWrite.hEvent != NULL)
    {
        ResetEvent(m_ovWrite.hEvent);
    }
    else
    {
        m_ovWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, TEXT("openport_ex_write_event"));
    }
    szPort = QString("COM%1").arg(portNo);

    LPCWSTR lstr =reinterpret_cast<LPCWSTR>( szPort.utf16());
    m_hComm = CreateFile(
        lstr,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        NULL
        );

    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        return FALSE;

    }

    if (!GetCommState(m_hComm,&commParam))
    {
        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }


    commParam.BaudRate = baud;
    commParam.fBinary = TRUE;
    commParam.fParity = TRUE;
    commParam.ByteSize = databits;
    commParam.Parity = NOPARITY;
    commParam.StopBits = stopbits;

    commParam.fOutxCtsFlow = FALSE;
    commParam.fOutxDsrFlow = FALSE;
    commParam.fDtrControl = DTR_CONTROL_DISABLE;
    // DTR flow control type
    commParam.fDsrSensitivity = FALSE;			// DSR sensitivity
    commParam.fTXContinueOnXoff = TRUE;			// XOFF continues Tx
    commParam.fOutX = FALSE;					// No XON/XOFF out flow control
    commParam.fInX = FALSE;						// No XON/XOFF in flow control
    commParam.fErrorChar = FALSE;				// Disable error replacement
    commParam.fNull = FALSE;					// Disable null stripping
    commParam.fRtsControl = RTS_CONTROL_DISABLE;
    // RTS flow control
    commParam.fAbortOnError = FALSE;

    if (!SetCommState(m_hComm, &commParam))
    {
        qDebug()<<("SetCommState error");
        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    COMMTIMEOUTS CommTimeOuts;
    GetCommTimeouts (m_hComm, &CommTimeOuts);


    CommTimeOuts.ReadIntervalTimeout = 100;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 100;
    CommTimeOuts.ReadTotalTimeoutConstant = 1000;

    CommTimeOuts.WriteTotalTimeoutMultiplier = 100;
    CommTimeOuts.WriteTotalTimeoutConstant = 1000;

    if(!SetCommTimeouts( m_hComm, &CommTimeOuts ))
    {
        qDebug()<<QStringLiteral("SetCommTimeouts 返回错误");
        CloseHandle (m_hComm);

        m_hComm = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    SetCommMask (m_hComm, EV_RXCHAR);

    SetupComm(m_hComm,5120,512);

    PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    m_hReadCloseEvent = CreateEvent(NULL,FALSE,FALSE,TEXT("Com_ReadCloseEvent"));//L"Com_ReadCloseEvent");

    m_hReadThread = CreateThread(NULL,0,ReadThreadFunc,this,0,&m_dwReadThreadID);

    SetThreadPriority(m_hReadThread,THREAD_PRIORITY_TIME_CRITICAL);

    //qDebug()<<QStringLiteral("串口打开成功\n");
    m_bOpened = TRUE;
    return TRUE;
#endif
}

void Serial::ClosePort()
{
    #ifdef Q_WS_WIN

    if (m_hComm == INVALID_HANDLE_VALUE)
    {
        return ;
    }


    CloseReadThread();

    CloseHandle (m_hComm);

    CloseHandle(m_hReadCloseEvent);

    CloseHandle(m_ovRead.hEvent);
    CloseHandle(m_ovWrite.hEvent);


    m_hComm = INVALID_HANDLE_VALUE;
    m_bOpened = FALSE;

    m_ovRead.Offset = 0;
    m_ovRead.OffsetHigh = 0;
    m_ovWrite.Offset = 0;
    m_ovWrite.OffsetHigh = 0;

    // create events
    m_ovRead.hEvent = NULL;
    m_ovWrite.hEvent = NULL;
#endif

}

bool Serial::GetComOpened()
{
    return m_bOpened;
}

bool Serial::SetSeriesTimeouts(COMMTIMEOUTS CommTimeOuts)
{
#ifdef Q_WS_WIN
    return SetCommTimeouts(m_hComm,&CommTimeOuts);
#endif
}

DWORD Serial::ReadThreadFunc(LPVOID lparam)
{
#ifdef Q_WS_WIN
    Serial *port = (Serial*)lparam;

    DWORD actualReadLen=0;

    DWORD dwError;
    DWORD	CommEvent = 0;
    COMSTAT comstat;
    BOOL	bResult = TRUE;
    BYTE readBuf[1024];

    PurgeComm(port->m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    SetCommMask (port->m_hComm, EV_RXCHAR );
    while (TRUE)
    {
        if (WaitForSingleObject(port->m_hReadCloseEvent,10) == WAIT_OBJECT_0)
        {
            break;
        }

        bResult= WaitCommEvent(port->m_hComm, &CommEvent, &port->m_ovRead);

        ClearCommError(port->m_hComm,&dwError,&comstat);
        actualReadLen=(1024<(DWORD)comstat.cbInQue?1024:(DWORD)comstat.cbInQue);
        if (actualReadLen <=0)
        {
            continue;
        }

        bResult=ReadFile(port->m_hComm,readBuf,
                             actualReadLen,&actualReadLen,&(port->m_ovRead));

        if(!bResult)
        {
            if(GetLastError()==ERROR_IO_PENDING)
            {
                WaitForSingleObject(port->m_ovRead.hEvent,1000);
                PurgeComm(port->m_hComm, PURGE_TXABORT|
                    PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

                if (actualReadLen>0)
                {
                    if (port->m_OnSeriesRead)
                    {
                        port->m_OnSeriesRead(port->m_pOwner,readBuf,actualReadLen);
                    }
                }

            }
        }
        else
        {
                if (actualReadLen>0)
                {
                    if (port->m_OnSeriesRead)
                    {
                        port->m_OnSeriesRead(port->m_pOwner,readBuf,actualReadLen);
                    }
                }
        }
    }

    //qDebug()<<QStringLiteral("串口读线程退出\n");
    return 0;
#endif
}


bool Serial::WriteSyncPort(const BYTE*buf , DWORD bufLen)
{
#ifdef Q_WS_WIN
    DWORD dwBytesWritten=0;

    BOOL bWriteStat;

    if (!m_bOpened)
    {
        return false;
    }

    bWriteStat=WriteFile(m_hComm,buf,bufLen,
        &dwBytesWritten,&m_ovWrite);
    if(!bWriteStat)
    {
        if(GetLastError()==ERROR_IO_PENDING)
        {
            WaitForSingleObject(m_ovWrite.hEvent,3000);
            if (dwBytesWritten == bufLen)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#endif
}


void Serial::CloseReadThread()
{
#ifdef Q_WS_WIN
    SetEvent(m_hReadCloseEvent);

    if (WaitForSingleObject(m_hReadThread,4000) == WAIT_TIMEOUT)
    {
        TerminateThread(m_hReadThread,0);
    }
    m_hReadThread = NULL;
#endif
}
