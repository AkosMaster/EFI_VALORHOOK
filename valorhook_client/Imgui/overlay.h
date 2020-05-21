#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "imconfig.h"
#include "imstb_truetype.h"
#include "imstb_textedit.h"
#include "imstb_rectpack.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

#include "colors.h"

#include <dwmapi.h>
#include <comdef.h> 
#include <d3d9.h>
#include <d3dx9.h>

#include <inttypes.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dwmapi.lib")

#include <string>

std::string string_To_UTF8(const std::string& str);

void DrawStrokeText(int x, int y, RGBA* color, const char* str);
void DrawNewText(int x, int y, RGBA* color, const char* str);
void DrawRect(int x, int y, int w, int h, RGBA* color, int thickness);
void DrawFilledRect(int x, int y, int w, int h, RGBA* color);
void DrawCircleFilled(int x, int y, int radius, RGBA* color, int segments);
void DrawCircle(int x, int y, int radius, RGBA* color, int segments);
void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, RGBA* color, float thickne);
void DrawTriangleFilled(int x1, int y1, int x2, int y2, int x3, int y3, RGBA* color);
void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness);
void DrawCornerBox(int x, int y, int w, int h, int borderPx, RGBA* color);