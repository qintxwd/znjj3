#ifndef SERIAL_H
#define SERIAL_H

#include <windows.h>
//#include <windef.h>

//windows serial read and write

typedef void (CALLBACK* ONSERIESREAD)(void * pOwner,BYTE* buf,int bufLen);
#define Q_WS_WIN        1
class Serial
{
public:
    Serial();

    virtual ~Serial();
public:
    bool OpenPort(void* pOwner,int portNo	= 1,int baud = 9600,int parity = NOPARITY, int databits = 8, int stopbits = 0);
    bool OpenPortEx(void* pOwner,int portNo = 1,int baud = 9600,int parity = NOPARITY,int databits = 8,int stopbits = 0);

    void ClosePort();
    bool SetSeriesTimeouts(COMMTIMEOUTS CommTimeOuts);
    bool WriteSyncPort(const BYTE*buf , DWORD bufLen);
    bool GetComOpened();
private:
    static  DWORD WINAPI ReadThreadFunc(LPVOID lparam);

    void CloseReadThread();
private:
    HANDLE	m_hComm;

    HANDLE m_hReadThread;
    DWORD m_dwReadThreadID;
    HANDLE m_hReadCloseEvent;

    OVERLAPPED	m_ovWrite ;
    OVERLAPPED	m_ovRead ;

public:
    bool m_bOpened;
    void * m_pOwner;

    ONSERIESREAD m_OnSeriesRead;
};

#endif // SERIAL_H
