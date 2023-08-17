#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "triangleapi.h"
#include "r_studioint.h"
#include "com_model.h"


#include <string.h>
#include <stdio.h>

extern engine_studio_api_t IEngineStudio;

void CHudZoom::DrawQuad(float xmin, float ymin, float xmax, float ymax)
{
	//top left
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(xmin, ymin, 0);
	//bottom left
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(xmin, ymax, 0);
	//bottom right
	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(xmax, ymax, 0);
	//top right
	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3f(xmax, ymin, 0);
}

DECLARE_MESSAGE(m_Zoom, ZoomHud)

int CHudZoom::Init()
{
	m_iHudMode = 0;
	HOOK_MESSAGE(ZoomHud);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);
	return 1;
}

int CHudZoom::VidInit()
{
	m_hScope = SPR_Load("sprites/scope_98k.spr");
	m_iHudMode = 0;
	return 1;
}

int CHudZoom::MsgFunc_ZoomHud(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_iHudMode = READ_BYTE();
	return 1;
}

int CHudZoom::Draw(float flTime)
{
	if (!IEngineStudio.IsHardware() || !m_hScope) return 0;

	if (!m_iHudMode) return 0;//draw scope

	gEngfuncs.pTriAPI->RenderMode(kRenderTransColor);
	gEngfuncs.pTriAPI->Brightness(1.0);
	gEngfuncs.pTriAPI->Color4ub(255, 255, 255, 255);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	/*float left = (ScreenWidth - ScreenHeight) / 2;
	float right = left + ScreenHeight;
	float centerx = ScreenWidth / 2;
	float centery = ScreenHeight / 2;


	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hTop_Left), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(left, 0, centerx, centery);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hTop_Right), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(centerx, 0, right, centery);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hBottom_Right), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(centerx, centery, right, ScreenHeight);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hBottom_Left), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(left, centery, centerx, ScreenHeight);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hBlack), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(0, 0, (left + 1), ScreenHeight);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hBlack), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad((right - 1), 0, ScreenWidth, ScreenHeight);
	gEngfuncs.pTriAPI->End();*/

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hScope), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(0, 0, ScreenWidth / 2, ScreenHeight / 2);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hScope), 1);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(ScreenWidth / 2, 0, ScreenWidth, ScreenHeight / 2);
	gEngfuncs.pTriAPI->End();
	
	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hScope), 2);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(ScreenWidth / 2, ScreenHeight / 2, ScreenWidth , ScreenHeight );
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->SpriteTexture((struct model_s*)gEngfuncs.GetSpritePointer(m_hScope), 3);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(0, ScreenHeight / 2, ScreenWidth / 2, ScreenHeight );
	gEngfuncs.pTriAPI->End();

	return 1;
}