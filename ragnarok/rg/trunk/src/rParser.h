#ifndef RPARSER_H
#define RPARSER_H

#include "win32.h"
#include "rSavechat.h"

////////////////////////////////////////////////////////////////////////////////


class ROParser
{
private:

	ROParser(const ROParser& _class);
	ROParser& operator=(const ROParser& _class);

private:

	ROSavechat* m_Chat;

	uintptr_t m_THCore;
	bool m_Terminate;

public:

	ROParser();
	virtual ~ROParser();

private:

	void Init();
	void Release();

	void LoadConfig();
	void ResetSessionTable();

public:

	ULONG Worker();

private:

	void SuspendWorker();
	void ResumeWorker();
	void TerminateWorker();

public:

	int Recv(PUCHAR Buffer, int Length);
	int Send(PUCHAR Buffer, int Length);

private:

	bool IsOwnAID(ULONG SID);

	void PushCondition(USHORT Type, UCHAR Flag);
	void PopCondition(USHORT Type);

	void NoticeState(PSTR String, UCHAR Flag);
	void NoticeAbnormal(PSTR String);

	void EnchantArms(USHORT Type, UCHAR Flag, bool EnablePush);
	void ItemEnchantArms(USHORT Type);
	void DeleteItemEnchantArms();
	void CheckItemEnchantArms(ULONG Tick);

	void ConnFirstProc(PUCHAR Buffer);

private:

	void OwnOption(PUCHAR Buffer);
	void OwnCondition(PUCHAR Buffer);

	void ClAuthAck(PUCHAR Buffer);
	void ClCCEnterAck(PUCHAR Buffer);
	void ClZCEnterAck(PUCHAR Buffer);
	void ClEraseActor(PUCHAR Buffer);
	void ClOption(PUCHAR Buffer);
	void ClCondition(PUCHAR Buffer);
	void ClSkillTmp(PUCHAR Buffer);
	void ClSkillAck(PUCHAR Buffer);
//	void ClShortcut(PUCHAR Buffer);
//	void ClSeedAck(PUCHAR Buffer);
//	void ClNProtect(PUCHAR Buffer);
};


#endif	// #ifndef RPARSER_H
