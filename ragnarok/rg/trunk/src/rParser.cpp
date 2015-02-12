#include "rParser.h"
#include <process.h>
#include "intrnl.h"
#include "client.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: ParserWorkThread
// Purpose	:
// Arguments:
// Return	:

unsigned int _stdcall ParseWorkerThread(VOID* Parameter)
{
	return reinterpret_cast<ROParser*>(Parameter)->Worker();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: ROParser
// Purpose	:

ROParser::ROParser() : m_Chat(NULL), m_THCore(0), m_Terminate(false)
{
	Init();
}

// Function	: ~ROParser
// Purpose	:

ROParser::~ROParser()
{
	Release();
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Init
// Purpose	:

void ROParser::Init()
{
	ResetSessionTable();
	LoadConfig();

	if (m_THCore == 0)
	{
		m_Terminate = false;
		m_THCore = _beginthreadex(NULL, 0, ParseWorkerThread, this, 0, NULL);
	}
	if (m_Chat == NULL)
		m_Chat = new ROSavechat;
}

// Function	: Release
// Purpose	:

void ROParser::Release()
{
	if (m_THCore)
	{
		TerminateWorker();

		CloseHandle(reinterpret_cast<HANDLE>(m_THCore));
		m_THCore = 0;
	}
	if (m_Chat)
	{
		m_Chat->Runnable();
		delete m_Chat;
		m_Chat = NULL;
	}
}

// Function	: LoadConfig
// Purpose	:

void ROParser::LoadConfig()
{
	g_Intrnl.Session.NoDivDmg = ConfIsEnable(_CRT_WIDE("PACKET"), _CRT_WIDE("NODIVDMG"));
	g_Intrnl.Session.TrueSight = ConfIsEnable(_CRT_WIDE("PACKET"), _CRT_WIDE("TRUESIGHT"));
}

// Function	: ResetSessionTable
// Purpose	:

void ROParser::ResetSessionTable()
{
	RtlZeroMemory(&g_Intrnl.Session, sizeof(SESSION_TBL));
	g_Intrnl.Session.EnArms = CI_NONE;
	g_Intrnl.Session.ConnFirst = false;
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Worker
// Purpose	:
// Return	:

ULONG ROParser::Worker()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
	SuspendThread(GetCurrentThread());

	while (m_Terminate==false)
	{
		ULONG Tick = GetTickCount();

		CheckItemEnchantArms(Tick);
		m_Chat->Runnable();

		Sleep(50);
	}

	return 0;
}

void ROParser::SuspendWorker()
{
	SuspendThread(reinterpret_cast<HANDLE>(m_THCore));
}

void ROParser::ResumeWorker()
{
	while (ResumeThread(reinterpret_cast<HANDLE>(m_THCore))>1);
}

void ROParser::TerminateWorker()
{
	m_Terminate = true;
	ResumeWorker();
	WaitForSingleObject(reinterpret_cast<HANDLE>(m_THCore), 1*1000);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: Recv
// Purpose	:
// Arguments:
// Return	:

int ROParser::Recv(PUCHAR Buffer, int Length)
{
	DbgPrintW(DBGINFL2, _CRT_WIDE("INF ROParser::Recv Length=%d\n"), Length);
	DbgHexPrintW(Buffer, Length);


	switch (*reinterpret_cast<USHORT*>(Buffer))
	{
		case PACKET_AC_AUTH_ACK:
			ClAuthAck(Buffer);
			break;

		case PACKET_CC_SELECT_ACK:
			ConnFirstProc(Buffer);
			break;

		case PACKET_CC_ENTER_ACK:
			ClCCEnterAck(Buffer);
			break;

		case PACKET_ZC_ENTER_ACK:
			ClZCEnterAck(Buffer);
			break;

		case PACKET_ZC_ERASE_ACTOR:
			ClEraseActor(Buffer);
			break;

		case PACKET_ZC_CHAT:
		case PACKET_ZC_CHAT_THIS:
		case PACKET_ZC_WHISPER:
		case PACKET_ZC_WHISPER_ACK:
		case PACKET_ZC_BROADCAST:
		case PACKET_ZC_PARTYCHAT:
		case PACKET_ZC_MVP:
			m_Chat->Push(Buffer, Length);
			break;

		case PACKET_ZC_OPTION1:
			ClOption(Buffer);
			break;

		case PACKET_ZC_SKILL_TMP:
			ClSkillTmp(Buffer);
			break;

		case PACKET_ZC_CONDITION:
			ClCondition(Buffer);
			break;

		case PACKET_ZC_GUILDCHAT:
			m_Chat->Push(Buffer, Length);
			break;

		case PACKET_ZC_SKILL_ACK5:
			ClSkillAck(Buffer);
			break;

		case PACKET_ZC_BROADCAST2:
			m_Chat->Push(Buffer, Length);
			break;

		case PACKET_ZC_OPTION2:
			ClOption(Buffer);
			break;

		case PACKET_AC_SEED_ACK:
			ConnFirstProc(Buffer);
			break;

		case PACKET_NC_NPROTECT_0259:
			ConnFirstProc(Buffer);
			break;

		default:
			break;
	}


	return Length;
}

// Function	: Send
// Purpose	:
// Arguments:
// Return	: int	0		send
//					>0		block

int ROParser::Send(PUCHAR Buffer, int Length)
{
	DbgPrintW(DBGINFL2, _CRT_WIDE("INF ROParser::Send Length=%d\n"), Length);
	DbgHexPrintW(Buffer, Length);

	return 0;
}
