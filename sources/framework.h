#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#ifdef _WIN32
#pragma once
#endif

#include <Windows.h>
#include <iostream>  
#include <random>
#include <inetmessage.h>
#include <inetchannelinfo.h>
#include <inetmsghandler.h>
#include <utlvector.h> 
#include <inetchannel.h>
#include <cdll_int.h>
#include <memory>
#include <string> 
#include <checksum_crc.h>
#include <memalloc.h>
#include <bitbuf.h>
#include <icvar.h>
#include <igameevents.h>
#include <KeyValues.h>
#include <time.h> 
#include <bitvec.h>
#include "dbg.h"
#include "convar.h"
#include "protocol.h"
#include "const.h"
#include "iclientnetworkable.h"

#include "XorStr.h"

#include <GameUI/iGameConsole.h>

#include "detours.h"
#include "sigscan.h"
#include "hash_md5.h"

#include "defs.h"
#include "asciiart.h"

#include "Setti.h"
#include "StrUtils.h"
#include "RevSpoofer.h"
#include "Encryption/CRijndael.h"
#include "Encryption/SHA.h"
#include "RevEmu2013.h"
#include <shlwapi.h>
#include <fstream>

#ifdef DEBUG
void printfdbg(const char* format, ...);
}
#else
#define printfdbg Msg
#endif

#define NETMSG_TYPE_BITS	5

// DWORDs defined in hooks.cpp & main.cpp
extern DWORD dwProcessMessages;
extern DWORD dwPrepareSteamConnectResponse;
extern DWORD dwGetUserMessageName;
extern DWORD dwClientState;
extern DWORD dwDisconnectMessage;
extern DWORD dwCvarSetValue;
extern DWORD dwBuildConVarUpdateMessage;
extern DWORD NetChannel_SendNetMsg;
extern DWORD dwDownloadManager_Queue;
extern DWORD dwFindClientClass;
extern DWORD dwSVC_ServerInfo_ReadFromBuffer;
extern DWORD NC; //INetChannel
extern DWORD NetChannel_SendNetMsg;
extern DWORD dwDispatchUserMessage;
extern DWORD dwDownloadManager_Queue;
extern DWORD dwSendNetMsg;
extern DWORD dwReadSubChannelData;
extern DWORD dwSetStringUserData;


// Interfaces
extern ICvar*			g_pCVar;
extern IVEngineClient*	g_pEngineClient;
class CGameEventManager;
extern CGameEventManager* g_GameEventManager;
extern void*			CUserMessages;

extern bool srcds;
extern bool textmode;

extern ofstream logfile;

// NET MESSAGE HELPERS
#define DECLARE_BASE_MESSAGE( msgtype )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer );	\
		bool			WriteToBuffer( bf_write &buffer );	\
		const char		*ToString() const;					\
		int				GetType() const { return msgtype; } \
		const char		*GetName() const { return #msgtype;}\

#define DECLARE_NET_MESSAGE( name )			\
	DECLARE_BASE_MESSAGE( net_##name );		\
	INetMessageHandler *m_pMessageHandler;	\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

#define DECLARE_CLC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( clc_##name );	\
	IClientMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\


#define DECLARE_SVC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( svc_##name );	\
	IServerMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

typedef int QueryCvarCookie_t;

class CGameEventDescriptor;

typedef enum
{
	eQueryCvarValueStatus_ValueIntact = 0,	// It got the value fine.
	eQueryCvarValueStatus_CvarNotFound = 1,
	eQueryCvarValueStatus_NotACvar = 2,		// There's a ConCommand, but it's not a ConVar.
	eQueryCvarValueStatus_CvarProtected = 3	// The cvar was marked with FCVAR_SERVER_CAN_NOT_QUERY, so the server is not allowed to have its value.
} EQueryCvarValueStatus;

// hooks.cpp functions
bool __fastcall			Hooked_PrepareSteamConnectResponse(DWORD* ecx, void* edx, int keySize, const char* encryptionKey, uint64 unGSSteamID, bool bGSSecure, const netadr_t& adr, bf_write& msg);
void*					GetInterface(const char* dllname, const char* interfacename);
void					Hooked_BuildConVarUpdateMessage(NET_SetConVar* cvarMsg, int flags, bool nonDefault);
static bool				IsSafeFileToDownload(const char* pFilename);
CGameEventDescriptor*	GetEventDescriptor(CGameEventDescriptor* descriptors, int count, const char* name);
bool					ProcessControlMessage(INetChannel* chan, int cmd, bf_read& buf);
INetMessage*			FindMessage(INetChannel* ecx, int type);
const char*				GetEventName(int eventid);
void					getEIP();
void					ReturnCvarValue(INetChannel* pThis, EQueryCvarValueStatus status, QueryCvarCookie_t cookie, const char* cvarname, char* value_to_pass);
bool __fastcall			Hooked_ProcessMessages(INetChannel* pThis, void* edx, bf_read& buf);
void __fastcall			hkWriteListenEventList(CGameEventManager* _this, void* edx, int msg);
void					MD5UpdateString(MD5Context_t* ctx, std::string str);
void					BinaryToReadable(unsigned char* in, size_t inlen, std::string& out);
void					GenerateFriendsName(char* friendsName, size_t uMaxLength);
bool __fastcall			hkSendNetMsg(INetChannel* this_, void* edx, INetMessage& msg, bool bVoice);
bool __fastcall			hkDispatchUserMessage(DWORD* this_, void* edx, int msg_type, bf_read& msg_data);
void __fastcall			hkDownloadManager_Queue(DWORD* this_, void* unk, char* baseURL, char* gamePath);
int __cdecl				hkFindClientClass(char* event_name);
bool __fastcall			hkSVC_ServerInfo_ReadFromBuffer(int this_, void* unk, int buf);
bool __fastcall			hkSetStringUserData(DWORD** this_, void* unk, char* userdata, int stringNumber, int* length);
short __fastcall		hkCvarSetValue(ConVar* this_, void* unk, char* String1);
char __fastcall			hkReadSubChannelData(void* this_, void* edx, bf_read& buf, int stream);

inline void MD5UpdateString(MD5Context_t* ctx, std::string str)
{
	MD5Update(ctx, (unsigned char*)(str.c_str()), str.length());
}

void InitCVars();

#endif
