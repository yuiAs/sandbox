#ifndef _ROSAVECHAT_
#define _ROSAVECHAT_

#include <windows.h>

//------------------------------------------------------------------------------
// Macro
//------------------------------------------------------------------------------
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName)  \
    TypeName(const TypeName&);              \
    void operator=(const TypeName&);
#endif

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------
//class ROSaveChat;

//------------------------------------------------------------------------------
// Struct
//------------------------------------------------------------------------------
typedef struct _CHATPACKET
{
    PVOID This;
    ULONG ChatType;
    FILETIME RecvTm;
    UCHAR Data[1];
} CHATPACKET, *PCHATPACKET;

//------------------------------------------------------------------------------
//  Class   :   ROSaveChat
//  Note    :
//------------------------------------------------------------------------------
class ROSaveChat
{
protected:
    HANDLE m_File;
    FILETIME m_NextTm;

    ULONG m_WhisperType;

public:
    ROSaveChat(ULONG WhisperType=2);
    virtual ~ROSaveChat();

private:
    DISALLOW_COPY_AND_ASSIGN(ROSaveChat);

private:
    void Init();
    void Release();
    void Adjust(FILETIME* CurrentTm);

public:
    void Save(PCHATPACKET Item);
};

#endif  // _ROSAVECHAT_
