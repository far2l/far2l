#include <node.h>
#include <uv.h>

using namespace node;

int main(int argc, char* argv[]) {
    // 1. Initialize Node
    std::vector<std::string> args = { "embedded-node" };
    std::vector<std::string> exec_args;
    std::vector<std::string> errors;

    // Create Node instance
    std::unique_ptr<MultiIsolatePlatform> platform = MultiIsolatePlatform::Create(4);
    V8::InitializePlatform(platform.get());
    V8::Initialize();

    // 2. Create Node environment
    std::unique_ptr<CommonEnvironment> env = CreateEnvironment(
        platform.get(),
        args,
        exec_args
    );

    // 3. Run a JS file
    RunScript(env.get(), "require('./plugin.js')");

    // 4. Run event loop
    uv_run(env->event_loop(), UV_RUN_DEFAULT);

    // 5. Cleanup
    env->Dispose();
    V8::Dispose();
    V8::ShutdownPlatform();

    return 0;
}

#include <napi.h>

Napi::Value AskUser(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    std::string prompt = info[0].As<Napi::String>();

    DialogResult r = editor_show_dialog(utf8_to_wchar(prompt));

    Napi::Object obj = Napi::Object::New(env);
    obj.Set("ok", Napi::Boolean::New(env, r.ok));
    obj.Set("text", Napi::String::New(env, wchar_to_utf8(r.text)));

    return obj;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("askUser", Napi::Function::New(env, AskUser));
    return exports;
}

NODE_API_MODULE(editor, Init)

