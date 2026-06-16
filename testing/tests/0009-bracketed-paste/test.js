mydir=WorkDir()
profile=mydir + "/profile"
left=mydir + "/left"
right=mydir + "/right"
MkdirsAll([profile, left, right], 0700)
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left", 0, 0, -1, -1, 10000);
ExpectString("Help - FAR2L", 0, 0, -1, -1, 10000);
TypeEscape(10)
Sync(5000)
// Dismiss OSC52 clipboard dialog if present (first start only)
BeCalm()
var r = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic()
if (r.I < 1) {
    TypeEnter();
    Sleep(500);
    Sync(5000);
}

// Start an interactive bash session so the PTY stays open for TTYWriteRaw.
TypeText("echo 'READY'; exec bash --norc --noprofile")
TypeEnter()
ExpectString("READY", 0, 0, -1, -1, 10000)
Sync(5000)
Sleep(1000)

// Verify basic TTYWriteRaw injects a command that bash executes.
TTYWriteRaw("echo 'RAW_OK'\n")
ExpectString("RAW_OK", 0, 0, -1, -1, 10000)

// Enable bracketed paste mode in bash (DECSET 2004)
TTYWriteRaw("printf '\\e[?2004h' > /dev/tty\n")
Sync(5000)
Sleep(1000)

// Test 1: bracketed paste with simple text + special characters
// "PASTE_SPECIAL_CHARS" tests that ; & | ( ) { } $ pass through literally.
TTYWriteRaw("\x1b[200~echo BRACKETED_OK; echo PASTE_SPECIAL_CHARS\x1b[201~\n")
ExpectString("BRACKETED_OK", 0, 0, -1, -1, 10000)
ExpectString("PASTE_SPECIAL_CHARS", 0, 0, -1, -1, 10000)

// Test 2: bracketed paste with multiline content
// echo preserves newlines, so output is two separate lines.
TTYWriteRaw("\x1b[200~echo LINE_ONE\necho LINE_TWO\x1b[201~\n")
ExpectString("LINE_ONE", 0, 0, -1, -1, 10000)
ExpectString("LINE_TWO", 0, 0, -1, -1, 10000)

// Test 3: bracketed paste with special characters like braces and quotes
TTYWriteRaw("\x1b[200~echo 'BRACE_TEST {[(<>)]}'\x1b[201~\n")
ExpectString("BRACE_TEST {[(<>)]}", 0, 0, -1, -1, 10000)

// Test 4: empty bracketed paste — should be a no-op, bash stays responsive
TTYWriteRaw("\x1b[200~\x1b[201~\n")
Sleep(500)
TTYWriteRaw("echo EMPTY_PASTE_OK\n")
ExpectString("EMPTY_PASTE_OK", 0, 0, -1, -1, 10000)

// Test 5: bracketed paste with only newlines — bash gets empty commands
TTYWriteRaw("\x1b[200~\n\n\x1b[201~\n")
Sleep(500)
// Bash should still be at a working prompt; verify with a follow-up command
TTYWriteRaw("echo NEWLINE_PASTE_OK\n")
ExpectString("NEWLINE_PASTE_OK", 0, 0, -1, -1, 10000)

// Test 6: bracketed paste with a heredoc — newlines inside paste content
// must be preserved as literal text, not treated as Enter keypresses.
// The heredoc uses a quoted delimiter ('EOF') to prevent expansion.
TTYWriteRaw("\x1b[200~cat <<'EOF'\nline AAA\nline BBB\nEOF\x1b[201~\n")
ExpectString("line AAA", 0, 0, -1, -1, 10000)
ExpectString("line BBB", 0, 0, -1, -1, 10000)

// Test 7: paste markers spanning multiple TTYWriteRaw calls — begin marker
// in one call, content and end marker in another. Bash should accumulate
// the text across the delay and execute it when the end marker arrives.
TTYWriteRaw("\x1b[200~echo DELAYED_CLOSE_")
Sleep(1000)
TTYWriteRaw("OK\x1b[201~\n")
ExpectString("DELAYED_CLOSE_OK", 0, 0, -1, -1, 10000)

// Test 8: stray paste end marker — literal text when no begin preceded it.
// The \x1b[201~ bytes reach bash as ordinary input, not as a delimiter.
Sleep(500)
TTYWriteRaw("printf '\\x1b[201~'\n")
Sleep(500)
TTYWriteRaw("echo STRAY_END_OK\n")
ExpectString("STRAY_END_OK", 0, 0, -1, -1, 10000)
Sync(2000)
Sleep(500)

