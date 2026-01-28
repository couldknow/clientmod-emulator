
#define _CRT_SECURE_NO_WARNINGS 
#define DISCMSG
//#define TIMEDACCESS 

#include "framework.h"
#include <iomanip>

using namespace std;

bool srcds = false;
bool textmode = false;

#ifdef DEBUG
void printfdbg(const char* format, ...)
{
	va_list arglist;
	auto time = std::time(nullptr);
	std::cout << std::put_time(std::localtime(&time), "[%H:%M:%S] ");
	logfile << std::put_time(std::localtime(&time), "[%H:%M:%S] ");
	va_start(arglist, format);
	//vprintf(format, arglist); 
	char logstring[1024];
	vsprintf(logstring, format, arglist);
	va_end(arglist);
	Msg(logstring);
	logfile << logstring;
}
std::default_random_engine generator(time)
{
	return std::default_random_engine();
}
#endif

HMODULE hMod = 0;
ICvar* g_pCVar = nullptr;
void* CUserMessages = nullptr;

DWORD dwProcessMessages;
DWORD dwPrepareSteamConnectResponse;
DWORD dwGetUserMessageName;
DWORD dwClientState = 0;
DWORD dwDisconnectMessage = 0;

IVEngineClient* g_pEngineClient = 0;
CGameEventManager* g_GameEventManager;

void* GetInterface(const char* dllname, const char* interfacename)
{
	CreateInterfaceFn CreateInterface = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA(dllname), "CreateInterface");
	int returnCode = 0;
	void* ointerface = CreateInterface(interfacename, &returnCode);
	printfdbg("Interface %s/%s = %x\n", dllname, interfacename, ointerface);
	return ointerface;
}

