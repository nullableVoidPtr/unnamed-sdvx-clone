#pragma once
#include "nanovg.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "Graphics/Font.hpp"
#include "Graphics/RenderQueue.hpp"
#include "Shared/Transform.hpp"

struct GUIState
{
	NVGcontext* vg;
	RenderQueue* rq;
	Transform t;
	Map<lua_State*, Map<int, Text>> textCache;
	Map<lua_State*, int> nextTextId;
	Map<String, Graphics::Font> fontCahce;
	Graphics::Font* currentFont;
	Vector4 fillColor;
	int textAlign;
	int fontSize;
	Material* fontMaterial;
	Material* fillMaterial;
};


GUIState g_guiState;

static int LoadFont(const char* name, const char* filename)
{
	{
		Graphics::Font* cached = g_guiState.fontCahce.Find(name);
		if (cached)
		{
			g_guiState.currentFont = cached;
		}
		else
		{
			String path = filename;
			Graphics::Font newFont = FontRes::Create(g_gl, path);
			g_guiState.fontCahce.Add(name, newFont);
			g_guiState.currentFont = g_guiState.fontCahce.Find(name);
		}
	}

	{ //nanovg
		if (nvgFindFont(g_guiState.vg, name) != -1)
		{
			nvgFontFace(g_guiState.vg, name);
			return 0;
		}

		nvgFontFaceId(g_guiState.vg, nvgCreateFont(g_guiState.vg, name, filename));
		nvgAddFallbackFont(g_guiState.vg, name, "fallback");
	}
	return 0;
}

static int lBeginPath(lua_State* L)
{
	g_guiState.fillColor = Vector4(1.0);
	nvgBeginPath(g_guiState.vg);
	return 0;
}

static int lText(lua_State* L /*const char* s, float x, float y*/)
{
	const char* s;
	float x, y;
	s = luaL_checkstring(L, 1);
	x = luaL_checknumber(L, 2);
	y = luaL_checknumber(L, 3);
	nvgText(g_guiState.vg, x, y, s, NULL);

	//{ //Fast text
	//	WString text = Utility::Convert	ToWString(s);
	//	Text te = (*g_guiState.currentFont)->CreateText(text, g_guiState.fontSize);
	//	Transform textTransform = g_guiState.t;
	//	textTransform *= Transform::Translation(Vector2(x, y));

	//	//vertical alignment
	//	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_BOTTOM) != 0)
	//	{
	//		textTransform *= Transform::Translation(Vector2(0, -te->size.y));
	//	}
	//	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_MIDDLE) != 0)
	//	{
	//		textTransform *= Transform::Translation(Vector2(0, -te->size.y / 2));
	//	}

	//	//horizontal alignment
	//	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_CENTER) != 0)
	//	{
	//		textTransform *= Transform::Translation(Vector2(-te->size.x / 2, 0));
	//	}
	//	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_RIGHT) != 0)
	//	{
	//		textTransform *= Transform::Translation(Vector2(-te->size.x, 0));
	//	}
	//	MaterialParameterSet params;
	//	params.SetParameter("color", g_guiState.fillColor);
	//	g_guiState.rq->Draw(textTransform, te, g_application->GetFontMaterial(), params);
	//}
	return 0;
}
static int guiText(const char* s, float x, float y)
{
	nvgText(g_guiState.vg, x, y, s, 0);
	return 0;
}

