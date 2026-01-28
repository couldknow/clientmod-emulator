#include "framework.h"

#define FRAGMENT_BITS		8
#define FRAGMENT_SIZE		(1<<FRAGMENT_BITS)
#define MAX_FILE_SIZE		((1<<MAX_FILE_SIZE_BITS)-1)	// maximum transferable size is	64MB
#define MAX_FILE_SIZE_BITS 26
#define NET_MAX_PALYLOAD_BITS 17
#define MAX_STREAMS 2

DWORD dwSetStringUserData = 0;
DWORD dwCvarSetValue = 0;
DWORD dwBuildConVarUpdateMessage = 0;
DWORD dwDownloadManager_Queue = 0;
DWORD dwFindClientClass = 0;
DWORD dwSVC_ServerInfo_ReadFromBuffer = 0;
DWORD NC = 0; //INetChannel
DWORD NetChannel_SendNetMsg = 0;
DWORD dwDispatchUserMessage = 0;
DWORD dwSendNetMsg = 0;
DWORD dwReadSubChannelData = 0;

std::default_random_engine generator(time(0));
std::uniform_int_distribution<uint32_t> distribution(1, MAXINT);
std::uniform_int_distribution<uint32_t> friendsID(0xD000000, 0xDD00000);

ofstream logfile;

template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}

