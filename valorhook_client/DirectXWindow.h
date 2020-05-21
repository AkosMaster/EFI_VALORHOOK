#pragma once
#include "Imgui/overlay.h"

void startOverlayProcess();

namespace ScreenInfo {
	extern int Width;
	extern int Height;
}

// defined in Menu.cpp
void renderMenu();
void renderExtraGui();