#include <napi.h>

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

----------------->

Napi::Object js_from_KEY_EVENT_RECORD(
    Napi::Env env,
    const KEY_EVENT_RECORD* s
) {
    Napi::Object obj = Napi::Object::New(env);

    obj.Set("bKeyDown", s->bKeyDown);
    obj.Set("wRepeatCount", s->wRepeatCount);
    obj.Set("wVirtualKeyCode", s->wVirtualKeyCode);
    obj.Set("wVirtualScanCode", s->wVirtualScanCode);

    if (s->uChar.UnicodeChar != 0)
        obj.Set("UnicodeChar", s->uChar.UnicodeChar);
    else
        obj.Set("AsciiChar", s->uChar.AsciiChar);

    obj.Set("dwControlKeyState", s->dwControlKeyState);

    return obj;
}

-----------------

exports.Set("EEC_MODIFIED", Napi::Number::New(env, EEC_MODIFIED));

-----------------

Napi::Value AdvControl(const Napi::CallbackInfo& info) {
    INT_PTR module = info[0].As<Napi::Number>();
    int cmd = info[1].As<Napi::Number>();
    void* p1 = reinterpret_cast<void*>(info[2].As<Napi::Number>().Int64Value());
    void* p2 = reinterpret_cast<void*>(info[3].As<Napi::Number>().Int64Value());

    INT_PTR result = g_AdvControl(module, cmd, p1, p2);

    return Napi::Number::New(info.Env(), result);
}


