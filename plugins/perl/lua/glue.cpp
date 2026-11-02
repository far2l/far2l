typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wRepeatCount;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD;

enum EDITOR_EXITCODE {
    EEC_OPEN_ERROR = 0,
    EEC_MODIFIED = 1,
    ...
};

typedef INT_PTR (*FARAPIADVCONTROL)(
    INT_PTR ModuleNumber,
    int Command,
    void *Param1,
    void *Param2
);

------------------------->

static void lua_push_COORD(lua_State* L, const COORD* s) {
    lua_newtable(L);
    lua_pushinteger(L, s->X); lua_setfield(L, -2, "X");
    lua_pushinteger(L, s->Y); lua_setfield(L, -2, "Y");
}

static void lua_push_KEY_EVENT_RECORD(lua_State* L, const KEY_EVENT_RECORD* s) {
    lua_newtable(L);
    lua_pushboolean(L, s->bKeyDown); lua_setfield(L, -2, "bKeyDown");
    lua_pushinteger(L, s->wRepeatCount); lua_setfield(L, -2, "wRepeatCount");
    lua_pushinteger(L, s->wVirtualKeyCode); lua_setfield(L, -2, "wVirtualKeyCode");
    lua_pushinteger(L, s->wVirtualScanCode); lua_setfield(L, -2, "wVirtualScanCode");

    if (s->uChar.UnicodeChar != 0) {
        lua_pushinteger(L, s->uChar.UnicodeChar);
        lua_setfield(L, -2, "UnicodeChar");
    } else {
        lua_pushinteger(L, s->uChar.AsciiChar);
        lua_setfield(L, -2, "AsciiChar");
    }

    lua_pushinteger(L, s->dwControlKeyState);
    lua_setfield(L, -2, "dwControlKeyState");
}


static void register_editor_exitcode(lua_State* L) {
    lua_newtable(L);

    lua_pushinteger(L, EEC_OPEN_ERROR);
    lua_setfield(L, -2, "EEC_OPEN_ERROR");

    lua_pushinteger(L, EEC_MODIFIED);
    lua_setfield(L, -2, "EEC_MODIFIED");

    // etc...

    lua_setglobal(L, "EditorExitCode");
}


static int lua_adv_control(lua_State* L) {
    INT_PTR module = luaL_checkinteger(L, 1);
    int cmd = luaL_checkinteger(L, 2);
    void* p1 = (void*)luaL_optinteger(L, 3, 0);
    void* p2 = (void*)luaL_optinteger(L, 4, 0);

    INT_PTR result = g_AdvControl(module, cmd, p1, p2);

    lua_pushinteger(L, result);
    return 1;
}

-------------------------

static const luaL_Reg editor_funcs[] = {
    {"ask_user", lua_ask_user},
    {"adv_control", lua_adv_control},
    {NULL, NULL}
};

void register_editor_api(lua_State* L) {
    luaL_newlib(L, editor_funcs);
    lua_setglobal(L, "editor");

    register_editor_exitcode(L);
}

