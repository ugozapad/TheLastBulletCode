#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"

#include "demo.h"
#include "demo_api.h"
#include "vgui_ScorePanel.h"

DECLARE_MESSAGE(m_Use, ShowUseHud)

int CHudUse::Init(void)
{
	m_bHudMode = false;
	HOOK_MESSAGE(ShowUseHud);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
}

int CHudUse::VidInit(void)
{
	m_hIcon = SPR_Load("sprites/scope_98k.spr");
	m_iFlags = 0;
	return 1;
}

int CHudUse::Draw(float flTime)
{
	if (!m_hIcon) return 0;

	return 0;
}

void CHudUse::DrawQuad(float xmin, float ymin, float xmax, float ymax)
{
}

int CHudUse::MsgFunc_ShowUseHud(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_bHudMode = READ_BYTE();
	return 1;
}
