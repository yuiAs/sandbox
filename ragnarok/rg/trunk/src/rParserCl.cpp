#include "rParser.h"
#include "intrnl.h"
#include "client.h"
#include "debug.h"

////////////////////////////////////////////////////////////////////////////////

// AC_AUTH_ACK

void ROParser::ClAuthAck(PUCHAR Buffer)
{
	SuspendWorker();

	ResetSessionTable();
	g_Intrnl.Session.AID = *reinterpret_cast<ULONG*>(Buffer+8);

	DbgPrintW(DBGINF, _CRT_WIDE("INF AC_AUTH_ACK AID=%08X\n"), g_Intrnl.Session.AID);
}

////////////////////////////////////////////////////////////////////////////////

// CC_ENTER_ACK

void ROParser::ClCCEnterAck(PUCHAR Buffer)
{
	SuspendWorker();

	if (g_Intrnl.ClCmd[CLCMD_SH])
		SetClCmdValue(g_Intrnl.Addr[IA_CMDSH], 1);
	if (g_Intrnl.ClCmd[CLCMD_WI])
		SetClCmdValue(g_Intrnl.Addr[IA_CMDWI], 1);

	g_Intrnl.Session.EnArms = CI_NONE;
	g_Intrnl.Session.EnArmsCount = 0;
}

////////////////////////////////////////////////////////////////////////////////

// ZC_ENTER_ACK

void ROParser::ClZCEnterAck(PUCHAR Buffer)
{
	// CHAR->ZONE: ZC_ENTER_ACK
	// ZONE->ZONE: ZC_MAPCHANGE_IP->ZC_ENTER_ACK

	LoadConfig();

	if (g_Intrnl.Session.TrueSight)
		PushCondition(CI_TRUESIGHT, 1);
	if (g_Intrnl.Session.EnArms != CI_NONE)
		EnchantArms(g_Intrnl.Session.EnArms, 1, true);

	ResumeWorker();
}

////////////////////////////////////////////////////////////////////////////////

// ZC_ERASE_ACTOR

void ROParser::ClEraseActor(PUCHAR Buffer)
{
	if (IsOwnAID(*reinterpret_cast<ULONG*>(Buffer+2)) == false)
		return;
	if (*reinterpret_cast<UCHAR*>(Buffer+6) != 0x01)
		return;

	if (g_Intrnl.Session.EnArms != CI_NONE)
		DeleteItemEnchantArms();
}

////////////////////////////////////////////////////////////////////////////////

// ZC_OPTION1/2

void ROParser::ClOption(PUCHAR Buffer)
{
	if (IsOwnAID(*reinterpret_cast<ULONG*>(Buffer+2)))
		OwnOption(Buffer);
}

void ROParser::OwnOption(PUCHAR Buffer)
{
	USHORT Param1 = *reinterpret_cast<USHORT*>(Buffer+6);
	USHORT Param2 = *reinterpret_cast<USHORT*>(Buffer+8);
	USHORT Param3 = *reinterpret_cast<USHORT*>(Buffer+10);
	DbgPrintW(DBGINF, _CRT_WIDE("INF ZC_OPTION TGT=OWN P1=%04X P2=%04X P3=%04X\n"), Param1, Param2, Param3);

	switch (Param1)
	{
		case OPT_STUN:
			NoticeAbnormal("CONDITION_TO_STUN");
			break;
		case OPT_SLEEP:
			NoticeAbnormal("CONDITION_TO_CURSE_SLEEP");
			break;
		default:
			break;
	}

	if (Param2 & OPT_POISON)
		NoticeAbnormal("CONDITION_TO_POISON");
	if (Param2 & OPT_BLIND)
	{
		*reinterpret_cast<USHORT*>(Buffer+8) ^= OPT_BLIND;
		NoticeAbnormal("CONDITION_TO_BLIND");
	}
}

////////////////////////////////////////////////////////////////////////////////

// ZC_CONDITION

void ROParser::ClCondition(PUCHAR Buffer)
{
	if (IsOwnAID(*reinterpret_cast<ULONG*>(Buffer+4)))
		OwnCondition(Buffer);
}

