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

=============>

SV* perl_from_COORD(const COORD* s) {
    HV* hv = newHV();
    hv_stores(hv, "X", newSViv(s->X));
    hv_stores(hv, "Y", newSViv(s->Y));
    return newRV_noinc((SV*)hv);
}

SV* perl_from_KEY_EVENT_RECORD(const KEY_EVENT_RECORD* s) {
    HV* hv = newHV();
    hv_stores(hv, "bKeyDown", newSViv(s->bKeyDown));
    hv_stores(hv, "wRepeatCount", newSViv(s->wRepeatCount));
    hv_stores(hv, "wVirtualKeyCode", newSViv(s->wVirtualKeyCode));
    hv_stores(hv, "wVirtualScanCode", newSViv(s->wVirtualScanCode));

    // union handling
    if (s->uChar.UnicodeChar != 0) {
        hv_stores(hv, "UnicodeChar", newSViv(s->uChar.UnicodeChar));
    } else {
        hv_stores(hv, "AsciiChar", newSViv(s->uChar.AsciiChar));
    }

    hv_stores(hv, "dwControlKeyState", newSViv(s->dwControlKeyState));

    return newRV_noinc((SV*)hv);
}

void register_editor_exitcode_constants() {
    HV* stash = gv_stashpv("Editor::Const", TRUE);

    newCONSTSUB(stash, "EEC_OPEN_ERROR", newSViv(EEC_OPEN_ERROR));
    newCONSTSUB(stash, "EEC_MODIFIED", newSViv(EEC_MODIFIED));
    ...
}

XS(Editor_adv_control) {
    dXSARGS;

    if (items != 4)
        XSRETURN_UNDEF;

    INT_PTR module = SvIV(ST(0));
    int cmd = SvIV(ST(1));
    void* p1 = INT2PTR(void*, SvIV(ST(2)));
    void* p2 = INT2PTR(void*, SvIV(ST(3)));

    INT_PTR result = g_AdvControl(module, cmd, p1, p2);

    ST(0) = newSViv(result);
    XSRETURN(1);
}

-----------

use Editor::Const;

print EEC_MODIFIED;

sub on_key_event {
    my ($ev) = @_;
    print "Key: $ev->{wVirtualKeyCode}\n";
}

my $res = Editor::adv_control($module, $cmd, $p1, $p2);

-------------

Possible struct

Editor/
  Const.pm        # enums
  Struct.pm       # blessed hashref constructors
  XS.xs           # glue for functions
  StructGlue.cpp  # struct → hashref converters
  Functions.cpp   # wrappers for function pointers

use Editor::Const;
use Editor::Struct;
use Editor;

sub on_key {
    my ($ev) = @_;
    print "Key: $ev->{wVirtualKeyCode}\n";
}

my $res = Editor::adv_control(0, CMD_SOMETHING, undef, undef);