static int lFontFace(lua_State* L /*const char* s*/)
{
	const char* s;
	s = luaL_checkstring(L, 1);
	nvgFontFace(g_guiState.vg, s);
	return 0;
}
static int lFontSize(lua_State* L /*float size*/)
{
	float size = luaL_checknumber(L, 1);
	nvgFontSize(g_guiState.vg, size);
	g_guiState.fontSize = size;
	return 0;
}
static int lFillColor(lua_State* L /*int r, int g, int b, int a = 255*/)
{
	int r, g, b, a;
	r = luaL_checkinteger(L, 1);
	g = luaL_checkinteger(L, 2);
	b = luaL_checkinteger(L, 3);
	if (lua_gettop(L) == 4)
	{
		a = luaL_checkinteger(L, 4);
	}
	else
	{
		a = 255;
	}
	nvgFillColor(g_guiState.vg, nvgRGBA(r, g, b, a));
	g_guiState.fillColor = Vector4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
	return 0;
}
static int lRect(lua_State* L /*float x, float y, float w, float h*/)
{
	float x, y, w, h;
	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);
	w = luaL_checknumber(L, 3);
	h = luaL_checknumber(L, 4);
	nvgRect(g_guiState.vg, x, y, w, h);
	return 0;
}
static int lFill(lua_State* L)
{
	nvgFill(g_guiState.vg);
	return 0;
}
static int lTextAlign(lua_State* L /*int align*/)
{
	nvgTextAlign(g_guiState.vg, luaL_checkinteger(L, 1));
	g_guiState.textAlign = luaL_checkinteger(L, 1);
	return 0;
}
static int lCreateImage(lua_State* L /*const char* filename, int imageflags */)
{
	const char* filename = luaL_checkstring(L, 1);
	int imageflags = luaL_checkinteger(L, 2);
	int handle = nvgCreateImage(g_guiState.vg, filename, imageflags);
	if (handle != 0)
	{
		lua_pushnumber(L, handle);
		return 1;
	}
	return 0;
}
static int lImagePatternFill(lua_State* L /*int image, float alpha*/)
{
	int image = luaL_checkinteger(L, 1);
	float alpha = luaL_checknumber(L, 2);
	int w, h;
	nvgImageSize(g_guiState.vg, image, &w, &h);
	nvgFillPaint(g_guiState.vg, nvgImagePattern(g_guiState.vg, 0, 0, w, h, 0, image, alpha));
	return 0;
}
static int lImageRect(lua_State* L /*float x, float y, float w, float h, int image, float alpha, float angle*/)
{
	float x, y, w, h, alpha, angle;
	int image;
	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);
	w = luaL_checknumber(L, 3);
	h = luaL_checknumber(L, 4);
	image = luaL_checkinteger(L, 5);
	alpha = luaL_checknumber(L, 6);
	angle = luaL_checknumber(L, 7);

	int imgH, imgW;
	nvgImageSize(g_guiState.vg, image, &imgW, &imgH);
	float scaleX, scaleY;
	scaleX = w / imgW;
	scaleY = h / imgH;
	nvgTranslate(g_guiState.vg, x, y);
	nvgRotate(g_guiState.vg, angle);
	nvgScale(g_guiState.vg, scaleX, scaleY);
	nvgFillPaint(g_guiState.vg, nvgImagePattern(g_guiState.vg, 0, 0, imgW, imgH, 0, image, alpha));
	nvgRect(g_guiState.vg, 0, 0, imgW, imgH);
	nvgFill(g_guiState.vg);
	nvgScale(g_guiState.vg, 1.0 / scaleX, 1.0 / scaleY);
	nvgRotate(g_guiState.vg, -angle);
	nvgTranslate(g_guiState.vg, -x, -y);
	return 0;
}
static int lScale(lua_State* L /*float x, float y*/)
{
	float x, y;
	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);
	nvgScale(g_guiState.vg, x, y);
	g_guiState.t *= Transform::Scale({ x, y, 0 });
	return 0;
}
static int lTranslate(lua_State* L /*float x, float y*/)
{
	float x, y;
	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);
	g_guiState.t *= Transform::Translation({ x, y, 0 });
	nvgTranslate(g_guiState.vg, x, y);
	return 0;
}
static int lRotate(lua_State* L /*float angle*/)
{
	float angle = luaL_checknumber(L, 1);
	nvgRotate(g_guiState.vg, angle);
	g_guiState.t *= Transform::Rotation({ 0, 0, angle });
	return 0;
}
static int lResetTransform(lua_State* L)
{
	nvgResetTransform(g_guiState.vg);
	g_guiState.t = Transform();
	return 0;
}
static int lLoadFont(lua_State* L /*const char* name, const char* filename*/)
{
	const char* name = luaL_checkstring(L, 1);
	const char* filename = luaL_checkstring(L, 2);
	LoadFont(name, filename);
	return 0;
}
static int lCreateLabel(lua_State* L /*const char* text, int size, bool monospace*/)
{
	const char* text = luaL_checkstring(L, 1);
	int size = luaL_checkinteger(L, 2);
	int monospace = luaL_checkinteger(L, 3);

	g_guiState.textCache.FindOrAdd(L).Add(g_guiState.nextTextId[L], (*g_guiState.currentFont)->CreateText(Utility::ConvertToWString(text), size, (FontRes::TextOptions)monospace));
	lua_pushnumber(L, g_guiState.nextTextId[L]);
	g_guiState.nextTextId[L]++;
	return 1;
}

