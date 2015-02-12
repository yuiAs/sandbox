#include "ROSaveChat.h"
#include "Module.h"
#include "Client.h"
#include "WinAdvApi.h"
#include <wchar.h>
#include <stdio.h>

// C4996: This function or variable may be unsafe.
#pragma warning(disable: 4996)

//------------------------------------------------------------------------------
// Defined
//------------------------------------------------------------------------------
#ifndef SAVECHAT_COLMAX
#define SAVECHAT_COLMAX 128
#endif

////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::ROSaveChat
//  Note    :   
//------------------------------------------------------------------------------
ROSaveChat::ROSaveChat(ULONG WhisperType)
    : m_File(NULL), m_WhisperType(WhisperType)
{
    Init();
}

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::~ROSaveChat
//  Note    :   
//------------------------------------------------------------------------------
ROSaveChat::~ROSaveChat()
{
    Release();
}

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::Init
//  Note    :   
//------------------------------------------------------------------------------
void ROSaveChat::Init()
{
    RtlZeroMemory(&m_NextTm, sizeof(FILETIME));

    CreateDirectoryW(L"Chat", NULL);
    Adjust(NULL);
}

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::Release
//  Note    :   
//------------------------------------------------------------------------------
void ROSaveChat::Release()
{
    CloseHandle(m_File);
}

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::Adjust
//  Note    :   
//------------------------------------------------------------------------------
void ROSaveChat::Adjust(FILETIME* CurrentTm)
{
    if (m_File)
    {
        if (CurrentTm)
        {
            if (CompareFileTime(&m_NextTm, CurrentTm) > 0)
                return;
        }

        CloseHandle(m_File);
    }

    SYSTEMTIME LocalTm;
    GetLocalTime(&LocalTm);

    WCHAR FileName[MAX_PATH];
    _snwprintf(FileName, MAX_PATH-1, L"%s/%04d-%02d-%02d.txt", L"Chat", LocalTm.wYear, LocalTm.wMonth, LocalTm.wDay);

    m_File = CreateFileW(FileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_File == INVALID_HANDLE_VALUE)
    {
        m_File = NULL;
        RtlZeroMemory(&m_NextTm, sizeof(FILETIME));
    }
    else
    {
        // EOF‚ÖSeek‚³‚¹‚Ä‚¨‚­
        // 2G‚Í’´‚¦‚é‚±‚Æ‚Í‚È‚¢‚Æ’f’è(íŽ¯“I‚Él‚¦‚Ä‚ ‚è‚¦‚È‚¢)
        SetFilePointer(m_File, 0, NULL, FILE_END);

        LocalTm.wHour = 0;
        LocalTm.wMinute = 0;
        LocalTm.wSecond = 0;
        LocalTm.wMilliseconds = 0;
        SystemTimeToFileTimeAdd(&LocalTm, SecondToNanoSecond(24*60*60), &m_NextTm);
    }
}

//------------------------------------------------------------------------------
//  Function:   ROSaveChat::Save
//  Note    :   
//------------------------------------------------------------------------------
void ROSaveChat::Save(PCHATPACKET Item)
{
    Adjust(&Item->RecvTm);

    PUCHAR Data = Item->Data;
    SYSTEMTIME RecvTm;
    FileTimeToSystemTime(&Item->RecvTm, &RecvTm);

    int Result = -1;

    char Buffer[SAVECHAT_COLMAX];
    RtlZeroMemory(Buffer, sizeof(Buffer));

    switch (*reinterpret_cast<USHORT*>(Data))
    {
        case PACKET_ZC_CHAT:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATNOR, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+8, MSG_EOL);
            break;

        case PACKET_ZC_CHAT_OWN:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATNOR, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+4, MSG_EOL);
            break;

        case PACKET_CZ_WHISPER:
            break;

        case PACKET_ZC_WHISPER:
            {
                // 2: PACKET_ZC_WHISPER <LENGTH>.w <NAME>.24b <OID>.d <BODY>.?
                // 1: PACKET_ZC_WHISPER <LENGTH>.w <NAME>.24b <BODY>.?
                PSTR Name = reinterpret_cast<PSTR>(Data+4);
                PSTR Body = NULL;

                switch (m_WhisperType)
                {
                    case 1:
                        Body = reinterpret_cast<PSTR>(Data+28);
                        break;
                    case 2:
                        Body = reinterpret_cast<PSTR>(Data+32);
                        break;
                }

                if (Name && Body)
                    Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATWIS_R, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Name, Body, MSG_EOL);
            }
            break;

        case PACKET_ZC_ACK_WHISPER:
            {
                PSTR String = NULL;

                switch (*reinterpret_cast<UCHAR*>(Data+2))
                {
                    case 0x01:
                        String = ClGetMsgStr(MSG_WHISPER_FAILURE);
                        break;
                    case 0x02:
                        String = ClGetMsgStr(MSG_WHISPER_REFUSE);
                        break;
                }

                if (String)
                    Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATWIS_ACK, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, String, MSG_EOL);
            }
            break;

        case PACKET_ZC_BROADCAST:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATGBC, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+4, MSG_EOL);
            break;

        case PACKET_ZC_CHAT_PARTY:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATPRT, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+8, MSG_EOL);
            break;

        case PACKET_ZC_MPV_CHARACTER:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATMVP, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, *reinterpret_cast<ULONG*>(Data+2), MSG_EOL);
            break;

        case PACKET_ZC_CHAT_GUILD:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATGLD, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+4, MSG_EOL);
            break;

        case PACKET_ZC_MULTICAST:
            Result = _snprintf(Buffer, SAVECHAT_COLMAX-1, MSG_CHATLBC, RecvTm.wHour, RecvTm.wMinute, RecvTm.wSecond, Data+16, MSG_EOL);
            break;

        default:
            __assume(0);
    }

    if (Result > 0)
    {
        ULONG Dummy = 0;
        WriteFile(m_File, Buffer, Result, &Dummy, NULL);
    }
}