DWORD WINAPI MainThread(HMODULE hModule)
{
	TCHAR szExeFileName[MAX_PATH];
	GetModuleFileName(NULL, szExeFileName, MAX_PATH);
	string path = string(szExeFileName);
	string exe = path.substr(path.find_last_of("\\") + 1, path.size());

	srcds = !strcmp(exe.c_str(), XorStr("srcds.exe"));
	printfdbg("srcds.exe? %d\n", srcds);

	char client_dll[] = "client.dll";
	if (srcds) strcpy(client_dll, "server.dll");

#ifdef TIMEDACCESS
	printfdbg("compile time %d\n", compiletime);
	curtime = gTime();
	printfdbg("current time %d\n", curtime);
	timer = compiletime + duration - curtime;
	mStartedTime = chrono::system_clock::now();
#endif

	printfdbg(XorStr("ClientMod 3 Emulator\nOriginal code: InFro, Spy, updated by RuSHeRR\nCredits to cssandroid & atryrkakiv\n"));
	printfdbg("Compile time %s\n", __TIMESTAMP__);
	printfdbg("Project's github: https://github.com/rusherr-c/clientmod-emulator\n");

	SigScan scan;

	g_GameEventManager = (CGameEventManager*)GetInterface("engine.dll", INTERFACEVERSION_GAMEEVENTSMANAGER2);

	g_pEngineClient = (IVEngineClient*)GetInterface("engine.dll", VENGINE_CLIENT_INTERFACE_VERSION);

	if (!srcds) {
		DWORD dwEngine = (DWORD)GetModuleHandleA("engine.dll");

		IGameConsole* g_pGameConsole = (IGameConsole*)GetInterface(XorStr("gameui.dll"), XorStr(GAMECONSOLE_INTERFACE_VERSION));
		Color clr1 = Color(115, 0, 255, 255);
		Color clr2 = Color(166, 0, 255, 255);
		g_pGameConsole->ColorPrintf(clr1, clientmod_ascii::a1.c_str());
		g_pGameConsole->ColorPrintf(clr2, clientmod_ascii::a2.c_str());
		g_pGameConsole->ColorPrintf(clr1, clientmod_ascii::a3.c_str());
		g_pGameConsole->ColorPrintf(clr2, clientmod_ascii::a4.c_str());
		g_pGameConsole->ColorPrintf(clr1, clientmod_ascii::a5.c_str());
		g_pGameConsole->ColorPrintf(clr1, "ClientMod 3.0 Emulator\nOriginal code: ");
		g_pGameConsole->ColorPrintf(clr2, "InFro, Spy");
		g_pGameConsole->ColorPrintf(clr1, ", updated by ");
		g_pGameConsole->ColorPrintf(clr2, "RuSHeRR\n");
		g_pGameConsole->ColorPrintf(clr1, "Credits to ");
		g_pGameConsole->ColorPrintf(clr2, "cssandroid ");
		g_pGameConsole->ColorPrintf(clr1, "and ");
		g_pGameConsole->ColorPrintf(clr2, "atryrkakiv\n");
		g_pGameConsole->ColorPrintf(clr1, "Compile time: ");
		g_pGameConsole->ColorPrintf(clr2, __TIMESTAMP__);
		g_pGameConsole->ColorPrintf(clr1, "\nProject's github: ");
		g_pGameConsole->ColorPrintf(clr2, "https://github.com/rusherr-c/clientmod-emulator\n");

		g_pCVar = ((ICvar * (*)(void))GetProcAddress(GetModuleHandleA("vstdlib.dll"), "GetCVarIF"))();
		printfdbg("g_pCVar %x\n", g_pCVar);

		g_pEngineClient->ExecuteClientCmd(
			"setinfo cm_steamid 1337; setinfo cm_steamid_random 1; setinfo cm_steamid_enabled 1; setinfo cm_enabled 1; setinfo cm_version \"3.0.0.9135\"; setinfo cm_drawspray 1; setinfo cm_forcemap \"\"; setinfo cm_fakeconnect 0; setinfo cm_log 0");

		//FCVAR_PROTECTED flag
		g_pCVar->FindVar("cm_steamid")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_steamid_random")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_steamid_enabled")->m_nFlags = 537001984; 
		g_pCVar->FindVar("cm_version")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_enabled")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_drawspray")->m_nFlags = 537001984;
		g_pCVar->FindVar("sv_cheats")->m_nFlags = 0;
		g_pCVar->FindVar("cl_downloadfilter")->m_pszHelpString = "Determines which files can be downloaded from the server(all, none, nosounds, mapsonly)";
		g_pCVar->FindVar("cm_forcemap")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_fakeconnect")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_log")->m_nFlags = 537001984;
		g_pCVar->FindVar("cm_fakeconnect")->m_pszHelpString = "Drop connection at signon state: (1 = CONNECTED, 2 = NEW, 3 = PRESPAWN, 4 = SPAWN, 5 = FULL).";

		dwCvarSetValue = scan.FindPattern(XorStr("engine.dll"), XorStr("\x83\xec\xae\xa1\xae\xae\xae\xae\x33\xc4\x56\x89\x44\x24"), XorStr("xx?x????xxxxxx"));

		dwPrepareSteamConnectResponse = scan.FindPattern(XorStr("engine.dll"), XorStr("\x81\xEC\x00\x00\x00\x00\x56\x8B\xF1\x8B\x0D\x00\x00\x00\x00\x8B\x01\xFF\x50\x24"), XorStr("xx????xxxxx????xxxxx")); //engine.dll+5D50
		dwBuildConVarUpdateMessage = scan.FindPattern(XorStr("engine.dll"), XorStr("\xE8\x00\x00\x00\x00\x8D\x54\x24\x3C"), XorStr("x????xxxx"));
		dwBuildConVarUpdateMessage += 0x9719;

		NetChannel_SendNetMsg = scan.FindPattern(XorStr("engine.dll"), XorStr("\x56\x8b\xf1\x8d\x4e\xae\xe8\xae\xae\xae\xae\x85\xc0\x75"), XorStr("xxxxx?x????xxx"));
		printfdbg("NetChannel_SendNetMsg %x\n", NetChannel_SendNetMsg);

		///*
		auto CBaseClientState_ProcessGetCvarValue = scan.FindPattern(XorStr("engine.dll"), XorStr("\xff\x92\xae\xae\xae\xae\x83\xc8\xae\x89\x84\x24\xae\xae\xae\xae\xc7\x44\x24\xae\xae\xae\xae\xae\x89\x84\x24\xae\xae\xae\xae\x8b\x8c\x24"),
			XorStr("xx????xx?xxx????xxx?????xxx????xxx"));
		printfdbg("CBaseClientState_ProcessGetCvarValue %x\n", CBaseClientState_ProcessGetCvarValue);
		if (CBaseClientState_ProcessGetCvarValue) {
			DWORD PGCVdelta = NetChannel_SendNetMsg - CBaseClientState_ProcessGetCvarValue - 5;
			uint8 PGCVpatch[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90 };
			memcpy(&PGCVpatch[1], &PGCVdelta, 4);
			DWORD oldProtect;
			VirtualProtect((PVOID)(CBaseClientState_ProcessGetCvarValue), sizeof(PGCVpatch), PAGE_EXECUTE_READWRITE, &oldProtect);
			memcpy((PVOID)CBaseClientState_ProcessGetCvarValue, PGCVpatch, sizeof(PGCVpatch));
		}
		//*/

		dwDownloadManager_Queue = scan.FindPattern(XorStr("engine.dll"),
			XorStr("\x6a\xae\x68\xae\xae\xae\xae\x64\xa1\xae\xae\xae\xae\x50\x64\x89\x25\xae\xae\xae\xae\x83\xec\xae\x53\x8b\x5c\x24\xae\x85\xdb"),
			XorStr("x?x????xx????xxxx????xx?xxxx?xx"));
		printfdbg("dwDownloadManager_Queue %x\n", dwDownloadManager_Queue);

		dwClientState = scan.FindPattern(XorStr("engine.dll"),
			XorStr("\x68\xae\xae\xae\xae\x6a\xae\xe8\xae\xae\xae\xae\x83\xc4\xae\x8b\x0d"),
			XorStr("x????x?x????xx?xx")) + 1;
		dwClientState = *(DWORD*)dwClientState + 4;
		printfdbg("dwClientState %x\n", dwClientState);

		dwFindClientClass = scan.FindPattern(XorStr("engine.dll"),
			XorStr("\x56\x57\xe8\xae\xae\xae\xae\x8b\xf0\x85\xf6\x74"),
			XorStr("xxx????xxxxx"));
		printfdbg("dwFindClientClass %x\n", dwFindClientClass);

		//MapPatch 
		DWORD oldProtect;
		DWORD BadInlineModel = scan.FindPattern(XorStr("engine.dll"), XorStr("\x7c\xae\x83\x7e\xae\xae\x75\xae\x33\xc0"), XorStr("x?xx??x?xx"));
		if (BadInlineModel) {
			VirtualProtect((PVOID)(BadInlineModel), 0x100, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(WORD*)BadInlineModel = 0x9090; *(BYTE*)(BadInlineModel + 0x11) = 0xEB;
		}
		DWORD MapVersionExpecting = scan.FindPattern(XorStr("engine.dll"), XorStr("\x7c\xae\x83\xf8\xae\x7e\xae\x6a"), XorStr("x?xx?x?x"));
		if (MapVersionExpecting) {
			VirtualProtect((PVOID)(MapVersionExpecting), 0x100, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(WORD*)MapVersionExpecting = 0x9090; *(BYTE*)(MapVersionExpecting + 0x5) = 0xEB;
		}
		DWORD MapCheckCRC = scan.FindPattern(XorStr("engine.dll"), XorStr("\x74\xae\x8b\x0d\xae\xae\xae\xae\x8b\x11\xff\x52\xae\x84\xc0\x75\xae\x56"), XorStr("x?xx????xxxx?xxx?x"));
		if (MapCheckCRC) {
			VirtualProtect((PVOID)(MapCheckCRC), 0x100, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(BYTE*)(MapCheckCRC) = 0xEB;
		}
	}

	dwProcessMessages = scan.FindPattern(XorStr("engine.dll"), XorStr("\x83\xEC\x2C\x53\x55\x89\x4C\x24\x10"), XorStr("xxxxxxxxx"));
	printfdbg("dwPrepareSteamConnectResponse %x\n", dwPrepareSteamConnectResponse);

	dwReadSubChannelData = scan.FindPattern(XorStr("engine.dll"), XorStr("\x83\xec\xae\x8b\x44\x24\xae\x53\x8d\x14\x80"), XorStr("xx?xxx?xxxx"));
	printfdbg("dwReadSubChannelData %x\n", dwReadSubChannelData);

	DWORD dwWriteListenEventList;
	if (!srcds) {
		printfdbg("dwBuildConVarUpdateMessage %x\n", dwBuildConVarUpdateMessage);
		printfdbg("dwProcessMessages %x\n", dwProcessMessages);

		NC = scan.FindPattern(XorStr("engine.dll"), XorStr("\x00\xc7\x44\x24\x08\x0\x0\x0\x0\xc7\x84\x24"), XorStr("xxxxx????xxx")) + 5;
		NC = (DWORD) * (PVOID*)NC;
		printfdbg("NC %x\n", NC);

		dwWriteListenEventList = scan.FindPattern(XorStr("engine.dll"), XorStr("\x51\x8b\x44\x24\x08\x83\xc0\x10"), XorStr("xxxxxxxx")); //dwEngine + 0xADA80; 
		printfdbg("dwWriteListenEventList %x\n", dwWriteListenEventList);
	}

	CUserMessages = reinterpret_cast<LPVOID>(*(PVOID*)(scan.FindPattern(XorStr(client_dll), XorStr("\x8b\x0d\xae\xae\xae\xae\x6a\xae\x68\xae\xae\xae\xae\xe8"), XorStr("xx????x?x????x")) + 2));
	printfdbg("CUserMessages_ %x\n", CUserMessages);
	dwDispatchUserMessage = scan.FindPattern(XorStr(client_dll), XorStr("\x8b\x44\x24\xae\x83\xec\xae\x85\xc0\x0f\x8c"), XorStr("xxx?xx?xxxx"));
	printfdbg("dwDispatchUserMessage %x\n", dwDispatchUserMessage);

	dwGetUserMessageName = scan.FindPattern(XorStr(client_dll),
		XorStr("\x56\x8b\x74\x24\xae\x85\xf6\x57\x8b\xf9\x7c\xae\x3b\x77\xae\x7c\xae\x56\x68\xae\xae\xae\xae\xff\x15\xae\xae\xae\xae\x83\xc4\xae\x8b\x4f\xae\x8d\x04\x76\x8b\x44\xc1"),
		XorStr("xxxx?xxxxxx?xx?x?xx????xx????xx?xx?xxxxxx"));
	printfdbg("dwGetUserMessageName %x\n", dwGetUserMessageName);

	dwSendNetMsg = scan.FindPattern(XorStr("engine.dll"), XorStr("\xcc\x56\x8b\xf1\x8d\x4e\x74"), XorStr("xxxxxxx")) + 1; //dwEngine + 0xff950;
	printfdbg("dwSendNetMsg %x\n", dwSendNetMsg);

	dwSVC_ServerInfo_ReadFromBuffer = scan.FindPattern(XorStr("engine.dll"), XorStr("\x83\xec\xae\x53\x55\x56\x8b\x74\x24\xae\x57\x8b\xf9\x8d\x87"),
		XorStr("xx?xxxxxx?xxxxx"));
	printfdbg("dwSVC_ServerInfo_ReadFromBuffer %x\n", dwSVC_ServerInfo_ReadFromBuffer);

	dwSetStringUserData = scan.FindPattern(XorStr("engine.dll"), XorStr("\x56\x57\x8b\x7c\x24\xae\x85\xff\x8b\xf1\x75\xae\x8b\x06"), XorStr("xxxxx?xxxxx?xx"));
	printfdbg("dwSetStringUserData %x\n", dwSetStringUserData);

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	if (!srcds) {
		DetourAttach(&(LPVOID&)dwPrepareSteamConnectResponse, &Hooked_PrepareSteamConnectResponse);
		DetourAttach(&(LPVOID&)dwBuildConVarUpdateMessage, &Hooked_BuildConVarUpdateMessage);
		DetourAttach(&(LPVOID&)(dwWriteListenEventList), (PBYTE)hkWriteListenEventList);
		DetourAttach(&(LPVOID&)(dwDispatchUserMessage), (PBYTE)hkDispatchUserMessage);
		DetourAttach(&(LPVOID&)(dwDownloadManager_Queue), (PBYTE)hkDownloadManager_Queue);
		DetourAttach(&(LPVOID&)(dwFindClientClass), (PBYTE)hkFindClientClass);
		DetourAttach(&(LPVOID&)(dwSVC_ServerInfo_ReadFromBuffer), (PBYTE)hkSVC_ServerInfo_ReadFromBuffer);
		DetourAttach(&(LPVOID&)(dwSetStringUserData), (PBYTE)hkSetStringUserData);

		DetourAttach(&(LPVOID&)(dwCvarSetValue), (PBYTE)hkCvarSetValue);
	}

	DetourAttach(&(LPVOID&)dwProcessMessages, &Hooked_ProcessMessages);
	DetourAttach(&(LPVOID&)(dwSendNetMsg), (PBYTE)hkSendNetMsg);
	//DetourAttach(&(LPVOID&)(dwReadSubChannelData), (PBYTE)hkReadSubChannelData);

	DetourTransactionCommit();

	InitCVars();

#ifdef DISCMSG
	DWORD oldDscmsg;
	if (!srcds) {
		dwDisconnectMessage = scan.FindPattern(XorStr("engine.dll"), XorStr("\x74\x14\x8b\x01\x68\x0\x0\x0\x0\xff\x90"), XorStr("xxxxx????xx")); //dwEngine + 0x61cc; 
		printfdbg("dwDisconnectMessage %x\n", dwDisconnectMessage);
		if (dwDisconnectMessage) {
			char* dscmsg = "Disconnect by ClientMod\0";

			DWORD oldProtect;
			VirtualProtect((PVOID)(dwDisconnectMessage), 0x10, PAGE_EXECUTE_READWRITE, &oldProtect);
			dwDisconnectMessage += 5;
			memcpy(&oldDscmsg, (PVOID)(dwDisconnectMessage), 4);
			memcpy((PVOID)(dwDisconnectMessage), &dscmsg, 4); // CBaseClientState::Disconnect
		}
	}
#endif

	while (true)
	{
		if (!srcds && GetAsyncKeyState(VK_DELETE))
			break;

		if (srcds && GetAsyncKeyState(VK_END))
			break;

#ifdef TIMEDACCESS
		if (!CheckTime())
		{
			printfdbg(XorStr("Error: Time expired\n"));
			MessageBoxA_(XorStr("Error"), XorStr("Time expired!"));
			break;
		}
#endif

		Sleep(100);
	}

	printfdbg("Unhooking...\n");

#ifdef DISCMSG
	if (!srcds)
		if (dwDisconnectMessage)
			memcpy((PVOID)(dwDisconnectMessage), &oldDscmsg, 4);
#endif

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	if (!srcds) {
		DetourDetach(&(LPVOID&)dwPrepareSteamConnectResponse, reinterpret_cast<BYTE*>(Hooked_PrepareSteamConnectResponse));
		DetourDetach(&(LPVOID&)dwBuildConVarUpdateMessage, reinterpret_cast<BYTE*>(Hooked_BuildConVarUpdateMessage));
		DetourDetach(&(LPVOID&)dwWriteListenEventList, reinterpret_cast<BYTE*>(hkWriteListenEventList));
		DetourDetach(&(LPVOID&)(dwDispatchUserMessage), reinterpret_cast<BYTE*>(hkDispatchUserMessage));
		DetourDetach(&(LPVOID&)(dwDownloadManager_Queue), reinterpret_cast<BYTE*>(hkDownloadManager_Queue));
		DetourDetach(&(LPVOID&)(dwFindClientClass), reinterpret_cast<BYTE*>(hkFindClientClass));
		DetourDetach(&(LPVOID&)(dwSVC_ServerInfo_ReadFromBuffer), reinterpret_cast<BYTE*>(hkSVC_ServerInfo_ReadFromBuffer));
		DetourDetach(&(LPVOID&)(dwSetStringUserData), reinterpret_cast<BYTE*>(hkSetStringUserData));

		DetourDetach(&(LPVOID&)(dwCvarSetValue), reinterpret_cast<BYTE*>(hkCvarSetValue));
	}
	DetourDetach(&(LPVOID&)dwProcessMessages, reinterpret_cast<BYTE*>(Hooked_ProcessMessages));
	DetourDetach(&(LPVOID&)(dwSendNetMsg), reinterpret_cast<BYTE*>(hkSendNetMsg));
	//DetourDetach(&(LPVOID&)(dwReadSubChannelData), reinterpret_cast<BYTE*>(hkReadSubChannelData)); 

	DetourTransactionCommit();

	if (!srcds) {
#ifdef DEBUG
		if (f) fclose(f);
		FreeConsole();
#endif
	}
	FreeLibraryAndExitThread(hModule, 0);

	return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  dwReason,
	LPVOID lpReserved
)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		hMod = hModule;
		HANDLE hdl = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
		if (hdl) CloseHandle(hdl);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