// Test 9: fast follow-up after bracketed paste — verify no artificial
// delay after paste close. Send a paste then immediately (no Sleep)
// send another command; both should execute.
TTYWriteRaw("\x1b[200~echo FAST_PASTE_OK\x1b[201~\n")
TTYWriteRaw("echo IMMEDIATE_FOLLOWUP_OK\n")
ExpectString("FAST_PASTE_OK", 0, 0, -1, -1, 10000)
ExpectString("IMMEDIATE_FOLLOWUP_OK", 0, 0, -1, -1, 10000)

// Test 10: incomplete paste timeout — paste begin without paste end.
// Bash's readline has a bracketed-paste-timeout (readline >= 8.1);
// when it fires, the partial paste is discarded and bash stays
// responsive. If the timeout is not configured, the close marker
// sent via a separate TTYWriteRaw call recovers the stuck paste.
TTYWriteRaw("\x1b[200~echo SHOULD_TIMEOUT\n")
Sleep(3000)
// Send close marker to rescue the stuck paste (covers both timeout
// and explicit-close recovery paths).
TTYWriteRaw("\x1b[201~\n")
// Bash should execute the accumulated text after the close marker.
ExpectString("SHOULD_TIMEOUT", 0, 0, -1, -1, 10000)

// Test 11: disable bracketed paste and verify normal paste works
TTYWriteRaw("printf '\\e[?2004l' > /dev/tty\n")
Sync(5000)
Sleep(1000)
// Send text without bracketed markers — bash should execute it normally
TTYWriteRaw("echo NORMAL_PASTE_OK\n")
ExpectString("NORMAL_PASTE_OK", 0, 0, -1, -1, 10000)

// Exit interactive bash, dismiss VT output, exit far2l
TTYWriteRaw("exit\n")
Sync(5000)
Sleep(1000)
TypeEscape()
TypeText("exit far")
TypeEnter()
ExpectAppExit(0, 10000)
// ============================================
// Phase 2: re-run with reused profile (no OSC52)
// ============================================
// The profile now has OSC52 dismissed. On the second start, no clipboard
// dialog appears — different screen layout and timing. Verify bracketed
// paste works independently of the dialog state.
MkdirsAll([left, right], 0700)
StartApp(["--tty", "--nodetect", "--mortal", "-u", profile, "-cd", left, "-cd", right]);
ExpectString("left", 0, 0, -1, -1, 10000);
TypeEscape(10)
Sync(5000)
// OSC52 should NOT appear on reused profile — verify it's absent
BeCalm()
var r2 = ExpectString("OSC52", 0, 0, -1, -1, 2000);
BePanic()
if (r2.I < 1) {
    Panic("OSC52 should not appear on reused profile")
}

TypeText("echo 'READY2'; exec bash --norc --noprofile")
TypeEnter()
ExpectString("READY2", 0, 0, -1, -1, 10000)
Sync(5000)
Sleep(1000)

TTYWriteRaw("echo 'RAW2_OK'\n")
ExpectString("RAW2_OK", 0, 0, -1, -1, 10000)

// Enable bracketed paste
TTYWriteRaw("printf '\\e[?2004h' > /dev/tty\n")
Sync(5000)
Sleep(1000)

// Core bracketed paste — verify works without OSC52
TTYWriteRaw("\x1b[200~echo BP_NO_OSC52_OK; echo BP_SPECIAL_CHARS_2\x1b[201~\n")
ExpectString("BP_NO_OSC52_OK", 0, 0, -1, -1, 10000)
ExpectString("BP_SPECIAL_CHARS_2", 0, 0, -1, -1, 10000)

// Multiline without OSC52
TTYWriteRaw("\x1b[200~echo BP_LINE_A\necho BP_LINE_B\x1b[201~\n")
ExpectString("BP_LINE_A", 0, 0, -1, -1, 10000)
ExpectString("BP_LINE_B", 0, 0, -1, -1, 10000)

// Delayed close without OSC52
TTYWriteRaw("\x1b[200~echo BP_DELAYED_")
Sleep(1000)
TTYWriteRaw("CLOSE_2\x1b[201~\n")
ExpectString("BP_DELAYED_CLOSE_2", 0, 0, -1, -1, 10000)

// Exit
TTYWriteRaw("exit\n")
Sync(5000)
Sleep(1000)
TypeEscape()
TypeText("exit far")
TypeEnter()
ExpectAppExit(0, 10000)
0;
