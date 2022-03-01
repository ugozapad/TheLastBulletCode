#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "triangleapi.h"
#include "VGUI_TextImage.h"
#include "vgui_loadtga.h"
#include <string.h>
#include <stdio.h>


void CHudHelmet::DrawQuad(float xmin, float ymin, float xmax, float ymax)
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

int CHudHelmet::Init()
{
	
	
	gHUD.AddHudElem(this);
	//m_iFlags |= HUD_ACTIVE;

	
	return 1;
}

int CHudHelmet::VidInit()
{
	

	m_hHelmetSpr = SPR_Load("sprites/640helmet.spr");

	return 1;
}

int CHudHelmet::Draw(float flTime)
{
	/*int r, g, b, a = 255;
	UnpackRGB(r, g, b, RGB_YELLOWISH);
	ScaleColors(r, g, b, a);*/

	

	
	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 1;

	/*if(!m_hHelmetSpr)
		m_hHelmetSpr = gHUD.GetSprite(gHUD.GetSpriteIndex("helmet"));*/
	//gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Brightness(1.0f);
	gEngfuncs.pTriAPI->Color4f(1.0f, 1.0f, 1.0f, 1.0f);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	
	gEngfuncs.pTriAPI->SpriteTexture((struct model_s *)gEngfuncs.GetSpritePointer(m_hHelmetSpr), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	DrawQuad(0, 0, ScreenWidth, ScreenHeight);
	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	
	

	return 1;
}