static int lUpdateLabel(lua_State* L /*int labelId, const char* text, int size*/)
{
	int labelId = luaL_checkinteger(L, 1);
	const char* text = luaL_checkstring(L, 2);
	int size = luaL_checkinteger(L, 3);
	g_guiState.textCache[L][labelId] = (*g_guiState.currentFont)->CreateText(Utility::ConvertToWString(text), size);
	return 0;
}

static int lDrawLabel(lua_State* L /*int labelId, float x, float y, float maxWidth = -1*/)
{
	int labelId = luaL_checkinteger(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float maxWidth = -1;
	if (lua_gettop(L) == 4)
	{
		maxWidth = luaL_checknumber(L, 4);
	}
	Transform textTransform = g_guiState.t;
	textTransform *= Transform::Translation(Vector2(x, y));
	Text te = g_guiState.textCache[L][labelId];

	if (maxWidth > 0)
	{
		float scale = maxWidth / te->size.x;
		textTransform *= Transform::Scale(Vector2(Math::Min(scale,1.0f)));
	}

	//vertical alignment
	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_BOTTOM) != 0)
	{
		textTransform *= Transform::Translation(Vector2(0, -te->size.y));
	}
	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_MIDDLE) != 0)
	{
		textTransform *= Transform::Translation(Vector2(0, -te->size.y / 2));
	}

	//horizontal alignment
	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_CENTER) != 0)
	{
		textTransform *= Transform::Translation(Vector2(-te->size.x / 2, 0));
	}
	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_RIGHT) != 0)
	{
		textTransform *= Transform::Translation(Vector2(-te->size.x, 0));
	}

	MaterialParameterSet params;
	params.SetParameter("color", g_guiState.fillColor);
	g_guiState.rq->Draw(textTransform, te, *g_guiState.fontMaterial, params);
	return 0;
}

static int lMoveTo(lua_State* L /* float x, float y */)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	nvgMoveTo(g_guiState.vg, x, y);
	return 0;
}

static int lLineTo(lua_State* L /* float x, float y */)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	nvgLineTo(g_guiState.vg, x, y);
	return 0;
}

static int lBezierTo(lua_State* L /* float c1x, float c1y, float c2x, float c2y, float x, float y */)
{
	float c1x = luaL_checknumber(L, 1);
	float c1y = luaL_checknumber(L, 2);
	float c2x = luaL_checknumber(L, 3);
	float c2y = luaL_checknumber(L, 4);
	float x = luaL_checknumber(L, 5);
	float y = luaL_checknumber(L, 6);
	nvgBezierTo(g_guiState.vg, c1x, c1y, c2x, c2y, x, y);
	return 0;
}

static int lQuadTo(lua_State* L /* float cx, float cy, float x, float y */)
{
	float cx = luaL_checknumber(L, 1);
	float cy = luaL_checknumber(L, 2);
	float x = luaL_checknumber(L, 3);
	float y = luaL_checknumber(L, 4);
	nvgQuadTo(g_guiState.vg, cx, cy, x, y);
	return 0;
}

static int lArcTo(lua_State* L /* float x1, float y1, float x2, float y2, float radius */)
{
	float x1 = luaL_checknumber(L, 1);
	float y1 = luaL_checknumber(L, 2);
	float x2 = luaL_checknumber(L, 3);
	float y2 = luaL_checknumber(L, 4);
	float radius = luaL_checknumber(L, 5);
	nvgArcTo(g_guiState.vg, x1, y1, x2, y2, radius);
	return 0;
}

static int lClosePath(lua_State* L)
{
	nvgClosePath(g_guiState.vg);
	return 0;
}

static int lStroke(lua_State* L)
{
	nvgStroke(g_guiState.vg);
	return 0;
}

static int lMiterLimit(lua_State* L /* float limit */)
{
	float limit = luaL_checknumber(L, 1);
	nvgMiterLimit(g_guiState.vg, limit);
	return 0;
}

static int lStrokeWidth(lua_State* L /* float size */)
{
	float size = luaL_checknumber(L, 1);
	nvgStrokeWidth(g_guiState.vg, size);
	return 0;
}

static int lLineCap(lua_State* L /* int cap */)
{
	int cap = luaL_checkinteger(L, 1);
	nvgLineCap(g_guiState.vg, cap);
	return 0;
}

