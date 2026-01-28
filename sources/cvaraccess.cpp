//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "icvar.h"

extern ICvar* g_pCVar;

class CConVarAccessor : public IConCommandBaseAccessor
{
public:
	virtual bool	RegisterConCommandBase(ConCommandBase* pCommand)
	{
		// Unlink from plugin only list
		pCommand->SetNext(0);

		// Link to engine's list instead
		g_pCVar->RegisterConCommandBase(pCommand);
		return true;
	}

};

CConVarAccessor g_ConVarAccessor;

void InitCVars()
{
	if (g_pCVar)
	{
		ConCommandBaseMgr::OneTimeInit(&g_ConVarAccessor);
	}
}


