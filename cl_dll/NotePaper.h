#pragma once

#include<VGUI_Panel.h>
#include<VGUI_TablePanel.h>
#include<VGUI_HeaderPanel.h>
#include<VGUI_TextGrid.h>
#include<VGUI_Label.h>
#include<VGUI_TextImage.h>
#include "../game_shared/vgui_listbox.h"
#include "../game_shared/vgui_grid.h"
#include "../game_shared/vgui_defaultinputsignal.h"

#include <ctype.h>

#include <vector>

struct SNotePaperInfo
{
	vgui::Image* pImage;
};

class CNotePaperPanel : public vgui::Panel, public vgui::CDefaultInputSignal
{
public:
	void Initialize();
	void Shutdown();

	void Update();

	void AddTakenNotePaper(const char* pTextureName);

private:
	std::vector<SNotePaperInfo> m_notePapers;

};

