#ifndef _ENUMCLIENT_
#define _ENUMCLIENT_

//------------------------------------------------------------------------------
//  MSG_xxx
//  VarType : USHORT
//------------------------------------------------------------------------------
enum MsgStringID
{
    MSG_NOMSG                   = 0,
    MSG_WHISPER_FAILURE         = 148,
    MSG_WHISPER_REFUSE          = 150,
    MSG_PROPERTY_NEUTRAL        = 414,
    MSG_PROPERTY_WATER,
    MSG_PROPERTY_GROUND,
    MSG_PROPERTY_FIRE,
    MSG_PROPERTY_WIND,
    MSG_PROPERTY_POISON,
    MSG_PROPERTY_HOLY,
    MSG_PROPERTY_DARK,
    MSG_PROPERTY_TELEKINESIS,
    MSG_PROPERTY_UNDEAD,
    MSG_RESTORE_PROPERTY_A      = 471,
    MSG_RESTORE_PROPERTY_B      = 473,
};

//------------------------------------------------------------------------------
//  CMDOFS_xxx
//  VarType : UCHAR
//------------------------------------------------------------------------------
enum CommandOffset
{
    CMDOFS_AURA         = 0x6A,
    CMDOFS_BATTLEMODE   = 0x86,
    CMDOFS_WINDOW       = 0x8C,
    CMDOFS_SHOPPING     = 0xA6,

    CMDOFS_WI           = CMDOFS_WINDOW,
    CMDOFS_SH           = CMDOFS_SHOPPING,
    CMDOFS_BM           = CMDOFS_BATTLEMODE,
};

//------------------------------------------------------------------------------
//  COLOR_xxx
//  VarType : ULONG
//------------------------------------------------------------------------------
enum ColorDefine
{
    // Base
    COLOR_CYAN          = 0x00FFFF00,
    COLOR_RED           = 0x000000FF,
    COLOR_YELLOW        = 0x0000FFFF,
    COLOR_GREEN         = 0x0000FF00,
    COLOR_WHITE         = 0x00FFFFFF,
    COLOR_BLACK         = 0x00000000,

    // Client
    COLOR_NOTICE1       = COLOR_CYAN,
    COLOR_NOTICE2       = COLOR_RED,
    COLOR_NOTICE3       = 0x00CEFF00,       // Equip
    COLOR_NOTICE4       = COLOR_YELLOW,     // Command1
    COLOR_NOTICE5       = 0x0063FFFF,       // Command2
    COLOR_NOTICE7       = 0x0084FFFF,       // GuildNotice
    COLOR_NOTICE6       = 0x00FFE7E7,
    COLOR_CHAT          = COLOR_WHITE,
    COLOR_CHAT_OWN      = COLOR_GREEN,
    COLOR_PARTY         = 0x00CECEFF,
    COLOR_PARTY_OWN     = 0x0000CEFF,
    COLOR_GUILD         = 0x00B5FFB5,
    COLOR_BROADCAST     = COLOR_YELLOW,
    COLOR_WHISPER       = COLOR_YELLOW,
    COLOR_TALKIE        = 0x00FF8484,
    COLOR_NAME_MOB      = 0x00C6C6FF,
    COLOR_NAME_NPC      = 0x00F7BF94,
    COLOR_NAME_ITEM     = 0x0094EFFF,
};

#endif  // _ENUMCLIENT_
