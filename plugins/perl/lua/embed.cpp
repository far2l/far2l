extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <string>

// --- Your editor-side dialog API (example) ---

struct DialogResult {
    bool ok;
    std::wstring text;
};

DialogResult editor_show_dialog(const std::wstring& prompt) {
    DialogResult r;
    r.ok = true;                 // pretend user pressed OK
    r.text = L"User typed text"; // pretend this came from UI
    return r;
}

// UTF-8 helpers (stubbed)
std::string wchar_to_utf8(const std::wstring& w) {
    return "User typed text"; // replace with real conversion
}
std::wstring utf8_to_wchar(const char* s) {
    return L"Prompt"; // replace with real conversion
}

// --- Lua binding: editor.ask_user(prompt) -> { ok = bool, text = string } ---

static int lua_editor_ask_user(lua_State* L) {
    const char* prompt_utf8 = luaL_checkstring(L, 1);
    std::wstring prompt = utf8_to_wchar(prompt_utf8);

    DialogResult r = editor_show_dialog(prompt);

    lua_newtable(L);
    lua_pushboolean(L, r.ok);
    lua_setfield(L, -2, "ok");

    std::string text_utf8 = wchar_to_utf8(r.text);
    lua_pushlstring(L, text_utf8.c_str(), text_utf8.size());
    lua_setfield(L, -2, "text");

    return 1; // one return value (the table)
}

// --- Register editor API in Lua ---

static const luaL_Reg editor_funcs[] = {
    {"ask_user", lua_editor_ask_user},
    {NULL, NULL}
};

void register_editor_api(lua_State* L) {
    luaL_newlib(L, editor_funcs);
    lua_setglobal(L, "editor");
}

// --- Run a Lua script with an entrypoint main() ---

void run_lua_script(lua_State* L, const char* filename) {
    if (luaL_dofile(L, filename) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        fprintf(stderr, "Lua error: %s\n", err);
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "main");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        fprintf(stderr, "Lua error in main(): %s\n", err);
        lua_pop(L, 1);
    }
}

int main() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    register_editor_api(L);

    run_lua_script(L, "plugin.lua");

    lua_close(L);
    return 0;
}

-------------------------

static int lua_ask_user(lua_State* L) {
    const char* prompt_utf8 = luaL_checkstring(L, 1);
    std::wstring prompt = utf8_to_wchar(prompt_utf8);

    DialogResult r = editor_show_dialog(prompt);

    lua_newtable(L);
    lua_pushboolean(L, r.ok); lua_setfield(L, -2, "ok");
    lua_pushstring(L, wchar_to_utf8(r.text).c_str());
    lua_setfield(L, -2, "text");

    return 1;
}
