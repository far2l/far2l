#include <EXTERN.h>
#include <perl.h>
#include <string>
#include <vector>

static PerlInterpreter* my_perl = nullptr;

// ------------------------------------------------------------
// 1. Your editor’s dialog function (synthetic example)
// ------------------------------------------------------------
struct DialogResult {
    bool ok;
    std::wstring text;
};

DialogResult editor_show_dialog(const std::wstring& prompt) {
    // Replace this with your real dialog code
    DialogResult r;
    r.ok = true; // pretend user pressed OK
    r.text = L"User typed text"; 
    return r;
}

// Helpers to convert between UTF‑8 and wchar_t
std::string wchar_to_utf8(const std::wstring& w) {
    // Your real conversion here
    return std::string("User typed text");
}

std::wstring utf8_to_wchar(const char* s) {
    // Your real conversion here
    return L"User typed text";
}

// ------------------------------------------------------------
// 2. Expose Editor::ask_user to Perl
// ------------------------------------------------------------
XS(Editor_ask_user) {
    dXSARGS;

    if (items != 1)
        XSRETURN_UNDEF;

    // Get UTF‑8 prompt from Perl
    STRLEN len;
    const char* prompt_utf8 = SvPVutf8(ST(0), len);
    std::wstring prompt = utf8_to_wchar(prompt_utf8);

    // Call your editor dialog
    DialogResult r = editor_show_dialog(prompt);

    // Return a Perl hashref: { ok => 1/0, text => "..." }
    HV* hv = newHV();

    hv_stores(hv, "ok",   newSViv(r.ok ? 1 : 0));

    std::string text_utf8 = wchar_to_utf8(r.text);
    SV* sv_text = newSVpv(text_utf8.c_str(), text_utf8.size());
    SvUTF8_on(sv_text);
    hv_stores(hv, "text", sv_text);

    SV* rv = newRV_noinc((SV*)hv);
    ST(0) = rv;
    XSRETURN(1);
}

// ------------------------------------------------------------
// 3. Initialize Perl and load script
// ------------------------------------------------------------
void init_perl(const char* script_path) {
    int argc = 3;
    char* argv[] = { (char*)"", (char*)"-e", (char*)"0", nullptr };
    char** env = nullptr;

    PERL_SYS_INIT3(&argc, &argv, &env);
    my_perl = perl_alloc();
    perl_construct(my_perl);

    perl_parse(my_perl, nullptr, argc, argv, nullptr);

    // Register Editor::ask_user
    newXS("Editor::ask_user", Editor_ask_user, __FILE__);

    // Load user script
    std::string code = "do '" + std::string(script_path) + "';";
    eval_pv(code.c_str(), TRUE);
}

// ------------------------------------------------------------
// 4. Call a Perl function from C++
// ------------------------------------------------------------
void run_perl_entrypoint() {
    eval_pv("main()", TRUE);
}

// ------------------------------------------------------------
int main() {
    init_perl("plugin.pl");
    run_perl_entrypoint();
    return 0;
}

