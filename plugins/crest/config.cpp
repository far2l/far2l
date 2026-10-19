#include "crest.h"
#include <utils.h>
#include <KeyFileHelper.h>

#define INI_LOCATION InMyConfig("plugins/crest/config.ini")
#define INI_SECTION  "Settings"

static const char sKeyEnabled[] = ("Enabled");
static const char sKeyColor[] = ("Color");
static const char sKeyCenterColor[] = ("Cursor Color");
static const char sKeyRulerColor[] = ("Ruler Color");
static const char sKeyTempShow[] = ("Activators");
static const char sKeyLockShow[] = ("Lockers");
static const char sKeyFlags[] = ("Flags");

/*
extern "C" {
	void Editor_AddTrueColorForCrest(struct EditorColor* ec_old, struct PluginStartupInfo* info);
	void Editor_AddCenterTrueColorForCrest(struct EditorColor* ec_old, struct PluginStartupInfo* info);
	uint64_t Editor_GetTrueColorForRuler(struct PluginStartupInfo* info, struct EditorInfo* einfo);
}*/

extern "C" {
void RestoreConfig(CROptions *Options)
{
	KeyFileReadSection kfh(INI_LOCATION, INI_SECTION);
	Options->Enabled = !!kfh.GetInt(sKeyEnabled, Options->Enabled);
	Options->Color = kfh.GetInt(sKeyColor, Options->Color);
	Options->CenterColor = kfh.GetInt(sKeyCenterColor, Options->CenterColor);
	Options->RulerColor = kfh.GetInt(sKeyRulerColor, Options->RulerColor);
	Options->TempShow = kfh.GetInt(sKeyTempShow, Options->TempShow);
	Options->LockShow = kfh.GetInt(sKeyLockShow, Options->LockShow);
	Options->Flags = kfh.GetInt(sKeyFlags, Options->Flags);
}

void SaveConfig(const CROptions *Options)
{
	KeyFileHelper kfh(INI_LOCATION);
	kfh.SetInt(INI_SECTION, sKeyEnabled, Options->Enabled);
	kfh.SetInt(INI_SECTION, sKeyColor, Options->Color);
	kfh.SetInt(INI_SECTION, sKeyCenterColor, Options->CenterColor);
	kfh.SetInt(INI_SECTION, sKeyRulerColor, Options->RulerColor);
	kfh.SetInt(INI_SECTION, sKeyTempShow, Options->TempShow);
	kfh.SetInt(INI_SECTION, sKeyLockShow, Options->LockShow);
	kfh.SetInt(INI_SECTION, sKeyFlags, Options->Flags);
}

}

#include "farplug-wide.h"
#include <Colorspace.h>

static FarTrueColor toFarColor(const RGB& rgb) {
	iRGB i = toIRGB(rgb);
	FarTrueColor c {i.r, i.g, i.b, 1};
	return c;
}

static RGB fromFarColor(const FarTrueColor& c) {
	return toRGB(c.R, c.G, c.B);
}

static int assembleColorComponents(int r, int g, int b) {
	return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

static uint64_t assembleColor(RGB& fg, RGB& bg) {
	iRGB ifg = toIRGB(fg);
	iRGB ibg = toIRGB(bg);
	uint64_t color = 0;
	color |= (uint64_t)assembleColorComponents(ifg.r, ifg.g, ifg.b) << 16;
	color |= (uint64_t)assembleColorComponents(ibg.r, ibg.g, ibg.b) << 40;
	return color | FOREGROUND_TRUECOLOR | BACKGROUND_TRUECOLOR;
}

extern "C" __attribute__((visibility("hidden"))) void Editor_AddCenterTrueColorForCrest(EditorColor* ec_old, PluginStartupInfo* info) {
	EditorTrueColor ec;

	// first, we need the color under the position
	static bool counted = false;
	static FarTrueColor crestForeColor;
	static FarTrueColor crestBackColor;

	if (!counted) {
		ec.Base = *ec_old;
		info->EditorControl( ECTL_GETTRUECOLOR, &ec );

		RGB fg = fromFarColor(ec.TrueColor.Fore);
		RGB bg = fromFarColor(ec.TrueColor.Back);
		HoverResult r = ComputeControlAccent(fg, bg);
		RGB newFg;
		ComputeContrast(r.fg_hover, r.bg_hover, newFg);
		crestForeColor = toFarColor(newFg);
		crestBackColor = toFarColor(r.bg_hover);
		counted = true;
	}

	ec.Base = *ec_old;
	ec.TrueColor.Fore = crestForeColor;
	ec.TrueColor.Back = crestBackColor;

	info->EditorControl( ECTL_ADDTRUECOLOR, &ec );
}

extern "C" __attribute__((visibility("hidden"))) void Editor_AddTrueColorForCrest(EditorColor* ec_old, PluginStartupInfo* info) {
	Editor_AddCenterTrueColorForCrest(ec_old, info);
}

extern "C" __attribute__((visibility("hidden"))) uint64_t Editor_GetTrueColorForRuler(PluginStartupInfo* info, EditorInfo* einfo) {

	EditorTrueColor ec;
	ec.Base.StringNumber = 1;
	ec.Base.StartPos = ec.Base.EndPos = 1;
	ec.Base.Color = ec.Base.ColorItem = 0;
	info->EditorControl( ECTL_GETTRUECOLOR, &ec );

	// RGB bg = toRGB(0xAA, 0xFF, 0xFF);
	// RGB fg = toRGB(0xFF, 0x00, 0x00);
	RGB fg = fromFarColor(ec.TrueColor.Fore);
	RGB bg = fromFarColor(ec.TrueColor.Back);

	HoverResult r = ComputeControlAccent(fg, bg);
	r = ComputeHoverColors(r.fg_hover, r.bg_hover);
	r.bg_hover = SoftenToFocusedState_LAB(r.bg_hover);
	
	RGB newFg;
	ComputeContrast(r.fg_hover, r.bg_hover, newFg);

	//bg = toRGB(0xAA, 0xFF, 0xFF);
	//fg = toRGB(0xFF, 0xC0, 0x00);

	return assembleColor(newFg, r.bg_hover);
	// return assembleColor(r.bg_hover, newFg);
	// return assembleColor(bg, fg);
}