typedef bool(__thiscall* PrepareSteamConnectResponseFn)(void*, int, const char*, uint64, bool, const netadr_t&, bf_write&);
bool __fastcall Hooked_PrepareSteamConnectResponse(DWORD* ecx, void* edx, int keySize, const char* encryptionKey, uint64 unGSSteamID, bool bGSSecure, const netadr_t& adr, bf_write& msg)
{
	printfdbg("PrepareSteamConnectResponse called\n");

	static PrepareSteamConnectResponseFn PrepareSteamConnectResponse = (PrepareSteamConnectResponseFn)dwPrepareSteamConnectResponse;
	if (!g_pCVar->FindVar("cm_steamid_enabled")->GetInt())
		PrepareSteamConnectResponse(ecx, keySize, encryptionKey, unGSSteamID, bGSSecure, adr, msg);

	srand(time(NULL));
	unsigned int steamid = 0;
	if (g_pCVar->FindVar("cm_steamid_random")->GetInt())
		steamid = distribution(generator);
	else
		steamid = g_pCVar->FindVar("cm_steamid")->GetInt();

	msg.WriteShort(0x98);
	msg.WriteLong('S');

	char hwid[64];
	generateRandomHWID(hwid); //CreateRandomString(hwid, 32);
	if (!RevSpoofer::Spoof(hwid, steamid)) {
		printfdbg("RevSpoofer::Spoof ERROR\n");
		//CallVFunction<IVEngineClient* (__thiscall*)(void*, char*)>(g_pEngineClient, 97)(g_pEngineClient, "retry");
		g_pEngineClient->ExecuteClientCmd("retry");
		return false;
	}

	DWORD dwRevHash = RevSpoofer::Hash(hwid);

	msg.WriteLong(dwRevHash);
	msg.WriteLong('rev');
	msg.WriteLong(NULL);
	msg.WriteLong(dwRevHash * 2);
	msg.WriteLong(0x01100001);

	static const char AESKey[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
	auto AESRand = CRijndael();
	char AESHashRand[32];
	AESRand.MakeKey(AESKey, CRijndael::sm_chain0, 32, 32);
	AESRand.EncryptBlock(hwid, AESHashRand);
	msg.WriteBytes(AESHashRand, 32);

	auto AESRev = CRijndael();
	char AESHashRev[32];
	AESRev.MakeKey("_YOU_SERIOUSLY_NEED_TO_GET_LAID_sJ_r$WVsH%zRq&v$fl3jCY7SK3Em3s%f", CRijndael::sm_chain0, 32, 32);
	AESRev.EncryptBlock(AESKey, AESHashRev);
	msg.WriteBytes(AESHashRev, 32);

	auto sha = CSHA(CSHA::SHA256);
	char SHAHash[32];
	sha.AddData(hwid, 32);
	sha.FinalDigest(SHAHash);
	msg.WriteBytes(SHAHash, 32);

	for (size_t i = 1; i <= 32; i++)
	{
		msg.WriteByte(0);
	}

	char staticEnd[] = {
		0xA4, 0x00,
		0x4A, 0x00, 0x00, 0x00, 0x35, 0xC7, 0x4B, 0x8B, 0x76, 0x65, 0x72, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x6A, 0x8E, 0x97, 0x16, 0x01, 0x00, 0x10, 0x01,
		0x31, 0x32, 0x38, 0x38, 0x37, 0x36, 0x37, 0x30, 0x36, 0x31, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	return true;
}


struct CM
{
	int NumPlayers = 0;
	int UserID = 0;
	char Map[255];
	int Port = 0;
	int MaxPlayers = 0;
	int ServerCount = 0;
	int FriendsID = 0;
	int PlayerSlot = 0;
}; CM* _CM = new CM;

class CNetMessage : public INetMessage
{
public:
	CNetMessage() {
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	virtual ~CNetMessage() {};

	virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
	INetChannel* GetNetChannel() const { return m_NetChannel; }

	virtual void	SetReliable(bool state) { m_bReliable = state; };
	virtual bool	IsReliable() const { return m_bReliable; };
	virtual void    SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
	virtual bool	Process() { Assert(0); return false; };	// no handler set

	bool				m_bReliable;	// true if message should be send reliable
	INetChannel* m_NetChannel;	// netchannel this message is from/for
};

class NET_SetConVar : public CNetMessage
{
	DECLARE_NET_MESSAGE(SetConVar);

	int	GetGroup() const { return INetChannelInfo::STRINGCMD; }

	NET_SetConVar() {}
	NET_SetConVar(const char* name, const char* value)
	{
		cvar_t cvar;
		strncpy(cvar.name, name, MAX_PATH);
		strncpy(cvar.value, value, MAX_PATH);
		m_ConVars.AddToTail(cvar);
	}

public:

	typedef struct cvar_s
	{
		char	name[MAX_PATH];
		char	value[MAX_PATH];
	} cvar_t;

	CUtlVector<cvar_t> m_ConVars;
};

class CLC_RespondCvarValue : public CNetMessage
{
public:
	DECLARE_CLC_MESSAGE(RespondCvarValue);
	QueryCvarCookie_t		m_iCookie;
	const char* m_szCvarName;
	const char* m_szCvarValue;	// The sender sets this, and it automatically points it at m_szCvarNameBuffer when receiving.
	EQueryCvarValueStatus	 m_eStatusCode;
private:
	char		m_szCvarNameBuffer[256];
	char		m_szCvarValueBuffer[256];
};

class SVC_GameEventList : public CNetMessage
{
public:
	DECLARE_SVC_MESSAGE(GameEventList);

	int			m_nNumEvents;
	int			m_nLength;
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_GameEvent : public CNetMessage
{
	DECLARE_SVC_MESSAGE(GameEvent);

	int	GetGroup() const { return INetChannelInfo::EVENTS; }

public:
	int			m_nLength;	// data length in bits
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

class SVC_GetCvarValue : public CNetMessage
{
public:
	DECLARE_SVC_MESSAGE(GetCvarValue);

	QueryCvarCookie_t	m_iCookie;
	const char* m_szCvarName;	// The sender sets this, and it automatically points it at m_szCvarNameBuffer when receiving.

private:
	char		m_szCvarNameBuffer[256];
};

const char* CLC_RespondCvarValue::ToString(void) const { return 0; }
bool CLC_RespondCvarValue::ReadFromBuffer(bf_read& buffer) { return 0; }
bool CLC_RespondCvarValue::WriteToBuffer(bf_write& buffer) { return 0; }

class CLC_ListenEvents : public CNetMessage
{
	DECLARE_CLC_MESSAGE(ListenEvents);

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:
	CBitVec<MAX_EVENT_NUMBER> m_EventArray;
};

struct CLC_ClientInfo {
	char pad0[0x10];
	uint32 m_nServerCount;
	uint32 m_nSendTableCRC;
	bool IsHLTV;
	uint32	m_nFriendsID;
	char m_FriendsName[32];
	uint32 m_nCustomFiles[4]; //CustomFileCRC
};

typedef void(__cdecl* BuildConVarUpdateMessageFn)(NET_SetConVar*, int, bool);

#define AddtoTailWithVal(cvar, val) strncpy(acvar.name, XorStr(cvar), MAX_PATH);\
strncpy(acvar.value, val, MAX_PATH);\
cvarMsg->m_ConVars.AddToTail(acvar);

#define AddtoTail(cvar) strncpy(acvar.name, XorStr(cvar), MAX_PATH);\
strncpy(acvar.value, g_pCVar->FindVar(cvar)->GetString(), MAX_PATH);\
cvarMsg->m_ConVars.AddToTail(acvar);


//https://github.com/VSES/SourceEngine2007/blob/master/src_main/engine/host.cpp
void Hooked_BuildConVarUpdateMessage(NET_SetConVar* cvarMsg, int flags, bool nonDefault)
{
	printfdbg("Hooked_BuildConVarUpdateMessage called\n");

	static BuildConVarUpdateMessageFn BuildConVarUpdateMessage = (BuildConVarUpdateMessageFn)dwBuildConVarUpdateMessage;

	BuildConVarUpdateMessage(cvarMsg, flags, nonDefault);

	NET_SetConVar::cvar_t acvar;

	if (g_pCVar->FindVar("cm_enabled")->GetInt())
	{
		cvarMsg->m_ConVars.RemoveAll();
		AddtoTail("cl_team");
		AddtoTail("cl_updaterate");
		AddtoTailWithVal("_client_version", g_pCVar->FindVar("cm_version")->GetString());
		AddtoTail("cl_interp");
		AddtoTailWithVal("~clientmod", "3.0");
		AddtoTail("cl_lagcompensation");
		AddtoTail("cl_interp_npcs");
		AddtoTail("cl_interpolate");
		AddtoTail("cl_cmdrate");
		AddtoTail("cl_language");
		AddtoTail("english");
		AddtoTail("name");
		AddtoTail("cl_autohelp");
		AddtoTail("cl_predictweapons");
		AddtoTail("cl_rebuy");
		AddtoTail("cl_class");
		AddtoTailWithVal("clantag", "");
		AddtoTail("tv_nochat");
		AddtoTail("hap_HasDevice");
		AddtoTail("cl_predict");
		AddtoTail("cl_spec_mode");
		AddtoTail("rate");
		AddtoTail("cl_autobuy");
		AddtoTail("cl_interp_ratio");
		AddtoTail("closecaption");
		AddtoTailWithVal("voice_loopback", "0");
		AddtoTail("cl_autowepswitch");
	}

	for (int i = 0; i < cvarMsg->m_ConVars.Size(); i++) {
		printfdbg("%d %s : %s\n", i, cvarMsg->m_ConVars[i].name, cvarMsg->m_ConVars[i].value);
	}

}


static bool IsSafeFileToDownload(const char* pFilename)
{
	printfdbg("S2C Downloading %s\n", pFilename);
	// No absolute paths or weaseling up the tree with ".." allowed.
	if (V_strstr(pFilename, ":")
		|| V_strstr(pFilename, ".."))
	{
		return false;
	}

	// Only files with 3-letter extensions allowed.
	const char* pExt = V_strrchr(pFilename, '.');
	if (!pExt || V_strlen(pExt) != 4)
		return false;

	// Don't allow any of these extensions.
	if (V_stricmp(pExt, ".cfg") == 0
		|| V_stricmp(pExt, ".lst") == 0
		|| V_stricmp(pExt, ".exe") == 0
		|| V_stricmp(pExt, ".asi") == 0
		|| V_stricmp(pExt, ".mix") == 0
		|| V_stricmp(pExt, ".flt") == 0
		|| V_stricmp(pExt, ".vbs") == 0
		|| V_stricmp(pExt, ".com") == 0
		|| V_stricmp(pExt, ".bat") == 0
		|| V_stricmp(pExt, ".dll") == 0
		|| V_stricmp(pExt, ".ini") == 0
		|| V_stricmp(pExt, ".log") == 0)
	{
		printfdbg("S2C Incoming invalid file '%s'\n", pFilename);
		return false;
	}

	// Word.
	return true;
}


class CGameEventCallback
{
public:
	void* m_pCallback;		// callback pointer
	int					m_nListenerType;	// client or server side ?
};

class CGameEventDescriptor
{
public:
	CGameEventDescriptor()
	{
		name[0] = 0;
		eventid = -1;
		keys = NULL;
		local = false;
		reliable = true;
	}

public:
	char		name[32];	// name of this event
	int			eventid;	// network index number, -1 = not networked
	KeyValues* keys;		// KeyValue describing data types, if NULL only name 
	bool		local;		// local event, never tell clients about that
	bool		reliable;	// send this event as reliable message
	CUtlVector<CGameEventCallback*>	listeners;	// registered listeners
};

CGameEventDescriptor* GetEventDescriptor(CGameEventDescriptor* descriptors, int count, const char* name)
{
	for (size_t i = 0; i < count; i++)
	{
		if (!strcmp(descriptors[i].name, name))
			return &descriptors[i];
	}
	return NULL;
}

bool ProcessControlMessage(INetChannel* chan, int cmd, bf_read& buf)
{
	char string[1024];

	if (cmd == net_NOP)
	{
		return true;
	}

	INetChannelHandler* m_MessageHandler = CallVFunction<INetChannelHandler * (__thiscall*)(void*)>(chan, 45)(chan); //INetChannel::GetMsgHandler  

	printfdbg("ProcControlMessage %d Channel %x Handler %x\n", cmd, chan, m_MessageHandler);

	if (cmd == net_Disconnect)
	{
		buf.ReadString(string, sizeof(string));
		printfdbg("Connection closing: %s\n", string);

		if (!srcds)
			CallVFunction<void(__thiscall*)(void*, char*)>(chan, 0x20)(chan, string); //INetChannel::Disconnect 

		return false;
	}

	if (cmd == net_File)
	{
		unsigned int transferID = buf.ReadUBitLong(32);
		buf.ReadString(string, sizeof(string));

		if (buf.ReadOneBit() != 0 && IsSafeFileToDownload(string))
		{
			printfdbg("FileRequested %s\n", string);
			m_MessageHandler->FileRequested(string, transferID);
		}
		else
		{
			printfdbg("FileDenied %s\n", string);
			m_MessageHandler->FileDenied(string, transferID);
		}

		return true;
	}

	printfdbg("Netchannel: received bad control cmd %i from %s.\n", cmd, chan->GetAddress());
	return false;
}

// ---------------------------------------------------- //

class CNetChan;

INetMessage* FindMessage(INetChannel* ecx, int type)
{
	INetChannel* v2; // edi@1
	int numtypes; // ebx@1
	int idx; // esi@1
	INetMessage* result; // eax@4

	v2 = ecx;
	numtypes = *((DWORD*)ecx + 1918);
	idx = 0;
	if (numtypes <= 0)
	{
	LABEL_4:
		result = 0;
	}
	else
	{
		while ((*(int (**)(void))(**(DWORD**)(*((DWORD*)v2 + 1915) + 4 * idx) + 28))() != type)
		{
			if (++idx >= numtypes)
				goto LABEL_4;
		}
		result = *(INetMessage**)(*((DWORD*)v2 + 1915) + 4 * idx);
	}
	return result;
}


const char* GetEventName(int eventid)
{
	void* EDI;
	int count;
	CGameEventDescriptor* descriptors;
	__asm
	{
		mov eax, g_GameEventManager
		mov edx, [eax + 0x10]
		mov count, edx
		mov edx, [eax + 0x4]
		mov descriptors, edx
	}
	for (size_t i = 0; i < count; i++)
	{
		if (descriptors[i].eventid == eventid)
			return descriptors[i].name;
	}

	return "";
}

DWORD eip_;
__declspec(naked) void getEIP()
{
	__asm pushad
	__asm mov eax, [esp + 0x20]
		__asm mov[eip_], eax
	printfdbg("EIP %x\n", eip_);
	__asm popad
	__asm ret
}

int m_nHeaderBase = -1;
int m_nNewEntity = -1;
int m_nOldEntity = -1;

// ---------------------------------------------------- //
void ReturnCvarValue(INetChannel* pThis, EQueryCvarValueStatus status, QueryCvarCookie_t cookie, const char* cvarname, char* value_to_pass)
{
	CLC_RespondCvarValue returnMsg;
	memcpy(&returnMsg, &NC, 4);
	returnMsg.m_iCookie = cookie;
	returnMsg.m_szCvarName = cvarname;
	returnMsg.m_szCvarValue = value_to_pass;
	returnMsg.m_eStatusCode = status;
	((void(__thiscall*)(void*, CLC_RespondCvarValue*))NetChannel_SendNetMsg)(pThis, &returnMsg);
}

#define RespondCvarValue(name, value, status) \
if (!stricmp((char*)((DWORD)netmsg + 24), name)) \
{\
ReturnCvarValue(pThis, status, msgmsg->m_iCookie, name, value);\
continue;\
}

typedef bool(__thiscall* FunctionFn)(INetChannel*, bf_read&);
bool __fastcall Hooked_ProcessMessages(INetChannel* pThis, void* edx, bf_read& buf)
{
	static FunctionFn Function = (FunctionFn)dwProcessMessages;

	while (true)
	{
		if (buf.IsOverflowed())
		{
			printfdbg("Buffer overflow in net message\n");
			return false;
		}

		// Are we at the end?
		if (buf.GetNumBitsLeft() < NETMSG_TYPE_BITS)
		{
			break;
		}

		unsigned char cmd = buf.ReadUBitLong(NETMSG_TYPE_BITS);

		if (cmd <= net_File)
		{
			if (!ProcessControlMessage(pThis, cmd, buf))
			{
				return false; // disconnect or error
			}

			continue;
		}

		INetMessage* netmsg = FindMessage(pThis, cmd);

		if (netmsg)
		{
			bf_read backup = buf;

			if (cmd == svc_GameEvent)
			{
				int length = buf.ReadUBitLong(11);
				int eventid = buf.ReadUBitLong(MAX_EVENT_BITS);
				const char* name = GetEventName(eventid);

				printfdbg("svc_GameEvent: %s (%d)\n", name, eventid);

				if (name && !strcmp(name, "player_disconnect"))
				{
					short userid = (short)buf.ReadUBitLong(16);// buf.ReadShort();
					char reason[1024];
					buf.ReadString(reason, sizeof(reason));
					char name[1024];
					buf.ReadString(name, sizeof(name));
					char networkid[1024];
					buf.ReadString(networkid, sizeof(networkid));
					printfdbg("player_disconnect %d name %s reason %s networkid %s\n", userid, name, reason, networkid);

					//if (userid < 1)
					continue;
				}

				if (name && !strcmp(name, "player_info"))
				{
					char databuf[1024];
					buf.ReadString(databuf, sizeof(databuf));
					printfdbg("player_info buffer %s\n", databuf);
					buf.ReadString(databuf, sizeof(databuf));
					continue;
				}

				buf = backup;
			}

			if (cmd == svc_UserMessage)
			{
				auto msgType = buf.ReadByte();
				auto dataLengthInBits = buf.ReadUBitLong(11);
				//assert(math::BitsToBytes(data->dataLengthInBits) <= MAX_USER_MSG_DATA);
				char databuf[1024];
				buf.ReadBits(databuf, dataLengthInBits);

				if (msgType < 0 || msgType >= (*(DWORD**)CUserMessages)[5])
				{
					printfdbg("UserMsg Rejected: type %d dataLengthInBits %d\n", msgType, dataLengthInBits);
					continue;
				}

				buf = backup;
			}

			if (!srcds) {

				if (cmd == svc_ServerInfo)
				{
					printfdbg("svc_ServerInfo:\n");
					printfdbg("m_nProtocol %d\n", (uint16)buf.ReadUBitLong(16));
					_CM->ServerCount = (uint32)buf.ReadUBitLong(32);
					printfdbg("m_nServerCount %d\n", _CM->ServerCount);
					printfdbg("m_bIsHLTV %d\n", (uint8)buf.ReadOneBit() != 0);
					printfdbg("m_bIsDedicated %d\n", (uint8)buf.ReadOneBit() != 0);
					printfdbg("m_nClientCRC %x\n", (long)buf.ReadLong());
					printfdbg("m_nMaxClasses %d\n", (WORD)buf.ReadWord());
					printfdbg("m_nMapCRC %x\n", (long)buf.ReadLong());
					_CM->PlayerSlot = (uint8)buf.ReadByte();
					printfdbg("m_nPlayerSlot %d\n", _CM->PlayerSlot);
					_CM->MaxPlayers = (uint8)buf.ReadByte();
					printfdbg("m_nMaxClients %d\n", _CM->MaxPlayers);
					printfdbg("m_fTickInterval %f\n", (float32)buf.ReadFloat());
					printfdbg("m_cOS %c\n", (uint8)buf.ReadChar());
					char GameDir[1024]; buf.ReadString(GameDir, sizeof(GameDir));
					buf.ReadString(_CM->Map, sizeof(_CM->Map));
					char SkyName[1024]; buf.ReadString(SkyName, sizeof(SkyName));
					char HostName[1024]; buf.ReadString(HostName, sizeof(HostName));
					printfdbg("m_szGameDirBuffer %s\n", GameDir);
					printfdbg("m_szMapNameBuffer %s\n", _CM->Map);
					printfdbg("m_szSkyNameBuffer %s\n", SkyName);
					printfdbg("m_szHostNameBuffer %s\n", HostName);
					buf = backup;
				}

				if (cmd == svc_Menu)
				{
					short Type = (short)buf.ReadUBitLong(16);
					auto dataLength = buf.ReadUBitLong(16);
					char databuf[4096];
					buf.ReadBytes(databuf, dataLength);
					printfdbg("svc_Menu Rejected: type %d dataLength %d\n", Type, dataLength);
					continue;
				}

				if (cmd == net_StringCmd)
				{
					char stringcmd[1024];
					buf.ReadString(stringcmd, sizeof(stringcmd));
					printfdbg("Net_StringCmd Rejected: %s\n", stringcmd);
					continue;
				}

			}

			if (!netmsg->ReadFromBuffer(buf))
			{
				printfdbg("Netchannel: failed reading message %s from %s.\n", netmsg->GetName(), pThis->GetAddress());
				return false;
			}

			if (cmd != net_Tick && cmd != svc_PacketEntities && cmd != svc_UserMessage && cmd != clc_Move &&
				cmd != svc_Sounds && cmd != svc_GameEvent && cmd != svc_TempEntities)
			{
				printfdbg("Income msg %d from %s: %s\n", cmd, pThis->GetAddress(), netmsg->ToString());
				if (!srcds)
					if (cmd == svc_FixAngle || cmd == svc_SetPause)
					{
						//printf(" Rejected\n");
						continue;
					}
				//printf("\n");
			}

			if (cmd == svc_GetCvarValue)
			{
				SVC_GetCvarValue* msgmsg = (SVC_GetCvarValue*)netmsg;

				RespondCvarValue("cm_steamid", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_steamid_random", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_steamid_enabled", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_version", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_enabled", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_forcemap", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_drawspray", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_fakeconnect", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("cm_log", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("se_lkblox", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("se_autobunnyhopping", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("se_disablebunnyhopping", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_viewmodel_right", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_viewmodel_fov", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_viewmodel_up", "0", eQueryCvarValueStatus_ValueIntact);

				RespondCvarValue("se_respawn_on_death", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_blood_scale", "1", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_bob_lower_amt", "21", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("mat_potato_mode", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("mat_async_tex_maxtime_ms", "0.5", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("mat_colcorrection_disableentities", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("se_doubleduck", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("se_nowinpanel", "1", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("se_newsmoke", "14", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("e_showserverinfo", "0", eQueryCvarValueStatus_ValueIntact);

				RespondCvarValue("async_toggle_priority", "", eQueryCvarValueStatus_CvarNotFound);
				RespondCvarValue("_client_version", (char*)g_pCVar->FindVar("cm_version")->GetString(), eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("~clientmod", "3.0", eQueryCvarValueStatus_ValueIntact);

				RespondCvarValue("net_blockmsg", "none", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("net_compresspackets_minsize", "128", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("windows_speaker_config", "4", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("net_compresspackets", "1", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("pyro_vignette", "2", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("cl_minmodels", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("cl_min_ct", "1", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("cl_min_t", "1", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("cl_downloadfilter", "all", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("voice_inputfromfile", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("voice_loopback", "0", eQueryCvarValueStatus_ValueIntact);
				RespondCvarValue("sv_cheats", "0", eQueryCvarValueStatus_ValueIntact);
			}

			if (srcds)
			{
				if (cmd == net_SetConVar)
				{
					NET_SetConVar* msgmsg = (NET_SetConVar*)netmsg;
					if (msgmsg->m_ConVars.Count() > 1)
						for (int i = 0; i < msgmsg->m_ConVars.Count(); i++)
							printfdbg("NET_SetConVar %d %s -> %s\n", i, msgmsg->m_ConVars[i].name, msgmsg->m_ConVars[i].value);
				}

				if (cmd == clc_ClientInfo)
				{
					CLC_ClientInfo* Cl = (CLC_ClientInfo*)netmsg;
					printfdbg("clc_ClientInfo m_nFriendsID: %x m_FriendsName: %s\n", Cl->m_nFriendsID, Cl->m_FriendsName);
				}

				if (cmd == clc_ListenEvents)
				{
					CLC_ListenEvents* msgmsg = (CLC_ListenEvents*)netmsg;
					for (int i = 0; i < MAX_EVENT_NUMBER; i++)
						if (msgmsg->m_EventArray.Get(i)) {
							printfdbg("clc_ListenEvents %d: %s\n", i, GetEventName(i));
						}

					printfdbg("Bitset: ");
					for (int i = 0; i < 0x10; i++)
						printfdbg("%08x ", *(uint*)((int)netmsg + 0x10 + i * 4));
					printfdbg("\n");
				}
			}

			if (!netmsg->Process())
			{
				printfdbg("Netchannel: failed processing message %s.\n", netmsg->GetName());
				return false;
			}
		}
		else
		{
			printfdbg("Netchannel: unknown net message (%i) from %s.\n", cmd, pThis->GetAddress());
			return false;
		}
	}

	return true;
}

enum
{
	SERVERSIDE = 0,		// this is a server side listener, event logger etc
	CLIENTSIDE,			// this is a client side listenet, HUD element etc
	CLIENTSTUB,			// this is a serverside stub for a remote client listener (used by engine only)
	SERVERSIDE_OLD,		// legacy support for old server event listeners
	CLIENTSIDE_OLD,		// legecy support for old client event listeners
};

//Ultr@Hook fix 
void __fastcall hkWriteListenEventList(CGameEventManager* _this, void* edx, int msg) //SVC_GameEventList*
{
	int totalListened = 0;

	for (int i = 0; i < 0x10; i++)
		*(int*)(msg + 0x10 + i * 4) = 0;

	int EventCount = *(int*)((int)_this + 0x10);
	int EventNames = *(int*)((int)_this + 4);

	for (int j = 0; j < EventCount; j++)
	{
		CGameEventDescriptor* _descriptor = (CGameEventDescriptor*)(j * 0x40 + EventNames);
		//printfdbg("Event %d: %s %d\n", iterator, _descriptor->name, _descriptor->listeners.Count()); 
		bool bHasClientListener = false;
		for (int i = 0; i < _descriptor->listeners.Count(); i++) { // *(int*)(descriptor + 0x38)
			CGameEventCallback* listener = _descriptor->listeners[i]; // *(int*)(*(int*)(descriptor + 0x2c) + i * 4);
			if ((listener->m_nListenerType == CLIENTSIDE) || (listener->m_nListenerType == CLIENTSIDE_OLD)) { //(*(int*)(listener + 4)
				bHasClientListener = true;
				break;
			}
		}
		if ((bHasClientListener) && (_descriptor->eventid != -1))
		{
			if (strcmp(_descriptor->name, "player_say") && strcmp(_descriptor->name, "player_hurt"))
			{
				uint uVar5 = _descriptor->eventid; //*(uint*)(descriptor + 0x20);  
				//msg->add_event_mask( EventArray.GetDWord( i ) );
				*(uint*)(msg + 0x10 + (uVar5 >> 5) * 4) =
					1 << ((uint8)uVar5 & 0x1f) | *(uint*)(msg + 0x10 + (uVar5 >> 5) * 4);
				printfdbg("Listening Event %d: %s %x\n", uVar5, _descriptor, *(uint*)(msg + 0x10 + (uVar5 >> 5) * 4));
				totalListened++;
			}
		}
	}

	printfdbg("WriteListenEventList: Total %d events listened. Bitset: ", totalListened);
	for (int i = 0; i < 0x10; i++)
		printf("%08x ", *(uint*)(msg + 0x10 + i * 4)); //bits 0cc8a1c5 00000e0e 00003e60 
	printf("\n");

	return;
}

void BinaryToReadable(unsigned char* in, size_t inlen, std::string& out)
{
	char buf[4];
	for (size_t i = 0; i < inlen; i++)
	{
		auto c = in[i];
		snprintf(buf, sizeof(buf), (c <= ' ' || c >= 'y') ? "%02x" : "%01c", c);
		out.append(buf);
	}
}

void GenerateFriendsName(char* friendsName, size_t uMaxLength)
{
	MD5Context_t ctx;
	unsigned char digest[16];

	MD5Init(&ctx);

	ctx.buf[0] += 0x20;
	ctx.buf[1] -= 0x12;
	ctx.buf[2] += 0x79;
	ctx.buf[3] -= 0x8B;

	char md5string[255];
	sprintf(md5string, "7LRUT827L05D4GX7AG2LFR5NFI2SOHQ0%d0%d0%s%d%d%d%uDWXU38QN7A0X783WL2585UD753U0D6RE",
		_CM->PlayerSlot + 1, _CM->UserID, _CM->Map, _CM->Port, _CM->MaxPlayers, _CM->ServerCount, _CM->FriendsID);

	printfdbg("Key %s\n", md5string);
	MD5UpdateString(&ctx, md5string);

	MD5Final(digest, &ctx);

	for (size_t i = 0; i < sizeof(digest); i++)
	{
		digest[i] ^= 0x12; // static key
	}

	std::string readableHash;
	BinaryToReadable(digest, sizeof(digest), readableHash);

	CRC32_t checksum = -1;
	CRC32_ProcessBuffer(&checksum, digest, sizeof(digest));

	checksum = ~checksum;

	char szReadableHash[32];
	strncpy(szReadableHash, readableHash.c_str(), sizeof(szReadableHash));

	unsigned __int32 args[2];
	args[1] = *(DWORD*)szReadableHash;
	args[0] = checksum;

	auto finalHash = friends_name_hash((unsigned char*)szReadableHash, sizeof(szReadableHash), *(unsigned __int64*)&args);

	snprintf(friendsName, uMaxLength, "%16llX", finalHash);
}

typedef bool(__thiscall* pSendNetMsg)(INetChannel* pNetChan, INetMessage& msg, bool bVoice);
bool __fastcall hkSendNetMsg(INetChannel* this_, void* edx, INetMessage& msg, bool bVoice)
{
	int cmd = msg.GetType();
	if (cmd != net_Tick && cmd != clc_Move && cmd != svc_UserMessage && cmd != svc_GameEvent && cmd != clc_BaselineAck)
		printfdbg("Outcome msg %d: %s\n", cmd, msg.ToString()); //msg.GetName()

	if (!srcds) {
		if (cmd == net_SignonState)
		{
			uint8 m_nSignonState = *(DWORD*)((DWORD)&msg + 0x10);

			if (textmode && (m_nSignonState == SIGNONSTATE_FULL))
			{
				CallVFunction<IVEngineClient* (__thiscall*)(void*, char*)>(g_pEngineClient, 97)(g_pEngineClient,
					"jointeam; +voicerecord");
			}

			if (m_nSignonState == g_pCVar->FindVar("cm_fakeconnect")->GetInt() + 1) //2-6
			{
				auto clientport = g_pCVar->FindVar("clientport");
				clientport->SetValue(clientport->GetInt() + 1);
				printfdbg("Set client port to %d\n", clientport->GetInt());
				*(BYTE*)(dwDisconnectMessage - 5) = 0xEB;
				CallVFunction<IVEngineClient* (__thiscall*)(void*, char*)>(g_pEngineClient, 97)(g_pEngineClient,
					"disconnect; net_start; retry");
				*(BYTE*)(dwDisconnectMessage - 5) = 0x74;
				return false;
			}
		}
	}

	if (cmd == svc_UserMessage)
	{
		uint8 usermsgID = *(DWORD*)((DWORD)&msg + 0x10);
		printfdbg("svc_UserMessage %s (%d)\n", ((char* (__thiscall*)(void*, int))dwGetUserMessageName)((*(DWORD**)CUserMessages), usermsgID), usermsgID);
	}

	if (cmd == svc_GameEvent)
	{
		uint8 eventID = *(DWORD*)((DWORD)&msg + 0x44);
		printfdbg("svc_GameEvent: %s (%d).\n", GetEventName(eventID), eventID);
	}

	if (cmd == clc_ClientInfo)
	{
		if (g_pCVar->FindVar("cm_enabled")->GetInt())
		{
			CLC_ClientInfo* Cl = (CLC_ClientInfo*)&msg;
			Cl->m_nFriendsID = friendsID(generator);
			_CM->FriendsID = Cl->m_nFriendsID;

			string addr = string(this_->GetAddress());
			addr = addr.substr(addr.find(":") + 1, addr.size());
			_CM->Port = stoi(addr.c_str());

			GenerateFriendsName(Cl->m_FriendsName, 16);
		}
		/*
		Cl->m_nCustomFiles[0] = 0;
		Cl->m_nCustomFiles[1] = 0;
		Cl->m_nCustomFiles[2] = 0;
		Cl->m_nCustomFiles[3] = 0;
		*/
	}

	static pSendNetMsg SendNetMsg = (pSendNetMsg)dwSendNetMsg;
	return SendNetMsg(this_, msg, bVoice);
}


typedef bool(__thiscall* pDispatchUserMessage)(void* this_, int msg_type, bf_read& msg_data);
bool __fastcall hkDispatchUserMessage(DWORD* this_, void* edx, int msg_type, bf_read& msg_data)
{
	if (msg_type < 0 || msg_type >= this_[5])
		return false;

	if (msg_type == 11 || msg_type == 12) //Shake Fade 
	{
		printfdbg("svc_UserMessage: %s (%d) Rejected\n", ((char* (__thiscall*)(void*, int))dwGetUserMessageName)(this_, msg_type), msg_type);
		return true;
	}
	else
		if (msg_type == 13) //VGUIMenu
		{
			bf_read backup = msg_data;
			char name[1024];
			msg_data.ReadString(name, sizeof(name));
			printfdbg("svc_UserMessage: %s (%d): %s\n", ((char* (__thiscall*)(void*, int))dwGetUserMessageName)(this_, msg_type), msg_type, name);

			if (!strcmp(name, XorStr("info")))
			{
				msg_data.ReadByte(); msg_data.ReadByte();
				char title[1024]; char content[1024];
				memset(title, 0, sizeof(title)); memset(content, 0, sizeof(content));

				msg_data.ReadString(title, sizeof(title));
				while (title[0])
				{
					msg_data.ReadString(content, sizeof(content));
					printfdbg("%s: %s\n", title, content);
					memset(title, 0, sizeof(title)); memset(content, 0, sizeof(content));
					msg_data.ReadString(title, sizeof(title));
				}

				return true;
			}

			msg_data = backup;
		}
		else
			printfdbg("svc_UserMessage: %s (%d)\n", ((char* (__thiscall*)(void*, int))dwGetUserMessageName)(this_, msg_type), msg_type);


	static pDispatchUserMessage DispatchUserMessage = (pDispatchUserMessage)dwDispatchUserMessage;
	return DispatchUserMessage(this_, msg_type, msg_data);
}

typedef void(__thiscall* pDownloadManager_Queue)(DWORD* this_, char* Source, char* Str);
void __fastcall hkDownloadManager_Queue(DWORD* this_, void* unk, char* baseURL, char* gamePath)
{
	if (!strcmp(g_pCVar->FindVar("cl_downloadfilter")->GetString(), "mapsonly") && strcmp(PathFindExtensionA(gamePath), ".bsp"))
		return;
	printfdbg("Downloading %s from %s\n", baseURL, gamePath);
	static pDownloadManager_Queue DownloadManager_Queue = (pDownloadManager_Queue)dwDownloadManager_Queue;
	return DownloadManager_Queue(this_, baseURL, gamePath);
}

int ClassID;
typedef int(__cdecl* pFindClientClass)(char* event_name);
int __cdecl hkFindClientClass(char* event_name)
{
	__asm mov ClassID, edx
	printfdbg("svc_TempEntities: %s (%d)\n", event_name, ClassID >> 4);
	if (!(g_pCVar->FindVar("cm_drawspray")->GetInt()) && !strcmp(event_name, "CTEPlayerDecal")) return false;
	static pFindClientClass FindClientClass = (pFindClientClass)dwFindClientClass;
	return FindClientClass(event_name);
}

typedef char* (__thiscall* pSVC_ServerInfo_ReadFromBuffer)(int this_, int buf);
bool __fastcall hkSVC_ServerInfo_ReadFromBuffer(int this_, void* unk, int buf)
{
	static pSVC_ServerInfo_ReadFromBuffer SVC_ServerInfo_ReadFromBuffer = (pSVC_ServerInfo_ReadFromBuffer)dwSVC_ServerInfo_ReadFromBuffer;
	auto ret = SVC_ServerInfo_ReadFromBuffer(this_, buf);

	if (*(uint8*)g_pCVar->FindVar("cm_forcemap")->GetString() != 0)
	{
		int v11 = this_ + 328;
		strncpy((char*)v11, g_pCVar->FindVar("cm_forcemap")->GetString(), 128);
		//*(DWORD*)(this_ + 28) = 4105947211; //m_nMapCRC
	}
	return ret;
}

typedef char* (__thiscall* pSetStringUserData)(DWORD** this_, const void* userdata, int stringNumber, void* length);
bool __fastcall hkSetStringUserData(DWORD** this_, void* unk, char* userdata, int stringNumber, int* length)
{
	char* TableName = (char*)((int(__thiscall*)(DWORD**))(*this_)[1])(this_);

	if (!stricmp(TableName, "downloadables") || !stricmp(TableName, "modelprecache")) {
		if (*(uint8*)g_pCVar->FindVar("cm_forcemap")->GetString() != 0) {
			string usrdata = string(userdata);
			if (usrdata.find("maps/") != string::npos || usrdata.find("maps\\") != string::npos) {
				sprintf(userdata, "maps/%s.bsp", g_pCVar->FindVar("cm_forcemap")->GetString());
			}
		}
	}

	if (length && !stricmp(TableName, "userinfo"))
	{
		_CM->NumPlayers = stoi(userdata);
		if (_CM->NumPlayers == _CM->PlayerSlot)
		{
			_CM->UserID = *(int*)((int)(length)+0x20);
			printf(">> ");
		}
		printfdbg("svc_CreateStringTable %s: %s %s %d %s %x %s\n", TableName, userdata, length, *(int*)((int)(length)+0x20), ((int)(length)+0x24), *(int*)((int)(length)+0x48), ((int)(length)+0x4c));
		_CM->NumPlayers++;
	}
	//else printfdbg("svc_CreateStringTable %s: %d %s\n", TableName, stringNumber, userdata);

	static pSetStringUserData SetStringUserData = (pSetStringUserData)dwSetStringUserData;
	auto ret = SetStringUserData(this_, userdata, stringNumber, length);
	return ret;
}


typedef short(__thiscall* pCvarSetValue)(ConVar* this_, char* String);
short __fastcall hkCvarSetValue(ConVar* this_, void* unk, char* String1)
{
	if (V_stricmp(this_->GetName(), "cm_log") == 0)
	{
		int newlog = -1;
		if (V_stricmp(String1, "0") == 0)
			newlog = 0;
		else if (V_stricmp(String1, "1") == 0)
			newlog = 1;

		if (newlog != -1 && newlog != g_pCVar->FindVar("cm_log")->GetInt())
		{
			//changed
			g_pCVar->FindVar("cm_log")->SetValue(newlog);
			if (newlog == 1)
			{
				//createNewLogFile
				char logname[MAX_PATH];
				auto time = std::time(nullptr);
				std::tm* tm = std::localtime(&time);
				char timebuffer[26];
				std::strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d_%H-%M-%S", tm);
				sprintf(logname, "SpyLog_%s.txt", timebuffer);
				printfdbg("Log name: %s\n", logname);
				logfile.open(logname, std::ofstream::out | std::ofstream::app);
				if (!logfile) {
					cout << "Failed to open\n";
				}
			}

			if (newlog == 0)
			{
				//saveLogFile ;
				printfdbg("Saved log\n");
				logfile.close();
			}
		}
	}

	static pCvarSetValue CvarSetValue = (pCvarSetValue)dwCvarSetValue;
	auto ret = CvarSetValue(this_, String1);

	return ret;
}

typedef char(__thiscall* pReadSubChannelData)(void* this_, bf_read& buf, int stream);
char __fastcall hkReadSubChannelData(void* this_, void* edx, bf_read& buf, int stream)
{
	auto buf_copy = buf;
	bool bSingleBlock = buf.ReadOneBit() == 0; // is single block ? 
	unsigned int startFragment = 0;
	unsigned int numFragments = 0;
	unsigned int offset = 0;
	unsigned int length = 0;
	unsigned int nUncompressedSize = 0;
	unsigned int max_payload_bits = 0;
	unsigned int isFile = 0;
	unsigned int transferID = 0;
	char filename[MAX_PATH] = "";
	bool compressed = 0;
	unsigned int bytes = 0;

	if (!bSingleBlock)
	{
		startFragment = buf.ReadUBitLong(MAX_FILE_SIZE_BITS - FRAGMENT_BITS); // 16 MB max
		numFragments = buf.ReadUBitLong(3);  // 8 fragments per packet max
		offset = startFragment * FRAGMENT_SIZE;
		length = numFragments * FRAGMENT_SIZE;
	}

	if (offset == 0) // first fragment, read header info
	{
		auto max_payload_bits = buf.ReadUBitLong(NET_MAX_PALYLOAD_BITS);
		if (bSingleBlock)
		{
			// data compressed ?
			compressed = buf.ReadOneBit();
			if (compressed)
			{
				nUncompressedSize = buf.ReadUBitLong(MAX_FILE_SIZE_BITS);
			}
			max_payload_bits = buf.ReadUBitLong(NET_MAX_PALYLOAD_BITS);
		}
		else
		{
			isFile = buf.ReadOneBit();
			if (isFile) // is it a file ?
			{
				transferID = buf.ReadUBitLong(32);
				buf.ReadString(filename, MAX_PATH);
			}
			// data compressed ?
			compressed = buf.ReadOneBit();
			if (compressed)
			{
				nUncompressedSize = buf.ReadUBitLong(MAX_FILE_SIZE_BITS);
			}
			bytes = buf.ReadUBitLong(MAX_FILE_SIZE_BITS);
		}
		char* buffer = new char[length];
		buf.ReadBytes(buffer, length); // read data
		delete buffer;
	}

	printfdbg("ReadSubChannelData stream %d bSingleBlock %d offset %d length %d compressed %d isFile %d nUncompressedSize %d\n",
		stream, bSingleBlock, offset, length, compressed, isFile, nUncompressedSize);

	if (nUncompressedSize) return false;

	buf = buf_copy;
	static pReadSubChannelData ReadSubChannelData = (pReadSubChannelData)dwReadSubChannelData;
	auto ret = ReadSubChannelData(this_, buf, stream);
	return ret;
}