void ROParser::OwnCondition(PUCHAR Buffer)
{
	USHORT Type = *reinterpret_cast<USHORT*>(Buffer+2);
	UCHAR Flag = *reinterpret_cast<UCHAR*>(Buffer+8);
	DbgPrintW(DBGINF, _CRT_WIDE("INF ZC_CONDITION TGT=OWN TYPE=%04X FLAG=%02X\n"), Type, Flag);

	switch (Type)
	{
		case CI_BREAKWEAPON:
			if (g_Intrnl.Session.EnArms != CI_NONE)
				DeleteItemEnchantArms();
			break;

		case CI_HALLUCINATION:
			if (Flag)
			{
				*reinterpret_cast<UCHAR*>(Buffer+8) &= 0;
				NoticeAbnormal("CONDITION_TO_HALLUCINATION");
			}
			break;

		case CI_ELEMENT_FIRE:
		case CI_ELEMENT_WATER:
		case CI_ELEMENT_WIND:
		case CI_ELEMENT_GROUND:
		case CI_ELEMENT_SHADOW:
		case CI_ELEMENT_GHOST:
			EnchantArms(Type, Flag, false);
			break;

		case CI_EXPLOSIONSPIRITS:
			NoticeState("爆裂波動", Flag);
			break;
		case CI_BERSERK:
			NoticeState("バーサーク", Flag);
			break;
		case CI_ASSUMPTIO:
			NoticeState("アスムプティオ", Flag);
			break;
		case CI_MIRACLE:
			NoticeState("太陽と月と星の奇跡", Flag);
			break;
		case CI_CLOSECONFINE_OWN:
		case CI_CLOSECONFINE_TGT:
			NoticeState("クローズコンファイン", Flag);
			break;

		default:
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

// ZC_SKILL_TMP

void ROParser::ClSkillTmp(PUCHAR Buffer)
{
	USHORT SkillID = *reinterpret_cast<USHORT*>(Buffer+2);
	USHORT SkillLv = *reinterpret_cast<USHORT*>(Buffer+8);
	DbgPrintW(DBGINF, _CRT_WIDE("INF ZC_SKILL_TMP ID=%04X LV=%04X\n"), SkillID, SkillLv);

	if (SkillID != ITEM_ENCHANTARMS)
		return;

	switch (SkillLv)
	{
		case 0x0002:
			ItemEnchantArms(CI_ELEMENT_WATER);
			break;
		case 0x0003:
			ItemEnchantArms(CI_ELEMENT_GROUND);
			break;
		case 0x0004:
			ItemEnchantArms(CI_ELEMENT_FIRE);
			break;
		case 0x0005:
			ItemEnchantArms(CI_ELEMENT_WIND);
			break;
		case 0x0008:
			ItemEnchantArms(CI_ELEMENT_SHADOW);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

// ZC_SKILL_ACK5

void ROParser::ClSkillAck(PUCHAR Buffer)
{
	if (g_Intrnl.Session.NoDivDmg == false)
		return;
	if (IsOwnAID(*reinterpret_cast<ULONG*>(Buffer+8)) == false)
		return;

	DbgPrintW(DBGINF, _CRT_WIDE("INF ZC_SKILL_ACK5 ID=%04X LV=%04X Type=%02X Val=%d/%d\n"), *reinterpret_cast<USHORT*>(Buffer+2),
																							*reinterpret_cast<USHORT*>(Buffer+28),
																							*reinterpret_cast<ULONG*>(Buffer+24),
																							*reinterpret_cast<USHORT*>(Buffer+30),
																							*reinterpret_cast<UCHAR*>(Buffer+32));

	if (*reinterpret_cast<UCHAR*>(Buffer+32) == SKDMG_DIVIDE)
	{
		*reinterpret_cast<USHORT*>(Buffer+30) = 1;	// 単発は1固定
		*reinterpret_cast<UCHAR*>(Buffer+32) = SKDMG_ONE;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: IsOwnAID
// Purpose	:
// Arguments:
// Return	:

bool ROParser::IsOwnAID(ULONG SID)
{
	return (SID==g_Intrnl.Session.AID);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: PushCondition
// Purpose	:
// Arguments:

void ROParser::PushCondition(USHORT Type, UCHAR Flag)
{
//	SetConditionIcon(Type, Flag);

	UCHAR Buffer[9];
	*reinterpret_cast<USHORT*>(Buffer) = PACKET_ZC_CONDITION;
	*reinterpret_cast<USHORT*>(Buffer+2) = Type;
	*reinterpret_cast<ULONG*>(Buffer+4) = g_Intrnl.Session.AID;
	*reinterpret_cast<UCHAR*>(Buffer+8) = Flag;

	DbgPrintW(DBGINF, _CRT_WIDE("CHK PushCondition Vars OP=%04X Type=%04X AID=%08X FLAG=%02d\n"), PACKET_ZC_CONDITION, Type, g_Intrnl.Session.AID, Flag);
	ExecuteParser(PACKET_ZC_CONDITION, Buffer);
}

// Function	: PopCondition
// Purpose	:
// Arguments:

void ROParser::PopCondition(USHORT Type)
{
	PushCondition(Type, 0);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: NoticeState
// Purpose	:
// Arguments:

void ROParser::NoticeState(PSTR String, UCHAR Flag)
{
	if (Flag & 0x01)
		CWPushFormattedText(COL_NOTICE1, "%s状態になりました。", String);
	else
		CWPushFormattedText(COL_NOTICE2, "%s状態が解除されました。", String);
}

// Function	: PushNoticeAbnormal
// Purpose	:
// Arguments:

void ROParser::NoticeAbnormal(PSTR String)
{
	CWPushText(COL_NOTICE2, String);
}

////////////////////////////////////////////////////////////////////////////////

// Function	: EnchantArms
// Purpose	:
// Arguments:

void ROParser::EnchantArms(USHORT Type, UCHAR Flag, bool EnablePush)
{
	if (Flag & 0x01)
	{
		PSTR String = NULL;

		switch (Type)
		{
			case CI_ELEMENT_FIRE:
				String = "火";
				break;
			case CI_ELEMENT_WATER:
				String = "水";
				break;
			case CI_ELEMENT_WIND:
				String = "風";
				break;
			case CI_ELEMENT_GROUND:
				String = "地";
				break;
			case CI_ELEMENT_SHADOW:
				String = "闇";
				break;
			case CI_ELEMENT_GHOST:
				String = "念";
				break;
//			default:
//				String = "?";
//				break;
			default:
				__assume(0);
		}

		CWPushFormattedText(COL_NOTICE1, " 武器に%s属性が付与されました。", String);
	}
	else
	{
		PSTR String = NULL;
		GetMsgStringTablePtr(MSG_RESTORE_WEAPON_ELEMENT, reinterpret_cast<PVOID*>(&String));

		if (String)
			CWPushText(COL_NOTICE2, String);
	}

	if (EnablePush)
		PushCondition(Type, Flag);
}

// Function	: ItemEnchantArms
// Purpose	:
// Arguments:

void ROParser::ItemEnchantArms(USHORT Type)
{
	if (g_Intrnl.Session.EnArms != CI_NONE)
	{
		EnchantArms(g_Intrnl.Session.EnArms, 0, true);
		g_Intrnl.Session.EnArmsCount = 0;
	}
	if (Type != CI_NONE)
	{
		EnchantArms(Type, 1, true);
		g_Intrnl.Session.EnArmsCount = GetTickCount() + VAL_ITEMENARMS;
	}

	g_Intrnl.Session.EnArms = Type;
}

// Function	: DeleteItemEnchantArms
// Purpose	:

void ROParser::DeleteItemEnchantArms()
{
	ItemEnchantArms(CI_NONE);
}

// Function	: DeleteItemEnchantArms
// Purpose	:
// Arguments:

void ROParser::CheckItemEnchantArms(ULONG Tick)
{
	if (g_Intrnl.Session.EnArmsCount < Tick)
		DeleteItemEnchantArms();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Function	: ConnFirstProc
// Purpose	:
// Arguments:

void ROParser::ConnFirstProc(PUCHAR Buffer)
{
	switch (*reinterpret_cast<USHORT*>(Buffer))
	{

		case PACKET_CC_SELECT_ACK:
			g_Intrnl.Session.ConnFirst = true;
			break;
		case PACKET_AC_SEED_ACK:
			ResetSessionTable();
			break;
		case PACKET_NC_NPROTECT_0259:
			g_Intrnl.Session.ConnFirst = true;
			break;
		default:
			__assume(0);
	}
}
