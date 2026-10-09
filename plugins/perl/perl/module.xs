MODULE = Editor    PACKAGE = Editor

void
print_message(msg)
    char *msg
CODE:
    editor_print(msg);