static int lLineJoin(lua_State* L /* int join */)
{
	int join = luaL_checkinteger(L, 1);
	nvgLineJoin(g_guiState.vg, join);
	return 0;
}

static int lStrokeColor(lua_State* L /*int r, int g, int b, int a = 255*/)
{
	int r, g, b, a;
	r = luaL_checkinteger(L, 1);
	g = luaL_checkinteger(L, 2);
	b = luaL_checkinteger(L, 3);
	if (lua_gettop(L) == 4)
	{
		a = luaL_checkinteger(L, 4);
	}
	else
	{
		a = 255;
	}
	nvgStrokeColor(g_guiState.vg, nvgRGBA(r, g, b, a));
	return 0;
}

static int lFastRect(lua_State* L /*float x, float y, float w, float h*/)
{
	float x, y, w, h;
	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);
	w = luaL_checknumber(L, 3);
	h = luaL_checknumber(L, 4);
	Mesh quad = Graphics::MeshGenerators::Quad(g_gl, Vector2(x, y), Vector2(w, h));
	MaterialParameterSet params;
	params.SetParameter("color", g_guiState.fillColor);
	g_guiState.rq->Draw(g_guiState.t, quad, *g_guiState.fillMaterial, params);
	return 0;
}

static int lFastText(lua_State* L /* String utf8string, float x, float y */)
{
	const char* s;
	float x, y;
	s = luaL_checkstring(L, 1);
	x = luaL_checknumber(L, 2);
	y = luaL_checknumber(L, 3);

	WString text = Utility::ConvertToWString(s);
	Text te = (*g_guiState.currentFont)->CreateText(text, g_guiState.fontSize);
	Transform textTransform = g_guiState.t;
	textTransform *= Transform::Translation(Vector2(x, y));

	//vertical alignment
	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_BOTTOM) != 0)
	{
		textTransform *= Transform::Translation(Vector2(0, -te->size.y));
	}
	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_MIDDLE) != 0)
	{
		textTransform *= Transform::Translation(Vector2(0, -te->size.y / 2));
	}

	//horizontal alignment
	if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_CENTER) != 0)
	{
		textTransform *= Transform::Translation(Vector2(-te->size.x / 2, 0));
	}
	else if ((g_guiState.textAlign & (int)NVGalign::NVG_ALIGN_RIGHT) != 0)
	{
		textTransform *= Transform::Translation(Vector2(-te->size.x, 0));
	}
	MaterialParameterSet params;
	params.SetParameter("color", g_guiState.fillColor);
	g_guiState.rq->Draw(textTransform, te, *g_guiState.fontMaterial, params);

	return 0;
}

static int lRoundedRect(lua_State* L /* float x, float y, float w, float h, float r */)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float w = luaL_checknumber(L, 3);
	float h = luaL_checknumber(L, 4);
	float r = luaL_checknumber(L, 5);
	nvgRoundedRect(g_guiState.vg, x, y, w, h, r);
	return 0;
}

static int lRoundedRectVarying(lua_State* L /* float x, float y, float w, float h, float radTopLeft, float radTopRight, float radBottomRight, float radBottomLeft */)
{
	float x = luaL_checknumber(L, 1);
	float y = luaL_checknumber(L, 2);
	float w = luaL_checknumber(L, 3);
	float h = luaL_checknumber(L, 4);
	float radTopLeft = luaL_checknumber(L, 5);
	float radTopRight = luaL_checknumber(L, 6);
	float radBottomRight = luaL_checknumber(L, 7);
	float radBottomLeft = luaL_checknumber(L, 8);
	nvgRoundedRectVarying(g_guiState.vg, x, y, w, h, radTopLeft, radTopRight, radBottomRight, radBottomLeft);
	return 0;
}

static int lEllipse(lua_State* L /* float cx, float cy, float rx, float ry */)
{
	float cx = luaL_checknumber(L, 1);
	float cy = luaL_checknumber(L, 2);
	float rx = luaL_checknumber(L, 3);
	float ry = luaL_checknumber(L, 4);
	nvgEllipse(g_guiState.vg, cx, cy, rx, ry);
	return 0;
}

static int lCircle(lua_State* L /* float cx, float cy, float r */)
{
	float cx = luaL_checknumber(L, 1);
	float cy = luaL_checknumber(L, 2);
	float r = luaL_checknumber(L, 3);
	nvgCircle(g_guiState.vg, cx, cy, r);
	return 0;
}