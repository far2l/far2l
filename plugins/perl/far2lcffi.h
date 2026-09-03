#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic pop
       
#include <stdint.h>
#include <stddef.h>

typedef uint32_t ULONG;
typedef unsigned int UINT;
typedef unsigned short USHORT;
typedef int LONG;
typedef int INT, *PINT, *LPINT;
typedef short SHORT;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef void VOID;
typedef size_t SIZE_T;
typedef char CCHAR;
typedef short CSHORT;
typedef ULONG CLONG;
typedef CCHAR *PCCHAR;
typedef CSHORT *PCSHORT;
typedef CLONG *PCLONG;
typedef ULONG DWORD;
typedef USHORT WORD;
typedef unsigned long long DWORD64, *PDWORD64;
typedef int64_t INT64, *PINT64;
typedef uint64_t UINT64, *PUINT64;
typedef int64_t LONG64, *PLONG64;
typedef uint64_t ULONG64, *PULONG64;
typedef INT64 INT_PTR;
typedef UINT64 UINT_PTR;
typedef DWORD64 DWORD_PTR;
typedef LONG64 LONG_PTR;
typedef ULONG64 ULONG_PTR;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef char *PSTR;
typedef const wchar_t *LPCWSTR;
typedef wchar_t *LPWSTR;
typedef wchar_t *PWSTR;
typedef wchar_t WCHAR;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef BYTE *PBYTE;
typedef BYTE *LPBYTE;
typedef DWORD *LPDWORD;
typedef DWORD *PDWORD;
typedef ULONG *PULONG;
typedef LONG *PLONG;
typedef WORD *LPWORD;
typedef WORD *PWORD;
typedef USHORT *PUSHORT;
typedef SHORT *PSHORT;
typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;
typedef WCHAR TCHAR;
typedef const TCHAR *LPCTSTR;
typedef WCHAR OLECHAR;
typedef OLECHAR *BSTR;
typedef const OLECHAR *LPCOLESTR;
typedef OLECHAR *LPOLESTR;
typedef int BOOL;
typedef UCHAR BOOLEAN;
typedef BOOL *LPBOOL, *PBOOL;
typedef void *HANDLE;
typedef void *PVOID;
typedef void *LPVOID;
typedef const void *LPCVOID;
typedef ULONG LCID;
typedef PULONG PLCID;
typedef USHORT LANGID;
typedef HANDLE HKEY;
typedef struct _OVERLAPPED *LPOVERLAPPED;
typedef HKEY *PHKEY;
typedef DWORD ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;
typedef ACCESS_MASK REGSAM;
typedef int HRESULT;
typedef ULONG PROPID;
typedef LONG SCODE;
typedef int EXECUTION_STATE;
typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD64 nFileSize;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;
typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    struct {
        DWORD LowPart;
        LONG HighPart;
    };
    LONGLONG QuadPart;
} LARGE_INTEGER;
typedef LARGE_INTEGER *PLARGE_INTEGER;
typedef union _ULARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    ULONGLONG QuadPart;
} ULARGE_INTEGER;
typedef ULARGE_INTEGER *PULARGE_INTEGER;
typedef struct _SECURITY_ATTRIBUTES *LPSECURITY_ATTRIBUTES;
typedef ULONGLONG DWORDLONG, *PDWORDLONG;
typedef struct _MEMORYSTATUSEX {
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORDLONG ullTotalPhys;
  DWORDLONG ullAvailPhys;
  DWORDLONG ullTotalPageFile;
  DWORDLONG ullAvailPageFile;
  DWORDLONG ullTotalVirtual;
  DWORDLONG ullAvailVirtual;
  DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX, *LPMEMORYSTATUSEX;
typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
typedef struct _WIN32_FIND_DATAA {
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    uint32_t UnixOwner;
    uint32_t UnixGroup;
    DWORD64 UnixDevice;
    DWORD64 UnixNode;
    DWORD64 nPhysicalSize;
    DWORD64 nFileSize;
    DWORD dwFileAttributes;
    DWORD dwUnixMode;
    DWORD nHardLinks;
    CHAR cFileName[ 255 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef struct _WIN32_FIND_DATAW {
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    uint32_t UnixOwner;
    uint32_t UnixGroup;
    DWORD64 UnixDevice;
    DWORD64 UnixNode;
    DWORD64 nPhysicalSize;
    DWORD64 nFileSize;
    DWORD dwFileAttributes;
    DWORD dwUnixMode;
    DWORD nHardLinks;
    DWORD nBlockSize;
    WCHAR cFileName[ 255 ];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW, *LPWIN32_FIND_DATAW, WIN32_FIND_DATA, *PWIN32_FIND_DATA, *LPWIN32_FIND_DATA;
typedef struct _BY_HANDLE_FILE_INFORMATION {
  DWORD dwFileAttributes;
  FILETIME ftCreationTime;
  FILETIME ftLastAccessTime;
  FILETIME ftLastWriteTime;
  DWORD dwVolumeSerialNumber;
  DWORD64 nFileSize;
  DWORD nNumberOfLinks;
  DWORD64 nFileIndex;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;
typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;
typedef struct _CONSOLE_FONT_INFO {
    DWORD nFont;
    COORD dwFontSize;
} CONSOLE_FONT_INFO, *PCONSOLE_FONT_INFO;
typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT, *PSMALL_RECT;
typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    DWORD64 wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO, *PCONSOLE_SCREEN_BUFFER_INFO;
typedef struct _CONSOLE_CURSOR_INFO {
    DWORD dwSize;
    BOOL bVisible;
} CONSOLE_CURSOR_INFO, *PCONSOLE_CURSOR_INFO;
typedef DWORD64 COMP_CHAR;
typedef struct _CHAR_INFO {
    union {
        COMP_CHAR UnicodeChar;
        CHAR AsciiChar;
    } Char;
    DWORD64 Attributes;
} CHAR_INFO, *PCHAR_INFO;
typedef struct _WINDOW_BUFFER_SIZE_RECORD {
    COORD dwSize;
    BOOL bDamaged;
} WINDOW_BUFFER_SIZE_RECORD, *PWINDOW_BUFFER_SIZE_RECORD;
typedef struct _MENU_EVENT_RECORD {
    UINT dwCommandId;
} MENU_EVENT_RECORD, *PMENU_EVENT_RECORD;
typedef struct _FOCUS_EVENT_RECORD {
    BOOL bSetFocus;
} FOCUS_EVENT_RECORD, *PFOCUS_EVENT_RECORD;
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
} KEY_EVENT_RECORD, *PKEY_EVENT_RECORD;
typedef struct _CALLBACK_EVENT_RECORD {
    VOID (*Function)(VOID *Context);
    VOID *Context;
} CALLBACK_EVENT_RECORD, *PCALLBACK_EVENT_RECORD;
typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD, *PMOUSE_EVENT_RECORD;
typedef struct _BRACKETED_PASTE {
    BOOL bStartPaste;
} BRACKETED_PASTE, *PBRACKETED_PASTE;
typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        MENU_EVENT_RECORD MenuEvent;
        FOCUS_EVENT_RECORD FocusEvent;
        BRACKETED_PASTE BracketedPaste;
        CALLBACK_EVENT_RECORD CallbackEvent;
    } Event;
} INPUT_RECORD, *PINPUT_RECORD;
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;
typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t Data4[ 8 ];
} GUID, IID;
typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;
typedef void *HMODULE;
typedef void *HKL;
typedef struct _cpinfo {
    UINT MaxCharSize;
    BYTE DefaultChar[4];
    BYTE LeadByte[12];
} CPINFO, *LPCPINFO;
typedef struct _cpinfoex {
    UINT MaxCharSize;
    BYTE DefaultChar[4];
    BYTE LeadByte[12];
    WCHAR UnicodeDefaultChar;
    UINT CodePage;
    TCHAR CodePageName[255];
} CPINFOEX, *LPCPINFOEX;
typedef BOOL (*CODEPAGE_ENUMPROCW)(LPWSTR);
typedef LONG NTSTATUS;
typedef struct WinportGraphicsInfo1
{
    DWORD64 Caps;
    COORD PixPerCell;
} WinportGraphicsInfo;
typedef struct _nlsversioninfo{
    DWORD dwNLSVersionInfoSize;
    DWORD dwNLSVersion;
    DWORD dwDefinedVersion;
    DWORD dwEffectiveId;
    GUID guidCustomVersion;
} NLSVERSIONINFO, *LPNLSVERSIONINFO;
typedef UINT_PTR WPARAM;
typedef LONG_PTR LPARAM;
typedef LONG_PTR LRESULT;
typedef DWORD (*WINPORT_THREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef BOOL (*WINPORT_HANDLER_ROUTINE)( DWORD CtrlType );
typedef WINPORT_HANDLER_ROUTINE PHANDLER_ROUTINE;
typedef WINPORT_THREAD_START_ROUTINE LPTHREAD_START_ROUTINE, PTHREAD_START_ROUTINE;
typedef VOID (*PCONSOLE_SCROLL_CALLBACK)(PVOID pContext, HANDLE hConsole, unsigned int Width, CHAR_INFO *Chars);
       
#pragma pack(2)
typedef struct _INPUT_RECORD INPUT_RECORD;
typedef struct _CHAR_INFO CHAR_INFO;
       
typedef int FarLangMsgID;
typedef uint32_t FarKey;
enum VTLogExportFlags
{
 VT_LOGEXPORT_COLORED = 0x00000001,
 VT_LOGEXPORT_WITH_SCREENLINES = 0x00000002
};
typedef SIZE_T ( *FARAPIVT_ENUM_BACKGROUND)(HANDLE *con_hnds, SIZE_T count);
typedef uint32_t FARMESSAGEFLAGS;
typedef GUID UUID;
static const FARMESSAGEFLAGS
 FMSG_WARNING = 0x00000001,
 FMSG_ERRORTYPE = 0x00000002,
 FMSG_KEEPBACKGROUND = 0x00000004,
 FMSG_LEFTALIGN = 0x00000010,
 FMSG_ALLINONE = 0x00000020,
 FMSG_COLOURS = 0x00000040,
 FMSG_DISPLAYNOTIFY = 0x00000080,
 FMSG_ASYNC = 0x00000100,
 FMSG_MB_OK = 0x00010000,
 FMSG_MB_OKCANCEL = 0x00020000,
 FMSG_MB_ABORTRETRYIGNORE = 0x00030000,
 FMSG_MB_YESNO = 0x00040000,
 FMSG_MB_YESNOCANCEL = 0x00050000,
 FMSG_MB_RETRYCANCEL = 0x00060000,
 FMSG_NONE = 0;
typedef intptr_t ( *FARAPIMESSAGE)(
 INT_PTR PluginNumber,
 DWORD Flags,
 const wchar_t *HelpTopic,
 const wchar_t * const *Items,
 int ItemsNumber,
 int ButtonsNumber
);
enum DialogItemTypes
{
 DI_TEXT,
 DI_VTEXT,
 DI_SINGLEBOX,
 DI_DOUBLEBOX,
 DI_EDIT,
 DI_PSWEDIT,
 DI_FIXEDIT,
 DI_BUTTON,
 DI_CHECKBOX,
 DI_RADIOBUTTON,
 DI_COMBOBOX,
 DI_LISTBOX,
 DI_MEMOEDIT,
 DI_USERCONTROL=255,
};
enum FarDialogItemFlags
{
 DIF_NONE = 0,
 DIF_COLORMASK = 0x000000ffUL,
 DIF_SETCOLOR = 0x00000100UL,
 DIF_BOXCOLOR = 0x00000200UL,
 DIF_DEFAULT = 0x00000200UL,
 DIF_GROUP = 0x00000400UL,
 DIF_LEFTTEXT = 0x00000800UL,
 DIF_FOCUS = 0x00000800UL,
 DIF_MOVESELECT = 0x00001000UL,
 DIF_SHOWAMPERSAND = 0x00002000UL,
 DIF_CENTERGROUP = 0x00004000UL,
 DIF_NOBRACKETS = 0x00008000UL,
 DIF_MANUALADDHISTORY = 0x00008000UL,
 DIF_SEPARATOR = 0x00010000UL,
 DIF_SEPARATOR2 = 0x00020000UL,
 DIF_EDITOR = 0x00020000UL,
 DIF_LISTNOAMPERSAND = 0x00020000UL,
 DIF_LISTNOBOX = 0x00040000UL,
 DIF_HISTORY = 0x00040000UL,
 DIF_BTNNOCLOSE = 0x00040000UL,
 DIF_CENTERTEXT = 0x00040000UL,
 DIF_SEPARATORUSER = 0x00080000UL,
 DIF_SETSHIELD = 0x00080000UL,
 DIF_EDITEXPAND = 0x00080000UL,
 DIF_DROPDOWNLIST = 0x00100000UL,
 DIF_USELASTHISTORY = 0x00200000UL,
 DIF_MASKEDIT = 0x00400000UL,
 DIF_SELECTONENTRY = 0x00800000UL,
 DIF_3STATE = 0x00800000UL,
 DIF_EDITPATH = 0x01000000UL,
 DIF_LISTWRAPMODE = 0x01000000UL,
 DIF_NOAUTOCOMPLETE = 0x02000000UL,
 DIF_LISTAUTOHIGHLIGHT = 0x02000000UL,
 DIF_LISTNOCLOSE = 0x04000000UL,
 DIF_AUTOMATION = 0x08000000UL,
 DIF_HIDDEN = 0x10000000UL,
 DIF_READONLY = 0x20000000UL,
 DIF_NOFOCUS = 0x40000000UL,
 DIF_DISABLE = 0x80000000UL,
};
enum FarMessagesProc
{
 DM_FIRST=0,
 DM_CLOSE,
 DM_ENABLE,
 DM_ENABLEREDRAW,
 DM_GETDLGDATA,
 DM_GETDLGITEM,
 DM_GETDLGRECT,
 DM_GETTEXT,
 DM_GETTEXTLENGTH,
 DM_KEY,
 DM_MOVEDIALOG,
 DM_SETDLGDATA,
 DM_SETDLGITEM,
 DM_SETFOCUS,
 DM_REDRAW,
 DM_SETREDRAW=DM_REDRAW,
 DM_SETTEXT,
 DM_SETMAXTEXTLENGTH,
 DM_SETTEXTLENGTH=DM_SETMAXTEXTLENGTH,
 DM_SHOWDIALOG,
 DM_GETFOCUS,
 DM_GETCURSORPOS,
 DM_SETCURSORPOS,
 DM_GETTEXTPTR,
 DM_SETTEXTPTR,
 DM_SHOWITEM,
 DM_ADDHISTORY,
 DM_GETCHECK,
 DM_SETCHECK,
 DM_SET3STATE,
 DM_LISTSORT,
 DM_LISTGETITEM,
 DM_LISTGETCURPOS,
 DM_LISTSETCURPOS,
 DM_LISTDELETE,
 DM_LISTADD,
 DM_LISTADDSTR,
 DM_LISTUPDATE,
 DM_LISTINSERT,
 DM_LISTFINDSTRING,
 DM_LISTINFO,
 DM_LISTGETDATA,
 DM_LISTSETDATA,
 DM_LISTSETTITLES,
 DM_LISTGETTITLES,
 DM_RESIZEDIALOG,
 DM_SETITEMPOSITION,
 DM_GETDROPDOWNOPENED,
 DM_SETDROPDOWNOPENED,
 DM_SETHISTORY,
 DM_GETITEMPOSITION,
 DM_SETMOUSEEVENTNOTIFY,
 DM_EDITUNCHANGEDFLAG,
 DM_GETITEMDATA,
 DM_SETITEMDATA,
 DM_LISTSET,
 DM_LISTSETMOUSEREACTION,
 DM_GETCURSORSIZE,
 DM_SETCURSORSIZE,
 DM_LISTGETDATASIZE,
 DM_GETSELECTION,
 DM_SETSELECTION,
 DM_GETEDITPOSITION,
 DM_SETEDITPOSITION,
 DM_SETCOMBOBOXEVENT,
 DM_GETCOMBOBOXEVENT,
 DM_GETCONSTTEXTPTR,
 DM_GETDLGITEMSHORT,
 DM_SETDLGITEMSHORT,
 DM_GETDIALOGINFO,
 DM_SETREADONLY,
 DM_GETDEFAULTCOLOR,
 DM_GETTRUECOLOR,
 DM_GETCOLOR = DM_GETTRUECOLOR,
 DM_SETTRUECOLOR,
 DM_SETCOLOR = DM_SETTRUECOLOR,
 DM_SETTEXTPTRSILENT,
 DN_FIRST=0x1000,
 DN_BTNCLICK,
 DN_CTLCOLORDIALOG,
 DN_CTLCOLORDLGITEM,
 DN_CTLCOLORDLGLIST,
 DN_DRAWDIALOG,
 DN_DRAWDLGITEM,
 DN_EDITCHANGE,
 DN_ENTERIDLE,
 DN_GOTFOCUS,
 DN_HELP,
 DN_HOTKEY,
 DN_INITDIALOG,
 DN_KILLFOCUS,
 DN_LISTCHANGE,
 DN_MOUSECLICK,
 DN_DRAGGED,
 DN_RESIZECONSOLE,
 DN_MOUSEEVENT,
 DN_DRAWDIALOGDONE,
 DN_LISTHOTKEY,
 DN_DROPDOWNOPENED,
 DN_GETDIALOGINFO=DM_GETDIALOGINFO,
 DN_CLOSE=DM_CLOSE,
 DN_KEY=DM_KEY,
 DM_USER=0x4000,
 DM_KILLSAVESCREEN=DN_FIRST-1,
 DM_ALLKEYMODE=DN_FIRST-2,
 DN_ACTIVATEAPP=DM_USER-1,
};
enum FARCHECKEDSTATE
{
 BSTATE_UNCHECKED = 0,
 BSTATE_CHECKED = 1,
 BSTATE_3STATE = 2,
 BSTATE_TOGGLE = 3,
};
enum FARLISTMOUSEREACTIONTYPE
{
 LMRT_ONLYFOCUS = 0,
 LMRT_ALWAYS = 1,
 LMRT_NEVER = 2,
};
enum FARCOMBOBOXEVENTTYPE
{
 CBET_KEY = 0x00000001,
 CBET_MOUSE = 0x00000002,
};
enum LISTITEMFLAGS
{
 LIF_SELECTED = 0x00010000UL,
 LIF_CHECKED = 0x00020000UL,
 LIF_SEPARATOR = 0x00040000UL,
 LIF_DISABLE = 0x00080000UL,
 LIF_GRAYED = 0x00100000UL,
 LIF_HIDDEN = 0x00200000UL,
 LIF_DELETEUSERDATA = 0x80000000UL,
};
struct FarListItem
{
 DWORD Flags;
 const wchar_t *Text;
 DWORD Reserved[3];
};
struct FarListUpdate
{
 int Index;
 struct FarListItem Item;
};
struct FarListInsert
{
 int Index;
 struct FarListItem Item;
};
struct FarListGetItem
{
 int ItemIndex;
 struct FarListItem Item;
};
struct FarListPos
{
 int SelectPos;
 int TopPos;
};
enum FARLISTFINDFLAGS
{
 LIFIND_EXACTMATCH = 0x00000001,
 LIFIND_KEEPAMPERSAND = 0x00000002,
};
struct FarListFind
{
 int StartIndex;
 const wchar_t *Pattern;
 DWORD Flags;
 DWORD Reserved;
};
struct FarListDelete
{
 int StartIndex;
 int Count;
};
enum FARLISTINFOFLAGS
{
 LINFO_SHOWNOBOX = 0x00000400,
 LINFO_AUTOHIGHLIGHT = 0x00000800,
 LINFO_REVERSEHIGHLIGHT = 0x00001000,
 LINFO_WRAPMODE = 0x00008000,
 LINFO_SHOWAMPERSAND = 0x00010000,
};
struct FarListInfo
{
 DWORD Flags;
 int ItemsNumber;
 int SelectPos;
 int TopPos;
 int MaxHeight;
 int MaxLength;
 DWORD Reserved[6];
};
struct FarListItemData
{
 int Index;
 int DataSize;
 void *Data;
 DWORD Reserved;
};
struct FarList
{
 int ItemsNumber;
 struct FarListItem *Items;
};
struct FarListTitles
{
 int TitleLen;
 const wchar_t *Title;
 int BottomLen;
 const wchar_t *Bottom;
};
struct FarListColors
{
 DWORD Flags;
 DWORD Reserved;
 int ColorCount;
 uint64_t *Colors;
};
struct FarDialogItem
{
 int Type;
 int X1,Y1,X2,Y2;
 int Focus;
 union
 {
  DWORD_PTR Reserved;
  int Selected;
  const wchar_t *History;
  const wchar_t *Mask;
  struct FarList *ListItems;
  int ListPos;
  CHAR_INFO *VBuf;
 }
 Param
 ;
 DWORD Flags;
 int DefaultButton;
 const wchar_t *PtrData;
 size_t MaxLen;
};
struct FarDialogItemData
{
 size_t PtrLength;
 wchar_t *PtrData;
};
struct FarDialogEvent
{
 HANDLE hDlg;
 int Msg;
 int Param1;
 LONG_PTR Param2;
 LONG_PTR Result;
};
struct OpenDlgPluginData
{
 int ItemNumber;
 HANDLE hDlg;
};
struct DialogInfo
{
 int StructSize;
 GUID Id;
};
enum FARDIALOGFLAGS
{
 FDLG_WARNING = 0x00000001,
 FDLG_SMALLDIALOG = 0x00000002,
 FDLG_NODRAWSHADOW = 0x00000004,
 FDLG_NODRAWPANEL = 0x00000008,
 FDLG_NONMODAL = 0x00000010,
 FDLG_KEEPCONSOLETITLE = 0x00000020,
 FDLG_REGULARIDLE = 0x00000040
};
typedef LONG_PTR( *FARWINDOWPROC)(
 HANDLE hDlg,
 int Msg,
 int Param1,
 LONG_PTR Param2
);
typedef LONG_PTR( *FARAPISENDDLGMESSAGE)(
 HANDLE hDlg,
 int Msg,
 int Param1,
 LONG_PTR Param2
);
typedef LONG_PTR( *FARAPIDEFDLGPROC)(
 HANDLE hDlg,
 int Msg,
 int Param1,
 LONG_PTR Param2
);
typedef HANDLE( *FARAPIDIALOGINIT)(
 INT_PTR PluginNumber,
 int X1,
 int Y1,
 int X2,
 int Y2,
 const wchar_t *HelpTopic,
 struct FarDialogItem *Item,
 unsigned int ItemsNumber,
 DWORD Reserved,
 DWORD Flags,
 FARWINDOWPROC DlgProc,
 LONG_PTR Param
);
typedef int ( *FARAPIDIALOGRUN)(
 HANDLE hDlg
);
typedef void ( *FARAPIDIALOGFREE)(
 HANDLE hDlg
);
struct FarMenuItem
{
 const wchar_t *Text;
 int Selected;
 int Checked;
 int Separator;
};
enum MENUITEMFLAGS
{
 MIF_NONE = 0,
 MIF_SELECTED = 0x00010000UL,
 MIF_CHECKED = 0x00020000UL,
 MIF_SEPARATOR = 0x00040000UL,
 MIF_DISABLE = 0x00080000UL,
 MIF_GRAYED = 0x00100000UL,
 MIF_HIDDEN = 0x00200000UL,
 MIF_SUBMENU = 0x00400000UL,
};
struct FarMenuItemEx
{
 DWORD Flags;
 const wchar_t *Text;
 DWORD AccelKey;
 DWORD Reserved;
 DWORD_PTR UserData;
};
enum FARMENUFLAGS
{
 FMENU_SHOWAMPERSAND = 0x00000001,
 FMENU_WRAPMODE = 0x00000002,
 FMENU_AUTOHIGHLIGHT = 0x00000004,
 FMENU_REVERSEAUTOHIGHLIGHT = 0x00000008,
 FMENU_SHOWNOBOX = 0x00000010,
 FMENU_USEEXT = 0x00000020,
 FMENU_CHANGECONSOLETITLE = 0x00000040,
};
typedef int ( *FARAPIMENU)(
 INT_PTR PluginNumber,
 int X,
 int Y,
 int MaxHeight,
 DWORD Flags,
 const wchar_t *Title,
 const wchar_t *Bottom,
 const wchar_t *HelpTopic,
 const int *BreakKeys,
 int *BreakCode,
 const struct FarMenuItem *Item,
 int ItemsNumber
);
enum PLUGINPANELITEMFLAGS
{
 PPIF_PROCESSDESCR = 0x80000000,
 PPIF_SELECTED = 0x40000000,
 PPIF_USERDATA = 0x20000000,
};
struct FAR_FIND_DATA
{
 FILETIME ftCreationTime;
 FILETIME ftLastAccessTime;
 FILETIME ftLastWriteTime;
 uint64_t nPhysicalSize;
 uint64_t nFileSize;
 DWORD dwFileAttributes;
 DWORD dwUnixMode;
 wchar_t *lpwszFileName;
};
struct PluginPanelItem
{
 struct FAR_FIND_DATA FindData;
 DWORD_PTR UserData;
 DWORD Flags;
 DWORD NumberOfLinks;
 const wchar_t *Description;
 const wchar_t *Owner;
 const wchar_t *Group;
 const wchar_t * const *CustomColumnData;
 int CustomColumnNumber;
 DWORD CRC32;
 DWORD_PTR Reserved[2];
};
enum PANELINFOFLAGS
{
 PFLAGS_SHOWHIDDEN = 0x00000001,
 PFLAGS_HIGHLIGHT = 0x00000002,
 PFLAGS_REVERSESORTORDER = 0x00000004,
 PFLAGS_USESORTGROUPS = 0x00000008,
 PFLAGS_SELECTEDFIRST = 0x00000010,
 PFLAGS_REALNAMES = 0x00000020,
 PFLAGS_NUMERICSORT = 0x00000040,
 PFLAGS_PANELLEFT = 0x00000080,
 PFLAGS_DIRECTORIESFIRST = 0x00000100,
 PFLAGS_USECRC32 = 0x00000200,
 PFLAGS_CASESENSITIVESORT = 0x00000400,
 PFLAGS_HL_MARKERS_NOSHOW = 0x00000800,
 PFLAGS_HL_MARKERS_NOALIGN = 0x00001000,
 PFLAGS_EXECUTABLESFIRST = 0x00002000,
};
enum PANELINFOTYPE
{
 PTYPE_FILEPANEL,
 PTYPE_TREEPANEL,
 PTYPE_QVIEWPANEL,
 PTYPE_INFOPANEL
};
struct PanelInfo
{
 int PanelType;
 int Plugin;
 RECT PanelRect;
 int ItemsNumber;
 int SelectedItemsNumber;
 int CurrentItem;
 int TopPanelItem;
 int Visible;
 int Focus;
 int ViewMode;
 int SortMode;
 DWORD Flags;
 DWORD Reserved;
};
struct PanelRedrawInfo
{
 int CurrentItem;
 int TopPanelItem;
};
struct CmdLineSelect
{
 int SelStart;
 int SelEnd;
};
struct FarPanelLocation
{
 const wchar_t *PluginName;
 const wchar_t *HostFile;
 LONG_PTR Item;
 const wchar_t *Path;
};
enum FILE_CONTROL_COMMANDS
{
 FCTL_CLOSEPLUGIN,
 FCTL_GETPANELINFO,
 FCTL_UPDATEPANEL,
 FCTL_REDRAWPANEL,
 FCTL_GETCMDLINE,
 FCTL_SETCMDLINE,
 FCTL_SETSELECTION,
 FCTL_SETVIEWMODE,
 FCTL_INSERTCMDLINE,
 FCTL_SETUSERSCREEN,
 FCTL_SETPANELDIR,
 FCTL_SETCMDLINEPOS,
 FCTL_GETCMDLINEPOS,
 FCTL_SETSORTMODE,
 FCTL_SETSORTORDER,
 FCTL_GETCMDLINESELECTEDTEXT,
 FCTL_SETCMDLINESELECTION,
 FCTL_GETCMDLINESELECTION,
 FCTL_CHECKPANELSEXIST,
 FCTL_SETNUMERICSORT,
 FCTL_GETUSERSCREEN,
 FCTL_ISACTIVEPANEL,
 FCTL_GETPANELITEM,
 FCTL_GETSELECTEDPANELITEM,
 FCTL_GETCURRENTPANELITEM,
 FCTL_GETPANELDIR,
 FCTL_GETCOLUMNTYPES,
 FCTL_GETCOLUMNWIDTHS,
 FCTL_BEGINSELECTION,
 FCTL_ENDSELECTION,
 FCTL_CLEARSELECTION,
 FCTL_SETDIRECTORIESFIRST,
 FCTL_SETEXECUTABLESFIRST,
 FCTL_GETPANELFORMAT,
 FCTL_GETPANELHOSTFILE,
 FCTL_SETCASESENSITIVESORT,
 FCTL_GETPANELPLUGINHANDLE,
 FCTL_SETPANELLOCATION,
};
typedef int ( *FARAPICONTROL)(
 HANDLE hPlugin,
 int Command,
 int Param1,
 LONG_PTR Param2
);
typedef void ( *FARAPITEXT)(
 int X,
 int Y,
 uint64_t Color,
 const wchar_t *Str
);
typedef HANDLE( *FARAPISAVESCREEN)(int X1, int Y1, int X2, int Y2);
typedef void ( *FARAPIRESTORESCREEN)(HANDLE hScreen);
typedef int ( *FARAPIGETDIRLIST)(
 const wchar_t *Dir,
 struct FAR_FIND_DATA **pPanelItem,
 int *pItemsNumber
);
typedef int ( *FARAPIGETPLUGINDIRLIST)(
 INT_PTR PluginNumber,
 HANDLE hPlugin,
 const wchar_t *Dir,
 struct PluginPanelItem **pPanelItem,
 int *pItemsNumber
);
typedef void ( *FARAPIFREEDIRLIST)(struct FAR_FIND_DATA *PanelItem, int nItemsNumber);
typedef void ( *FARAPIFREEPLUGINDIRLIST)(struct PluginPanelItem *PanelItem, int nItemsNumber);
enum VIEWER_FLAGS
{
 VF_NONMODAL = 0x00000001,
 VF_DELETEONCLOSE = 0x00000002,
 VF_ENABLE_F6 = 0x00000004,
 VF_DISABLEHISTORY = 0x00000008,
 VF_IMMEDIATERETURN = 0x00000100,
 VF_DELETEONLYFILEONCLOSE = 0x00000200,
};
typedef int ( *FARAPIVIEWER)(
 const wchar_t *FileName,
 const wchar_t *Title,
 int X1,
 int Y1,
 int X2,
 int Y2,
 DWORD Flags,
 UINT CodePage
);
enum EDITOR_FLAGS
{
 EF_NONMODAL = 0x00000001,
 EF_CREATENEW = 0x00000002,
 EF_ENABLE_F6 = 0x00000004,
 EF_DISABLEHISTORY = 0x00000008,
 EF_DELETEONCLOSE = 0x00000010,
 EF_IMMEDIATERETURN = 0x00000100,
 EF_DELETEONLYFILEONCLOSE = 0x00000200,
 EF_LOCKED = 0x00000400,
 EF_OPENMODE_MASK = 0xF0000000,
 EF_OPENMODE_NEWIFOPEN = 0x10000000,
 EF_OPENMODE_USEEXISTING = 0x20000000,
 EF_OPENMODE_BREAKIFOPEN = 0x30000000,
 EF_OPENMODE_RELOADIFOPEN = 0x40000000,
 EF_SERVICEREGION = 0x00001000,
};
enum EDITOR_EXITCODE
{
 EEC_OPEN_ERROR = 0,
 EEC_MODIFIED = 1,
 EEC_NOT_MODIFIED = 2,
 EEC_LOADING_INTERRUPTED = 3,
 EEC_OPENED_EXISTING = 4,
 EEC_ALREADY_EXISTS = 5,
 EEC_OPEN_NEWINSTANCE = 6,
 EEC_RELOAD = 7,
};
typedef int ( *FARAPIEDITOR)(
 const wchar_t *FileName,
 const wchar_t *Title,
 int X1,
 int Y1,
 int X2,
 int Y2,
 DWORD Flags,
 int StartLine,
 int StartChar,
 UINT CodePage
);
typedef int ( *FARAPICMPNAME)(
 const wchar_t *Pattern,
 const wchar_t *String,
 int SkipPath
);
typedef const wchar_t*( *FARAPIGETMSG)(
 INT_PTR PluginNumber,
 FarLangMsgID MsgId
);
enum FarHelpFlags
{
 FHELP_NOSHOWERROR = 0x80000000,
 FHELP_SELFHELP = 0x00000000,
 FHELP_FARHELP = 0x00000001,
 FHELP_CUSTOMFILE = 0x00000002,
 FHELP_CUSTOMPATH = 0x00000004,
 FHELP_USECONTENTS = 0x40000000,
};
typedef BOOL ( *FARAPISHOWHELP)(
 const wchar_t *ModuleName,
 const wchar_t *Topic,
 DWORD Flags
);
enum ADVANCED_CONTROL_COMMANDS
{
 ACTL_GETFARVERSION = 0,
 ACTL_GETSYSWORDDIV = 2,
 ACTL_WAITKEY = 3,
 ACTL_GETCOLOR = 4,
 ACTL_GETARRAYCOLOR = 5,
 ACTL_EJECTMEDIA = 6,
 ACTL_KEYMACRO = 7,
 ACTL_POSTKEYSEQUENCE = 8,
 ACTL_GETWINDOWINFO = 9,
 ACTL_GETWINDOWCOUNT = 10,
 ACTL_SETCURRENTWINDOW = 11,
 ACTL_COMMIT = 12,
 ACTL_GETFARHWND = 13,
 ACTL_GETSYSTEMSETTINGS = 14,
 ACTL_GETPANELSETTINGS = 15,
 ACTL_GETINTERFACESETTINGS = 16,
 ACTL_GETCONFIRMATIONS = 17,
 ACTL_GETDESCSETTINGS = 18,
 ACTL_SETARRAYCOLOR = 19,
 ACTL_GETPLUGINMAXREADDATA = 21,
 ACTL_GETDIALOGSETTINGS = 22,
 ACTL_GETSHORTWINDOWINFO = 23,
 ACTL_REMOVEMEDIA = 24,
 ACTL_GETMEDIATYPE = 25,
 ACTL_GETPOLICIES = 26,
 ACTL_REDRAWALL = 27,
 ACTL_SYNCHRO = 28,
 ACTL_SETPROGRESSSTATE = 29,
 ACTL_SETPROGRESSVALUE = 30,
 ACTL_QUIT = 31,
 ACTL_GETFARRECT = 32,
 ACTL_GETCURSORPOS = 33,
 ACTL_SETCURSORPOS = 34,
 ACTL_PROGRESSNOTIFY = 35,
};
enum FarPoliciesFlags
{
 FFPOL_MAINMENUSYSTEM = 0x00000001,
 FFPOL_MAINMENUPANEL = 0x00000002,
 FFPOL_MAINMENUINTERFACE = 0x00000004,
 FFPOL_MAINMENULANGUAGE = 0x00000008,
 FFPOL_MAINMENUPLUGINS = 0x00000010,
 FFPOL_MAINMENUDIALOGS = 0x00000020,
 FFPOL_MAINMENUCONFIRMATIONS = 0x00000040,
 FFPOL_MAINMENUPANELMODE = 0x00000080,
 FFPOL_MAINMENUFILEDESCR = 0x00000100,
 FFPOL_MAINMENUFOLDERDESCR = 0x00000200,
 FFPOL_MAINMENUVIEWER = 0x00000800,
 FFPOL_MAINMENUEDITOR = 0x00001000,
 FFPOL_MAINMENUCOLORS = 0x00004000,
 FFPOL_MAINMENUHILIGHT = 0x00008000,
 FFPOL_MAINMENUSAVEPARAMS = 0x00020000,
 FFPOL_CREATEMACRO = 0x00040000,
 FFPOL_USEPSWITCH = 0x00080000,
 FFPOL_PERSONALPATH = 0x00100000,
 FFPOL_KILLTASK = 0x00200000,
 FFPOL_SHOWHIDDENDRIVES = 0x80000000,
};
enum FarSystemSettings
{
 FSS_DELETETORECYCLEBIN = 0x00000002,
 FSS_WRITETHROUGH = 0x00000004,
 FSS_RESERVED = 0x00000008,
 FSS_SAVECOMMANDSHISTORY = 0x00000020,
 FSS_SAVEFOLDERSHISTORY = 0x00000040,
 FSS_SAVEVIEWANDEDITHISTORY = 0x00000080,
 FSS_USEWINDOWSREGISTEREDTYPES = 0x00000100,
 FSS_AUTOSAVESETUP = 0x00000200,
 FSS_SCANSYMLINK = 0x00000400,
};
enum FarPanelSettings
{
 FPS_SHOWHIDDENANDSYSTEMFILES = 0x00000001,
 FPS_HIGHLIGHTFILES = 0x00000002,
 FPS_AUTOCHANGEFOLDER = 0x00000004,
 FPS_SELECTFOLDERS = 0x00000008,
 FPS_ALLOWREVERSESORTMODES = 0x00000010,
 FPS_SHOWCOLUMNTITLES = 0x00000020,
 FPS_SHOWSTATUSLINE = 0x00000040,
 FPS_SHOWFILESTOTALINFORMATION = 0x00000080,
 FPS_SHOWFREESIZE = 0x00000100,
 FPS_SHOWSCROLLBAR = 0x00000200,
 FPS_SHOWBACKGROUNDSCREENSNUMBER = 0x00000400,
 FPS_SHOWSORTMODELETTER = 0x00000800,
};
enum FarDialogSettings
{
 FDIS_HISTORYINDIALOGEDITCONTROLS = 0x00000001,
 FDIS_PERSISTENTBLOCKSINEDITCONTROLS = 0x00000002,
 FDIS_AUTOCOMPLETEININPUTLINES = 0x00000004,
 FDIS_BSDELETEUNCHANGEDTEXT = 0x00000008,
 FDIS_DELREMOVESBLOCKS = 0x00000010,
 FDIS_MOUSECLICKOUTSIDECLOSESDIALOG = 0x00000020,
};
enum FarInterfaceSettings
{
 FIS_CLOCKINPANELS = 0x00000001,
 FIS_CLOCKINVIEWERANDEDITOR = 0x00000002,
 FIS_MOUSE = 0x00000004,
 FIS_SHOWKEYBAR = 0x00000008,
 FIS_ALWAYSSHOWMENUBAR = 0x00000010,
 FIS_SHOWTOTALCOPYPROGRESSINDICATOR = 0x00000100,
 FIS_SHOWCOPYINGTIMEINFO = 0x00000200,
 FIS_USECTRLPGUPTOCHANGEDRIVE = 0x00000800,
 FIS_SHOWTOTALDELPROGRESSINDICATOR = 0x00001000,
};
enum FarConfirmationsSettings
{
 FCS_COPYOVERWRITE = 0x00000001,
 FCS_MOVEOVERWRITE = 0x00000002,
 FCS_DRAGANDDROP = 0x00000004,
 FCS_DELETE = 0x00000008,
 FCS_DELETENONEMPTYFOLDERS = 0x00000010,
 FCS_INTERRUPTOPERATION = 0x00000020,
 FCS_DISCONNECTNETWORKDRIVE = 0x00000040,
 FCS_RELOADEDITEDFILE = 0x00000080,
 FCS_CLEARHISTORYLIST = 0x00000100,
 FCS_EXIT = 0x00000200,
 FCS_OVERWRITEDELETEROFILES = 0x00000400,
};
enum FarDescriptionSettings
{
 FDS_UPDATEALWAYS = 0x00000001,
 FDS_UPDATEIFDISPLAYED = 0x00000002,
 FDS_SETHIDDEN = 0x00000004,
 FDS_UPDATEREADONLY = 0x00000008,
};
enum FAREJECTMEDIAFLAGS
{
 EJECT_NO_MESSAGE = 0x00000001,
 EJECT_LOAD_MEDIA = 0x00000002,
 EJECT_NOTIFY_AFTERREMOVE = 0x00000004,
 EJECT_READY = 0x80000000,
};
struct ActlEjectMedia
{
 DWORD Letter;
 DWORD Flags;
};
enum FARMEDIATYPE
{
 FMT_DRIVE_ERROR = -1,
 FMT_DRIVE_UNKNOWN = 0,
 FMT_DRIVE_NO_ROOT_DIR = 1,
 FMT_DRIVE_REMOVABLE = 2,
 FMT_DRIVE_FIXED = 3,
 FMT_DRIVE_REMOTE = 4,
 FMT_DRIVE_CDROM = 5,
 FMT_DRIVE_RAMDISK = 6,
 FMT_DRIVE_SUBSTITUTE = 15,
 FMT_DRIVE_REMOTE_NOT_CONNECTED = 16,
 FMT_DRIVE_CD_RW = 18,
 FMT_DRIVE_CD_RWDVD = 19,
 FMT_DRIVE_DVD_ROM = 20,
 FMT_DRIVE_DVD_RW = 21,
 FMT_DRIVE_DVD_RAM = 22,
 FMT_DRIVE_USBDRIVE = 40,
 FMT_DRIVE_NOT_INIT = 255,
};
enum FARMEDIATYPEFLAGS
{
 MEDIATYPE_NODETECTCDROM = 0x80000000,
};
struct ActlMediaType
{
 DWORD Letter;
 DWORD Flags;
 DWORD Reserved[2];
};
enum FARKEYSEQUENCEFLAGS
{
 KSFLAGS_DISABLEOUTPUT = 0x00000001,
 KSFLAGS_NOSENDKEYSTOPLUGINS = 0x00000002,
 KSFLAGS_SILENTCHECK = 0x00000001,
};
struct KeySequence
{
 DWORD Flags;
 int Count;
 const DWORD *Sequence;
};
enum FARMACROCOMMAND
{
 MCMD_LOADALL = 0,
 MCMD_SAVEALL = 1,
 MCMD_POSTMACROSTRING = 2,
 MCMD_COMPILEMACRO = 3,
 MCMD_CHECKMACRO = 4,
 MCMD_GETSTATE = 5,
 MCMD_GETAREA = 6,
 MCMD_RUNMACROSTRING = 7,
};
enum FARMACROAREA
{
 MACROAREA_OTHER = 0,
 MACROAREA_SHELL = 1,
 MACROAREA_VIEWER = 2,
 MACROAREA_EDITOR = 3,
 MACROAREA_DIALOG = 4,
 MACROAREA_SEARCH = 5,
 MACROAREA_DISKS = 6,
 MACROAREA_MAINMENU = 7,
 MACROAREA_MENU = 8,
 MACROAREA_HELP = 9,
 MACROAREA_INFOPANEL =10,
 MACROAREA_QVIEWPANEL =11,
 MACROAREA_TREEPANEL =12,
 MACROAREA_FINDFOLDER =13,
 MACROAREA_USERMENU =14,
 MACROAREA_AUTOCOMPLETION =15,
};
enum FARMACROSTATE
{
 MACROSTATE_NOMACRO = 0,
 MACROSTATE_EXECUTING = 1,
 MACROSTATE_EXECUTING_COMMON = 2,
 MACROSTATE_RECORDING = 3,
 MACROSTATE_RECORDING_COMMON = 4,
};
enum FARMACROPARSEERRORCODE
{
 MPEC_SUCCESS = 0,
 MPEC_UNRECOGNIZED_KEYWORD = 1,
 MPEC_UNRECOGNIZED_FUNCTION = 2,
 MPEC_FUNC_PARAM = 3,
 MPEC_NOT_EXPECTED_ELSE = 4,
 MPEC_NOT_EXPECTED_END = 5,
 MPEC_UNEXPECTED_EOS = 6,
 MPEC_EXPECTED_TOKEN = 7,
 MPEC_BAD_HEX_CONTROL_CHAR = 8,
 MPEC_BAD_CONTROL_CHAR = 9,
 MPEC_VAR_EXPECTED =10,
 MPEC_EXPR_EXPECTED =11,
 MPEC_ZEROLENGTHMACRO =12,
 MPEC_INTPARSERERROR =13,
 MPEC_CONTINUE_OTL =14,
};
struct MacroParseResult
{
 DWORD ErrCode;
 COORD ErrPos;
 const wchar_t *ErrSrc;
};
struct ActlKeyMacro
{
 int Command;
 union
 {
  struct
  {
   const wchar_t *SequenceText;
   DWORD Flags;
   DWORD AKey;
  } PlainText;
  struct KeySequence Compile;
  struct MacroParseResult MacroResult;
  DWORD_PTR Reserved[3];
 } Param;
};
enum FARMACROVARTYPE
{
 FMVT_INTEGER = 0,
 FMVT_STRING = 1,
 FMVT_DOUBLE = 2,
};
struct FarMacroValue
{
 enum FARMACROVARTYPE type;
 union
 {
  int64_t i;
  double d;
  const wchar_t *s;
 } v;
};
struct OpenMacroInfo
{
 size_t StructSize;
 size_t Count;
 struct FarMacroValue *Values;
};
struct FarMacroFunction
{
 DWORD Flags;
 const wchar_t *Name;
 int nParam;
 int oParam;
 const wchar_t *Syntax;
 const wchar_t *Description;
};
enum FARCOLORFLAGS
{
 FCLR_REDRAW = 0x00000001,
};
struct FarSetColors
{
 DWORD Flags;
 int StartIndex;
 int ColorCount;
 uint64_t *Colors;
};
struct FarTrueColor
{
 unsigned char R;
 unsigned char G;
 unsigned char B;
 unsigned char Flags;
};
struct FarTrueColorForeAndBack
{
 struct FarTrueColor Fore;
 struct FarTrueColor Back;
};
struct DialogItemTrueColors
{
 struct FarTrueColorForeAndBack Normal;
 struct FarTrueColorForeAndBack Hilighted;
 struct FarTrueColorForeAndBack Frame;
 struct FarTrueColorForeAndBack Reserved;
};
enum WINDOWINFO_TYPE
{
 WTYPE_VIRTUAL,
 WTYPE_PANELS=1,
 WTYPE_VIEWER,
 WTYPE_EDITOR,
 WTYPE_DIALOG,
 WTYPE_VMENU,
 WTYPE_HELP,
 WTYPE_COMBOBOX,
 WTYPE_FINDFOLDER,
 WTYPE_USER,
};
struct WindowInfo
{
 int Pos;
 int Type;
 int Modified;
 int Current;
 wchar_t *TypeName;
 int TypeNameSize;
 wchar_t *Name;
 int NameSize;
};
typedef uint32_t PROGRESSTATE;
static const PROGRESSTATE
 PGS_NOPROGRESS =0x0,
 PGS_INDETERMINATE=0x1,
 PGS_NORMAL =0x2,
 PGS_ERROR =0x4,
 PGS_PAUSED =0x8;
struct PROGRESSVALUE
{
 uint64_t Completed;
 uint64_t Total;
};
typedef INT_PTR( *FARAPIADVCONTROL)(
 INT_PTR ModuleNumber,
 int Command,
 void *Param1,
 void *Param2
);
enum VIEWER_CONTROL_COMMANDS
{
 VCTL_GETINFO,
 VCTL_QUIT,
 VCTL_REDRAW,
 VCTL_SETKEYBAR,
 VCTL_SETPOSITION,
 VCTL_SELECT,
 VCTL_SETMODE,
};
enum VIEWER_OPTIONS
{
 VOPT_SAVEFILEPOSITION=1,
 VOPT_AUTODETECTCODEPAGE=2,
};
enum VIEWER_SETMODE_TYPES
{
 VSMT_HEX,
 VSMT_WRAP,
 VSMT_WORDWRAP,
};
enum VIEWER_SETMODEFLAGS_TYPES
{
 VSMFL_REDRAW = 0x00000001,
};
struct ViewerSetMode
{
 int Type;
 union
 {
  int iParam;
  wchar_t *wszParam;
 } Param;
 DWORD Flags;
 DWORD Reserved;
};
struct ViewerSelect
{
 int64_t BlockStartPos;
 int BlockLen;
};
enum VIEWER_SETPOS_FLAGS
{
 VSP_NOREDRAW = 0x0001,
 VSP_PERCENT = 0x0002,
 VSP_RELATIVE = 0x0004,
 VSP_NORETNEWPOS = 0x0008,
};
struct ViewerSetPosition
{
 DWORD Flags;
 int64_t StartPos;
 int64_t LeftPos;
};
struct ViewerMode
{
 UINT CodePage;
 int Wrap;
 int WordWrap;
 int Hex;
 int Processed;
 DWORD Reserved[3];
};
struct ViewerInfo
{
 int StructSize;
 int ViewerID;
 const wchar_t *FileName;
 int64_t FileSize;
 int64_t FilePos;
 int WindowSizeX;
 int WindowSizeY;
 DWORD Options;
 int TabSize;
 struct ViewerMode CurMode;
 int64_t LeftPos;
};
typedef int ( *FARAPIVIEWERCONTROL)(
 int Command,
 void *Param
);
enum VIEWER_EVENTS
{
 VE_READ =0,
 VE_CLOSE =1,
 VE_GOTFOCUS =6,
 VE_KILLFOCUS =7,
};
enum EDITOR_EVENTS
{
 EE_READ =0,
 EE_SAVE =1,
 EE_REDRAW =2,
 EE_CLOSE =3,
 EE_GOTFOCUS =6,
 EE_KILLFOCUS =7,
};
enum DIALOG_EVENTS
{
 DE_DLGPROCINIT =0,
 DE_DEFDLGPROCINIT =1,
 DE_DLGPROCEND =2,
};
enum SYNCHRO_EVENTS
{
 SE_COMMONSYNCHRO =0,
};
enum EDITOR_CONTROL_COMMANDS
{
 ECTL_GETSTRING,
 ECTL_SETSTRING,
 ECTL_INSERTSTRING,
 ECTL_DELETESTRING,
 ECTL_DELETECHAR,
 ECTL_INSERTTEXT,
 ECTL_GETINFO,
 ECTL_SETPOSITION,
 ECTL_SELECT,
 ECTL_REDRAW,
 ECTL_TABTOREAL,
 ECTL_REALTOTAB,
 ECTL_EXPANDTABS,
 ECTL_SETTITLE,
 ECTL_READINPUT,
 ECTL_PROCESSINPUT,
 ECTL_ADDCOLOR,
 ECTL_GETCOLOR,
 ECTL_SAVEFILE,
 ECTL_QUIT,
 ECTL_SETKEYBAR,
 ECTL_PROCESSKEY,
 ECTL_SETPARAM,
 ECTL_GETBOOKMARKS,
 ECTL_TURNOFFMARKINGBLOCK,
 ECTL_DELETEBLOCK,
 ECTL_ADDSTACKBOOKMARK,
 ECTL_PREVSTACKBOOKMARK,
 ECTL_NEXTSTACKBOOKMARK,
 ECTL_CLEARSTACKBOOKMARKS,
 ECTL_DELETESTACKBOOKMARK,
 ECTL_GETSTACKBOOKMARKS,
 ECTL_UNDOREDO,
 ECTL_GETFILENAME,
 ECTL_ADDTRUECOLOR,
 ECTL_GETTRUECOLOR,
};
enum EDITOR_SETPARAMETER_TYPES
{
 ESPT_TABSIZE,
 ESPT_EXPANDTABS,
 ESPT_AUTOINDENT,
 ESPT_CURSORBEYONDEOL,
 ESPT_CHARCODEBASE,
 ESPT_CODEPAGE,
 ESPT_SAVEFILEPOSITION,
 ESPT_LOCKMODE,
 ESPT_SETWORDDIV,
 ESPT_GETWORDDIV,
 ESPT_SHOWWHITESPACE,
 ESPT_SETBOM,
};
struct EditorServiceRegion
{
 int Command;
 DWORD Flags;
};
struct EditorSetParameter
{
 int Type;
 union
 {
  int iParam;
  wchar_t *wszParam;
  DWORD Reserved1;
 } Param;
 DWORD Flags;
 DWORD Size;
};
enum EDITOR_UNDOREDO_COMMANDS
{
 EUR_BEGIN,
 EUR_END,
 EUR_UNDO,
 EUR_REDO
};
struct EditorUndoRedo
{
 int Command;
 DWORD_PTR Reserved[3];
};
struct EditorGetString
{
 int StringNumber;
 wchar_t *StringText;
 wchar_t *StringEOL;
 int StringLength;
 int SelStart;
 int SelEnd;
};
struct EditorSetString
{
 int StringNumber;
 const wchar_t *StringText;
 const wchar_t *StringEOL;
 int StringLength;
};
enum EXPAND_TABS
{
 EXPAND_NOTABS,
 EXPAND_ALLTABS,
 EXPAND_NEWTABS
};
enum EDITOR_OPTIONS
{
 EOPT_EXPANDALLTABS = 0x00000001,
 EOPT_PERSISTENTBLOCKS = 0x00000002,
 EOPT_DELREMOVESBLOCKS = 0x00000004,
 EOPT_AUTOINDENT = 0x00000008,
 EOPT_SAVEFILEPOSITION = 0x00000010,
 EOPT_AUTODETECTCODEPAGE= 0x00000020,
 EOPT_CURSORBEYONDEOL = 0x00000040,
 EOPT_EXPANDONLYNEWTABS = 0x00000080,
 EOPT_SHOWWHITESPACE = 0x00000100,
 EOPT_BOM = 0x00000200,
};
enum EDITOR_BLOCK_TYPES
{
 BTYPE_NONE,
 BTYPE_STREAM,
 BTYPE_COLUMN
};
enum EDITOR_CURRENTSTATE
{
 ECSTATE_MODIFIED = 0x00000001,
 ECSTATE_SAVED = 0x00000002,
 ECSTATE_LOCKED = 0x00000004,
};
struct EditorInfo
{
 int EditorID;
 int WindowSizeX;
 int WindowSizeY;
 int TotalLines;
 int CurLine;
 int CurPos;
 int CurTabPos;
 int TopScreenLine;
 int LeftPos;
 int Overtype;
 int BlockType;
 int BlockStartLine;
 DWORD Options;
 int TabSize;
 int BookMarkCount;
 DWORD CurState;
 UINT CodePage;
 DWORD Reserved[5];
};
struct EditorBookMarks
{
 long *Line;
 long *Cursor;
 long *ScreenLine;
 long *LeftPos;
 DWORD Reserved[4];
};
struct EditorSetPosition
{
 int CurLine;
 int CurPos;
 int CurTabPos;
 int TopScreenLine;
 int LeftPos;
 int Overtype;
};
struct EditorSelect
{
 int BlockType;
 int BlockStartLine;
 int BlockStartPos;
 int BlockWidth;
 int BlockHeight;
};
struct EditorConvertPos
{
 int StringNumber;
 int SrcPos;
 int DestPos;
};
enum EDITORCOLORFLAGS
{
 ECF_TAB1 = 0x10000,
};
struct EditorColor
{
 int StringNumber;
 int ColorItem;
 int StartPos;
 int EndPos;
 int Color;
};
struct EditorTrueColor
{
 struct EditorColor Base;
 struct FarTrueColorForeAndBack TrueColor;
};
struct EditorSaveFile
{
 const wchar_t *FileName;
 const wchar_t *FileEOL;
 UINT CodePage;
};
typedef int ( *FARAPIEDITORCONTROL)(
 int Command,
 void *Param
);
enum INPUTBOXFLAGS
{
 FIB_NONE = 0x00000000,
 FIB_ENABLEEMPTY = 0x00000001,
 FIB_PASSWORD = 0x00000002,
 FIB_EXPANDENV = 0x00000004,
 FIB_NOUSELASTHISTORY = 0x00000008,
 FIB_BUTTONS = 0x00000010,
 FIB_NOAMPERSAND = 0x00000020,
 FIB_CHECKBOX = 0x00010000,
 FIB_EDITPATH = 0x01000000,
};
typedef int ( *FARAPIINPUTBOX)(
 const wchar_t *Title,
 const wchar_t *SubTitle,
 const wchar_t *HistoryName,
 const wchar_t *SrcText,
 wchar_t *DestText,
 int DestLength,
 const wchar_t *HelpTopic,
 DWORD Flags
);
typedef int ( *FARAPICOLORDIALOG)(
 int Flags,
 uint64_t *Color
);
typedef int ( *FARAPIPLUGINSCONTROL)(
 HANDLE hHandle,
 int Command,
 int Param1,
 LONG_PTR Param2
);
typedef int ( *FARAPIFILEFILTERCONTROL)(
 HANDLE hHandle,
 int Command,
 int Param1,
 LONG_PTR Param2
);
typedef int ( *FARAPIREGEXPCONTROL)(
 HANDLE hHandle,
 int Command,
 LONG_PTR Param
);
typedef int ( *FARSTDSNPRINTF)(wchar_t *Buffer,size_t Sizebuf,const wchar_t *Format,...);
typedef int ( *FARSTDSSCANF)(const wchar_t *Buffer, const wchar_t *Format,...);
typedef void ( *FARSTDQSORT)(void *base, size_t nelem, size_t width, int ( *fcmp)(const void *, const void *));
typedef void ( *FARSTDQSORTEX)(void *base, size_t nelem, size_t width, int ( *fcmp)(const void *, const void *,void *userparam),void *userparam);
typedef void *( *FARSTDBSEARCH)(const void *key, const void *base, size_t nelem, size_t width, int ( *fcmp)(const void *, const void *));
typedef int ( *FARSTDGETFILEOWNER)(const wchar_t *Computer,const wchar_t *Name,wchar_t *Owner,int Size);
typedef int ( *FARSTDGETFILEGROUP)(const wchar_t *Computer,const wchar_t *Name,wchar_t *Group,int Size);
typedef int ( *FARSTDGETNUMBEROFLINKS)(const wchar_t *Name);
typedef int ( *FARSTDATOI)(const wchar_t *s);
typedef int64_t ( *FARSTDATOI64)(const wchar_t *s);
typedef wchar_t *( *FARSTDITOA64)(int64_t value, wchar_t *string, int radix);
typedef wchar_t *( *FARSTDITOA)(int value, wchar_t *string, int radix);
typedef wchar_t *( *FARSTDLTRIM)(wchar_t *Str);
typedef wchar_t *( *FARSTDRTRIM)(wchar_t *Str);
typedef wchar_t *( *FARSTDTRIM)(wchar_t *Str);
typedef wchar_t *( *FARSTDTRUNCSTR)(wchar_t *Str,int MaxCells);
typedef wchar_t *( *FARSTDTRUNCPATHSTR)(wchar_t *Str,int MaxCells);
typedef wchar_t *( *FARSTDQUOTESPACEONLY)(wchar_t *Str);
typedef const wchar_t*( *FARSTDPOINTTONAME)(const wchar_t *Path);
typedef int ( *FARSTDGETPATHROOT)(const wchar_t *Path,wchar_t *Root, int DestSize);
typedef BOOL ( *FARSTDADDENDSLASH)(wchar_t *Path);
typedef int ( *FARSTDCOPYTOCLIPBOARD)(const wchar_t *Data);
typedef wchar_t *( *FARSTDPASTEFROMCLIPBOARD)(void);
typedef FarKey ( *FARSTDINPUTRECORDTOKEY)(const INPUT_RECORD *r);
typedef int ( *FARSTDLOCALISLOWER)(wchar_t Ch);
typedef int ( *FARSTDLOCALISUPPER)(wchar_t Ch);
typedef int ( *FARSTDLOCALISALPHA)(wchar_t Ch);
typedef int ( *FARSTDLOCALISALPHANUM)(wchar_t Ch);
typedef wchar_t ( *FARSTDLOCALUPPER)(wchar_t LowerChar);
typedef wchar_t ( *FARSTDLOCALLOWER)(wchar_t UpperChar);
typedef void ( *FARSTDLOCALUPPERBUF)(wchar_t *Buf,int Length);
typedef void ( *FARSTDLOCALLOWERBUF)(wchar_t *Buf,int Length);
typedef void ( *FARSTDLOCALSTRUPR)(wchar_t *s1);
typedef void ( *FARSTDLOCALSTRLWR)(wchar_t *s1);
typedef int ( *FARSTDLOCALSTRICMP)(const wchar_t *s1,const wchar_t *s2);
typedef int ( *FARSTDLOCALSTRNICMP)(const wchar_t *s1,const wchar_t *s2,int n);
typedef int ( *FARSTDLOCALSTRCMP)(const wchar_t *s1,const wchar_t *s2);
typedef int ( *FARSTDLOCALSTRNCMP)(const wchar_t *s1,const wchar_t *s2,int n);
enum PROCESSNAME_FLAGS
{
 PN_CMPNAME = 0x00000000UL,
 PN_CMPNAMELIST = 0x00010000UL,
 PN_GENERATENAME = 0x00020000UL,
 PN_CHECKMASK = 0x00030000UL,
 PN_SKIPPATH = 0x01000000UL,
 PN_SHOWERRORMESSAGE = 0x02000000UL,
 PN_RESERVED1 = 0x04000000UL,
 PN_CASESENSITIVE = 0x08000000UL,
 PN_NONE = 0,
};
typedef int ( *FARSTDPROCESSNAME)(const wchar_t *param1, wchar_t *param2, DWORD size, DWORD flags);
typedef void ( *FARSTDUNQUOTE)(wchar_t *Str);
enum XLATMODE
{
 XLAT_SWITCHKEYBLAYOUT = 0x00000001UL,
 XLAT_SWITCHKEYBBEEP = 0x00000002UL,
 XLAT_USEKEYBLAYOUTNAME = 0x00000004UL,
 XLAT_CONVERTALLCMDLINE = 0x00010000UL,
};
typedef size_t ( *FARSTDKEYTOKEYNAME)(FarKey Key,wchar_t *KeyText,size_t Size);
typedef wchar_t*( *FARSTDXLAT)(wchar_t *Line,int StartPos,int EndPos,DWORD Flags);
typedef FarKey ( *FARSTDKEYNAMETOKEY)(const wchar_t *Name);
typedef int ( *FRSUSERFUNC)(
 const struct FAR_FIND_DATA *FData,
 const wchar_t *FullName,
 void *Param
);
enum FRSMODE
{
 FRS_NONE = 0x00,
 FRS_RETUPDIR = 0x01,
 FRS_RECUR = 0x02,
 FRS_SCANSYMLINK = 0x04,
};
typedef void ( *FARSTDRECURSIVESEARCH)(const wchar_t *InitDir,const wchar_t *Mask,FRSUSERFUNC Func,DWORD Flags,void *Param);
typedef int ( *FARSTDMKTEMP)(wchar_t *Dest, DWORD size, const wchar_t *Prefix);
typedef void ( *FARSTDDELETEBUFFER)(void *Buffer);
enum MKLINKOP
{
 FLINK_HARDLINK = 1,
 FLINK_JUNCTION = 2,
 FLINK_VOLMOUNT = 3,
 FLINK_SYMLINKFILE = 4,
 FLINK_SYMLINKDIR = 5,
 FLINK_SYMLINK = 6,
 FLINK_SHOWERRMSG = 0x10000,
 FLINK_DONOTUPDATEPANEL = 0x20000,
};
typedef int ( *FARSTDMKLINK)(const wchar_t *Src,const wchar_t *Dest,DWORD Flags);
typedef int ( *FARGETREPARSEPOINTINFO)(const wchar_t *Src, wchar_t *Dest,int DestSize);
enum CONVERTPATHMODES
{
 CPM_FULL,
 CPM_REAL,
 CPM_NATIVE,
};
typedef int ( *FARCONVERTPATH)(enum CONVERTPATHMODES Mode, const wchar_t *Src, wchar_t *Dest, int DestSize);
typedef DWORD ( *FARGETCURRENTDIRECTORY)(DWORD Size,wchar_t* Buffer);
enum EXECUTEFLAGS
{
 EF_HIDEOUT = 0x01,
 EF_NOWAIT = 0x02,
 EF_SUDO = 0x04,
 EF_NOTIFY = 0x08,
 EF_NOCMDPRINT = 0x10,
 EF_OPEN = 0x20,
 EF_MAYBGND = 0x40,
 EF_EXTERNALTERM = 0x80
};
typedef int ( *FAREXECUTE)(const wchar_t *CmdStr, unsigned int ExecFlags);
typedef int ( *FAREXECUTE_LIBRARY)(const wchar_t *Library, const wchar_t *Symbol, const wchar_t *CmdStr, unsigned int ExecFlags);
typedef void ( *FARDISPLAYNOTIFICATION)(const wchar_t *action, const wchar_t *object);
typedef int ( *FARDISPATCHNTRTHRDCALLS)();
typedef void ( *FARBACKGROUNDTASK)(const wchar_t *Info, BOOL Started);
typedef size_t ( *FARSTRCELLSCOUNT)(const wchar_t *Str, size_t CharsCount);
typedef size_t ( *FARSTRSIZEOFCELLS)(const wchar_t *Str, size_t CharsCount, size_t *CellsCount, BOOL RoundUp);
typedef int ( *FARGETFILEOWNER)(const wchar_t *Computer, const wchar_t *Name, wchar_t *Owner, int Size);
typedef int ( *FARSETFILEGROUP)(const wchar_t *Computer, const wchar_t *Name, wchar_t *Group, int Size);
typedef int ( *FARESETFILEMODE)(const wchar_t *Name, DWORD Mode, int SkipMode);
typedef int ( *FARESETFILETIME)(const wchar_t *Name, FILETIME *AccessTime, FILETIME *ModifyTime, DWORD FileAttr, int SkipMode);
typedef int ( *FARESETFILEGROUP)(const wchar_t *Name, const wchar_t *Group, int SkipMode);
typedef int ( *FARESETFILEOWNER)(const wchar_t *Name, const wchar_t *Owner, int SkipMode);
typedef const char *( *FAROWNERNAMEBYID)(uint32_t id);
typedef const char *( *FARGROUPNAMEBYID)(uint32_t id);
typedef size_t ( *FARREADLINK)(const char *path, char *buf, size_t bufsiz);
typedef BOOL ( *FARSDCLSTAT)(const wchar_t *path, void *s);
typedef int ( *FARSDCSYMLINK)(const char *path1, const char *path2);
typedef BOOL ( *FARGETFINDDATA)(const wchar_t *lpwszFileName, WIN32_FIND_DATAW *FindDataW);
typedef int ( *FARGETDATEFORMAT)(void);
typedef wchar_t ( *FARGETDATESEPARATOR)(void);
typedef wchar_t ( *FARGETTIMESEPARATOR)(void);
typedef wchar_t ( *FARGETDECIMALSEPARATOR)(void);
typedef BOOL ( *FARAPIVT_LOGEXPORT)(HANDLE con_hnd, DWORD flags, const wchar_t *file);
enum BOX_DEF_SYMBOLS
{
 BS_X_B0,
 BS_X_B1,
 BS_X_B2,
 BS_V1,
 BS_R_H1V1,
 BS_R_H2V1,
 BS_R_H1V2,
 BS_RT_H1V2,
 BS_RT_H2V1,
 BS_R_H2V2,
 BS_V2,
 BS_RT_H2V2,
 BS_RB_H2V2,
 BS_RB_H1V2,
 BS_RB_H2V1,
 BS_RT_H1V1,
 BS_LB_H1V1,
 BS_B_H1V1,
 BS_T_H1V1,
 BS_L_H1V1,
 BS_H1,
 BS_C_H1V1,
 BS_L_H2V1,
 BS_L_H1V2,
 BS_LB_H2V2,
 BS_LT_H2V2,
 BS_B_H2V2,
 BS_T_H2V2,
 BS_L_H2V2,
 BS_H2,
 BS_C_H2V2,
 BS_B_H2V1,
 BS_B_H1V2,
 BS_T_H2V1,
 BS_T_H1V2,
 BS_LB_H1V2,
 BS_LB_H2V1,
 BS_LT_H2V1,
 BS_LT_H1V2,
 BS_C_H1V2,
 BS_C_H2V1,
 BS_RB_H1V1,
 BS_LT_H1V1,
 BS_X_DB,
 BS_X_DC,
 BS_X_DD,
 BS_X_DE,
 BS_X_DF,
};
typedef struct FarStandardFunctions
{
 int StructSize;
 FARSTDATOI atoi;
 FARSTDATOI64 atoi64;
 FARSTDITOA itoa;
 FARSTDITOA64 itoa64;
 FARSTDSSCANF sscanf;
 FARSTDQSORT qsort;
 FARSTDBSEARCH bsearch;
 FARSTDQSORTEX qsortex;
 FARSTDSNPRINTF snprintf;
 DWORD_PTR Reserved[7];
 const WCHAR * BoxSymbols;
 FARSTDLOCALISLOWER LIsLower;
 FARSTDLOCALISUPPER LIsUpper;
 FARSTDLOCALISALPHA LIsAlpha;
 FARSTDLOCALISALPHANUM LIsAlphanum;
 FARSTDLOCALUPPER LUpper;
 FARSTDLOCALLOWER LLower;
 FARSTDLOCALUPPERBUF LUpperBuf;
 FARSTDLOCALLOWERBUF LLowerBuf;
 FARSTDLOCALSTRUPR LStrupr;
 FARSTDLOCALSTRLWR LStrlwr;
 FARSTDLOCALSTRICMP LStricmp;
 FARSTDLOCALSTRNICMP LStrnicmp;
 FARSTDUNQUOTE Unquote;
 FARSTDLTRIM LTrim;
 FARSTDRTRIM RTrim;
 FARSTDTRIM Trim;
 FARSTDTRUNCSTR TruncStr;
 FARSTDTRUNCPATHSTR TruncPathStr;
 FARSTDQUOTESPACEONLY QuoteSpaceOnly;
 FARSTDPOINTTONAME PointToName;
 FARSTDGETPATHROOT GetPathRoot;
 FARSTDADDENDSLASH AddEndSlash;
 FARSTDCOPYTOCLIPBOARD CopyToClipboard;
 FARSTDPASTEFROMCLIPBOARD PasteFromClipboard;
 FARSTDKEYTOKEYNAME FarKeyToName;
 FARSTDKEYNAMETOKEY FarNameToKey;
 FARSTDINPUTRECORDTOKEY FarInputRecordToKey;
 FARSTDXLAT XLat;
 FARSTDGETFILEOWNER GetFileOwner;
 FARSTDGETNUMBEROFLINKS GetNumberOfLinks;
 FARSTDRECURSIVESEARCH FarRecursiveSearch;
 FARSTDMKTEMP MkTemp;
 FARSTDDELETEBUFFER DeleteBuffer;
 FARSTDPROCESSNAME ProcessName;
 FARSTDMKLINK MkLink;
 FARCONVERTPATH ConvertPath;
 FARGETREPARSEPOINTINFO GetReparsePointInfo;
 FARGETCURRENTDIRECTORY GetCurrentDirectory;
 FAREXECUTE Execute;
 FAREXECUTE_LIBRARY ExecuteLibrary;
 FARDISPLAYNOTIFICATION DisplayNotification;
 FARDISPATCHNTRTHRDCALLS DispatchInterThreadCalls;
 FARBACKGROUNDTASK BackgroundTask;
 FARSTRCELLSCOUNT StrCellsCount;
 FARSTRSIZEOFCELLS StrSizeOfCells;
 FARSTDLOCALSTRICMP LStrcmp;
 FARSTDLOCALSTRNICMP LStrncmp;
 FARAPIVT_ENUM_BACKGROUND VTEnumBackground;
 FARAPIVT_LOGEXPORT VTLogExport;
 FARSETFILEGROUP GetFileGroup;
 FARESETFILEMODE ESetFileMode;
 FARESETFILETIME ESetFileTime;
 FARESETFILEGROUP ESetFileGroup;
 FARESETFILEOWNER ESetFileOwner;
 FAROWNERNAMEBYID OwnerNameByID;
 FARGROUPNAMEBYID GroupNameByID;
 FARGETFINDDATA GetFindData;
 FARGETDATEFORMAT GetDateFormat;
 FARGETDATESEPARATOR GetDateSeparator;
 FARGETTIMESEPARATOR GetTimeSeparator;
 FARGETDECIMALSEPARATOR GetDecimalSeparator;
} FARSTANDARDFUNCTIONS;
struct PluginStartupInfo
{
 int StructSize;
 const wchar_t *ModuleName;
 INT_PTR ModuleNumber;
 const wchar_t *RootKey;
 FARAPIMENU Menu;
 FARAPIMESSAGE Message;
 FARAPIGETMSG GetMsg;
 FARAPICONTROL Control;
 FARAPISAVESCREEN SaveScreen;
 FARAPIRESTORESCREEN RestoreScreen;
 FARAPIGETDIRLIST GetDirList;
 FARAPIGETPLUGINDIRLIST GetPluginDirList;
 FARAPIFREEDIRLIST FreeDirList;
 FARAPIFREEPLUGINDIRLIST FreePluginDirList;
 FARAPIVIEWER Viewer;
 FARAPIEDITOR Editor;
 FARAPICMPNAME CmpName;
 FARAPITEXT Text;
 FARAPIEDITORCONTROL EditorControl;
 FARSTANDARDFUNCTIONS *FSF;
 FARAPISHOWHELP ShowHelp;
 FARAPIADVCONTROL AdvControl;
 FARAPIADVCONTROL AdvControlAsync;
 FARAPIINPUTBOX InputBox;
 FARAPIDIALOGINIT DialogInit;
 FARAPIDIALOGRUN DialogRun;
 FARAPIDIALOGFREE DialogFree;
 FARAPISENDDLGMESSAGE SendDlgMessage;
 FARAPIDEFDLGPROC DefDlgProc;
 DWORD_PTR Reserved;
 FARAPIVIEWERCONTROL ViewerControl;
 FARAPIPLUGINSCONTROL PluginsControl;
 FARAPIFILEFILTERCONTROL FileFilterControl;
 FARAPIREGEXPCONTROL RegExpControl;
 FARAPICOLORDIALOG ColorDialog;
};
enum PLUGIN_FLAGS
{
 PF_PRELOAD = 0x0001,
 PF_DISABLEPANELS = 0x0002,
 PF_EDITOR = 0x0004,
 PF_VIEWER = 0x0008,
 PF_FULLCMDLINE = 0x0010,
 PF_DIALOG = 0x0020,
 PF_PREOPEN = 0x8000
};
struct PluginInfo
{
 int StructSize;
 DWORD Flags;
 const wchar_t * const *DiskMenuStrings;
 int *Reserved0;
 int DiskMenuStringsNumber;
 const wchar_t * const *PluginMenuStrings;
 int PluginMenuStringsNumber;
 const wchar_t * const *PluginConfigStrings;
 int PluginConfigStringsNumber;
 const wchar_t *CommandPrefix;
 DWORD SysID;
 int MacroFunctionNumber;
 const struct FarMacroFunction *MacroFunctions;
};
struct InfoPanelLine
{
 const wchar_t *Text;
 const wchar_t *Data;
 int Separator;
};
struct PanelMode
{
 const wchar_t *ColumnTypes;
 const wchar_t *ColumnWidths;
 const wchar_t * const *ColumnTitles;
 int FullScreen;
 int DetailedStatus;
 int AlignExtensions;
 int CaseConversion;
 const wchar_t *StatusColumnTypes;
 const wchar_t *StatusColumnWidths;
 DWORD Reserved[2];
};
enum OPENPLUGININFO_FLAGS
{
 OPIF_NONE = 0,
 OPIF_USEFILTER = 0x00000001,
 OPIF_USESORTGROUPS = 0x00000002,
 OPIF_USEHIGHLIGHTING = 0x00000004,
 OPIF_ADDDOTS = 0x00000008,
 OPIF_RAWSELECTION = 0x00000010,
 OPIF_REALNAMES = 0x00000020,
 OPIF_SHOWNAMESONLY = 0x00000040,
 OPIF_SHOWRIGHTALIGNNAMES = 0x00000080,
 OPIF_SHOWPRESERVECASE = 0x00000100,
 OPIF_COMPAREFATTIME = 0x00000400,
 OPIF_EXTERNALGET = 0x00000800,
 OPIF_EXTERNALPUT = 0x00001000,
 OPIF_EXTERNALDELETE = 0x00002000,
 OPIF_EXTERNALMKDIR = 0x00004000,
 OPIF_USEATTRHIGHLIGHTING = 0x00008000,
 OPIF_USECRC32 = 0x00010000,
 OPIF_HL_MARKERS_NOSHOW = 0x00020000,
 OPIF_HL_MARKERS_NOALIGN = 0x00040000,
 OPIF_USEFREESIZE = 0x00080000,
 OPIF_SHORTCUT = 0x00100000,
 OPIF_RECURSIVEPANEL = 0x00200000,
 OPIF_DELETEFILEONCLOSE = 0x00400000,
 OPIF_DELETEDIRONCLOSE = 0x00800000,
};
enum OPENPLUGININFO_SORTMODES
{
 SM_DEFAULT,
 SM_UNSORTED,
 SM_NAME,
 SM_EXT,
 SM_MTIME,
 SM_CTIME,
 SM_ATIME,
 SM_SIZE,
 SM_DESCR,
 SM_OWNER,
 SM_COMPRESSEDSIZE,
 SM_NUMLINKS,
 SM_FULLNAME,
 SM_CHTIME,
};
struct KeyBarTitles
{
 wchar_t *Titles[12];
 wchar_t *CtrlTitles[12];
 wchar_t *AltTitles[12];
 wchar_t *ShiftTitles[12];
 wchar_t *CtrlShiftTitles[12];
 wchar_t *AltShiftTitles[12];
 wchar_t *CtrlAltTitles[12];
};
typedef uint32_t OPERATION_MODES;
static const OPERATION_MODES
 OPM_SILENT =0x0000000000000001ULL,
 OPM_FIND =0x0000000000000002ULL,
 OPM_VIEW =0x0000000000000004ULL,
 OPM_EDIT =0x0000000000000008ULL,
 OPM_TOPLEVEL =0x0000000000000010ULL,
 OPM_DESCR =0x0000000000000020ULL,
 OPM_QUICKVIEW =0x0000000000000040ULL,
 OPM_PGDN =0x0000000000000080ULL,
 OPM_COMMANDS =0x0000000000000100ULL,
 OPM_NONE =0;
struct OpenPluginInfo
{
 int StructSize;
 DWORD Flags;
 const wchar_t *HostFile;
 const wchar_t *CurDir;
 const wchar_t *Format;
 const wchar_t *PanelTitle;
 const struct InfoPanelLine *InfoLines;
 int InfoLinesNumber;
 const wchar_t * const *DescrFiles;
 int DescrFilesNumber;
 const struct PanelMode *PanelModesArray;
 int PanelModesNumber;
 int StartPanelMode;
 int StartSortMode;
 int StartSortOrder;
 const struct KeyBarTitles *KeyBar;
 const wchar_t *ShortcutData;
 const wchar_t *CurURL;
 long Reserved;
};
struct AnalyseInfo
{
 size_t StructSize;
 const wchar_t *FileName;
 void *Buffer;
 size_t BufferSize;
 uint32_t OpMode;
 void *Instance;
};
struct CloseAnalyseInfo
{
 size_t StructSize;
 HANDLE Handle;
 void* Instance;
};
struct OpenAnalyseInfo
{
 size_t StructSize;
 struct AnalyseInfo* Info;
 HANDLE Handle;
};
enum OPENPLUGIN_OPENFROM
{
 OPEN_FROM_MASK = 0x000000FF,
 OPEN_DISKMENU = 0,
 OPEN_PLUGINSMENU = 1,
 OPEN_FINDLIST = 2,
 OPEN_SHORTCUT = 3,
 OPEN_COMMANDLINE = 4,
 OPEN_EDITOR = 5,
 OPEN_VIEWER = 6,
 OPEN_FILEPANEL = 7,
 OPEN_DIALOG = 8,
 OPEN_ANALYSE = 9,
 OPEN_FROMMACRO_MASK = 0x000F0000,
 OPEN_FROMMACRO = 0x00010000,
 OPEN_FROMMACROSTRING = 0x00020000,
};
enum FAR_PKF_FLAGS
{
 PKF_CONTROL = 0x00000001,
 PKF_ALT = 0x00000002,
 PKF_SHIFT = 0x00000004,
 PKF_PREPROCESS = 0x00080000,
};
enum FAR_EVENTS
{
 FE_CHANGEVIEWMODE =0,
 FE_REDRAW =1,
 FE_IDLE =2,
 FE_CLOSE =3,
 FE_BREAK =4,
 FE_COMMAND =5,
 FE_GOTFOCUS =6,
 FE_KILLFOCUS =7,
};
enum FAR_PLUGINS_CONTROL_COMMANDS
{
 PCTL_LOADPLUGIN = 0,
 PCTL_UNLOADPLUGIN = 1,
 PCTL_FORCEDLOADPLUGIN = 2,
 PCTL_CACHEFORGET = 3
};
enum FAR_PLUGIN_LOAD_TYPE
{
 PLT_PATH = 0,
};
enum FAR_FILE_FILTER_CONTROL_COMMANDS
{
 FFCTL_CREATEFILEFILTER = 0,
 FFCTL_FREEFILEFILTER,
 FFCTL_OPENFILTERSMENU,
 FFCTL_STARTINGTOFILTER,
 FFCTL_ISFILEINFILTER,
};
enum FAR_FILE_FILTER_TYPE
{
 FFT_PANEL = 0,
 FFT_FINDFILE,
 FFT_COPY,
 FFT_SELECT,
 FFT_CUSTOM,
};
enum FAR_REGEXP_CONTROL_COMMANDS
{
 RECTL_CREATE=0,
 RECTL_FREE,
 RECTL_COMPILE,
 RECTL_OPTIMIZE,
 RECTL_MATCHEX,
 RECTL_SEARCHEX,
 RECTL_BRACKETSCOUNT
};
struct RegExpMatch
{
 int start,end;
};
struct RegExpSearch
{
 const wchar_t* Text;
 int Position;
 int Length;
 struct RegExpMatch* Match;
 int Count;
 void* Reserved;
};
 void PluginModuleOpen(const char *path);
 void ClosePluginW(HANDLE hPlugin);
 int CompareW(HANDLE hPlugin,const struct PluginPanelItem *Item1,const struct PluginPanelItem *Item2,unsigned int Mode);
 int ConfigureW(int ItemNumber);
 int DeleteFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
 void ExitFARW(void);
 int MayExitFARW(void);
 void FreeFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
 void FreeVirtualFindDataW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber);
 int GetFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t **DestPath,int OpMode);
 int GetFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,int OpMode);
 int GetLinkTargetW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,wchar_t *Target,size_t TargetSize,int OpMode);
 int GetMinFarVersionW(void);
 void GetOpenPluginInfoW(HANDLE hPlugin,struct OpenPluginInfo *Info);
 void GetPluginInfoW(struct PluginInfo *Info);
 int GetVirtualFindDataW(HANDLE hPlugin,struct PluginPanelItem **pPanelItem,int *pItemsNumber,const wchar_t *Path);
 int MakeDirectoryW(HANDLE hPlugin,const wchar_t **Name,int OpMode);
 HANDLE OpenFilePluginW(const wchar_t *Name,const unsigned char *Data,int DataSize,int OpMode);
 HANDLE OpenPluginW(int OpenFrom,INT_PTR Item);
 int ProcessDialogEventW(int Event,void *Param);
 int ProcessEditorEventW(int Event,void *Param);
 int ProcessEditorInputW(const INPUT_RECORD *Rec);
 int ProcessEventW(HANDLE hPlugin,int Event,void *Param);
 int ProcessHostFileW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int OpMode);
 int ProcessKeyW(HANDLE hPlugin,int Key,unsigned int ControlState);
 int ProcessMacroFuncW(const wchar_t *Name, const struct FarMacroValue *Params, int nParams, struct FarMacroValue **Results, int *nResults);
 int ProcessSynchroEventW(int Event,void *Param);
 int ProcessViewerEventW(int Event,void *Param);
 int PutFilesW(HANDLE hPlugin,struct PluginPanelItem *PanelItem,int ItemsNumber,int Move,const wchar_t *SrcPath,int OpMode);
 int SetDirectoryW(HANDLE hPlugin,const wchar_t *Dir,int OpMode);
 int SetFindListW(HANDLE hPlugin,const struct PluginPanelItem *PanelItem,int ItemsNumber);
 void SetStartupInfoW(const struct PluginStartupInfo *Info);
#pragma pack()
       
enum PaletteColors
{
 COL_MENUTEXT,
 COL_MENUSELECTEDTEXT,
 COL_MENUHIGHLIGHT,
 COL_MENUSELECTEDHIGHLIGHT,
 COL_MENUBOX,
 COL_MENUTITLE,
 COL_HMENUTEXT,
 COL_HMENUSELECTEDTEXT,
 COL_HMENUHIGHLIGHT,
 COL_HMENUSELECTEDHIGHLIGHT,
 COL_PANELTEXT,
 COL_PANELSELECTEDTEXT,
 COL_PANELHIGHLIGHTTEXT,
 COL_PANELINFOTEXT,
 COL_PANELCURSOR,
 COL_PANELSELECTEDCURSOR,
 COL_PANELTITLE,
 COL_PANELSELECTEDTITLE,
 COL_PANELCOLUMNTITLE,
 COL_PANELTOTALINFO,
 COL_PANELSELECTEDINFO,
 COL_DIALOGTEXT,
 COL_DIALOGHIGHLIGHTTEXT,
 COL_DIALOGBOX,
 COL_DIALOGBOXTITLE,
 COL_DIALOGHIGHLIGHTBOXTITLE,
 COL_DIALOGEDIT,
 COL_DIALOGBUTTON,
 COL_DIALOGSELECTEDBUTTON,
 COL_DIALOGHIGHLIGHTBUTTON,
 COL_DIALOGHIGHLIGHTSELECTEDBUTTON,
 COL_DIALOGLISTTEXT,
 COL_DIALOGLISTSELECTEDTEXT,
 COL_DIALOGLISTHIGHLIGHT,
 COL_DIALOGLISTSELECTEDHIGHLIGHT,
 COL_WARNDIALOGTEXT,
 COL_WARNDIALOGHIGHLIGHTTEXT,
 COL_WARNDIALOGBOX,
 COL_WARNDIALOGBOXTITLE,
 COL_WARNDIALOGHIGHLIGHTBOXTITLE,
 COL_WARNDIALOGEDIT,
 COL_WARNDIALOGBUTTON,
 COL_WARNDIALOGSELECTEDBUTTON,
 COL_WARNDIALOGHIGHLIGHTBUTTON,
 COL_WARNDIALOGHIGHLIGHTSELECTEDBUTTON,
 COL_KEYBARNUM,
 COL_KEYBARTEXT,
 COL_KEYBARBACKGROUND,
 COL_COMMANDLINE,
 COL_CLOCK,
 COL_VIEWERTEXT,
 COL_VIEWERSELECTEDTEXT,
 COL_VIEWERSTATUS,
 COL_EDITORTEXT,
 COL_EDITORSELECTEDTEXT,
 COL_EDITORSTATUS,
 COL_HELPTEXT,
 COL_HELPHIGHLIGHTTEXT,
 COL_HELPTOPIC,
 COL_HELPSELECTEDTOPIC,
 COL_HELPBOX,
 COL_HELPBOXTITLE,
 COL_PANELDRAGTEXT,
 COL_DIALOGEDITUNCHANGED,
 COL_PANELSCROLLBAR,
 COL_HELPSCROLLBAR,
 COL_PANELBOX,
 COL_PANELSCREENSNUMBER,
 COL_DIALOGEDITSELECTED,
 COL_COMMANDLINESELECTED,
 COL_VIEWERARROWS,
 COL_RESERVED0,
 COL_DIALOGLISTSCROLLBAR,
 COL_MENUSCROLLBAR,
 COL_VIEWERSCROLLBAR,
 COL_COMMANDLINEPREFIX,
 COL_DIALOGDISABLED,
 COL_DIALOGEDITDISABLED,
 COL_DIALOGLISTDISABLED,
 COL_WARNDIALOGDISABLED,
 COL_WARNDIALOGEDITDISABLED,
 COL_WARNDIALOGLISTDISABLED,
 COL_MENUDISABLEDTEXT,
 COL_EDITORCLOCK,
 COL_VIEWERCLOCK,
 COL_DIALOGLISTTITLE,
 COL_DIALOGLISTBOX,
 COL_WARNDIALOGEDITSELECTED,
 COL_WARNDIALOGEDITUNCHANGED,
 COL_DIALOGCOMBOTEXT,
 COL_DIALOGCOMBOSELECTEDTEXT,
 COL_DIALOGCOMBOHIGHLIGHT,
 COL_DIALOGCOMBOSELECTEDHIGHLIGHT,
 COL_DIALOGCOMBOBOX,
 COL_DIALOGCOMBOTITLE,
 COL_DIALOGCOMBODISABLED,
 COL_DIALOGCOMBOSCROLLBAR,
 COL_WARNDIALOGLISTTEXT,
 COL_WARNDIALOGLISTSELECTEDTEXT,
 COL_WARNDIALOGLISTHIGHLIGHT,
 COL_WARNDIALOGLISTSELECTEDHIGHLIGHT,
 COL_WARNDIALOGLISTBOX,
 COL_WARNDIALOGLISTTITLE,
 COL_WARNDIALOGLISTSCROLLBAR,
 COL_WARNDIALOGCOMBOTEXT,
 COL_WARNDIALOGCOMBOSELECTEDTEXT,
 COL_WARNDIALOGCOMBOHIGHLIGHT,
 COL_WARNDIALOGCOMBOSELECTEDHIGHLIGHT,
 COL_WARNDIALOGCOMBOBOX,
 COL_WARNDIALOGCOMBOTITLE,
 COL_WARNDIALOGCOMBODISABLED,
 COL_WARNDIALOGCOMBOSCROLLBAR,
 COL_DIALOGLISTARROWS,
 COL_DIALOGLISTARROWSDISABLED,
 COL_DIALOGLISTARROWSSELECTED,
 COL_DIALOGCOMBOARROWS,
 COL_DIALOGCOMBOARROWSDISABLED,
 COL_DIALOGCOMBOARROWSSELECTED,
 COL_WARNDIALOGLISTARROWS,
 COL_WARNDIALOGLISTARROWSDISABLED,
 COL_WARNDIALOGLISTARROWSSELECTED,
 COL_WARNDIALOGCOMBOARROWS,
 COL_WARNDIALOGCOMBOARROWSDISABLED,
 COL_WARNDIALOGCOMBOARROWSSELECTED,
 COL_MENUARROWS,
 COL_MENUARROWSDISABLED,
 COL_MENUARROWSSELECTED,
 COL_COMMANDLINEUSERSCREEN,
 COL_EDITORSCROLLBAR,
 COL_MENUGRAYTEXT,
 COL_MENUSELECTEDGRAYTEXT,
 COL_DIALOGCOMBOGRAY,
 COL_DIALOGCOMBOSELECTEDGRAYTEXT,
 COL_DIALOGLISTGRAY,
 COL_DIALOGLISTSELECTEDGRAYTEXT,
 COL_WARNDIALOGCOMBOGRAY,
 COL_WARNDIALOGCOMBOSELECTEDGRAYTEXT,
 COL_WARNDIALOGLISTGRAY,
 COL_WARNDIALOGLISTSELECTEDGRAYTEXT,
 COL_DIALOGDEFAULTBUTTON,
 COL_DIALOGSELECTEDDEFAULTBUTTON,
 COL_DIALOGHIGHLIGHTDEFAULTBUTTON,
 COL_DIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
 COL_WARNDIALOGDEFAULTBUTTON,
 COL_WARNDIALOGSELECTEDDEFAULTBUTTON,
 COL_WARNDIALOGHIGHLIGHTDEFAULTBUTTON,
 COL_WARNDIALOGHIGHLIGHTSELECTEDDEFAULTBUTTON,
 COL_DIALOGOVERFLOWARROW,
 COL_WARNDIALOGOVERFLOWARROW,
 COL_DIALOGLISTPREFIX,
 COL_DIALOGLISTSELPREFIX,
 COL_DIALOGCOMBOPREFIX,
 COL_DIALOGCOMBOSELPREFIX,
 COL_WARNDIALOGLISTPREFIX,
 COL_WARNDIALOGLISTSELPREFIX,
 COL_WARNDIALOGCOMBOPREFIX,
 COL_WARNDIALOGCOMBOSELPREFIX,
 COL_MENUPREFIX,
 COL_MENUSELPREFIX,
 COL_LASTPALETTECOLOR
};
       
enum BaseDefKeyboard
{
 KEY_CTRLMASK = 0xFFC00000U,
 KEY_M_OEM = 0x00400000U,
 KEY_M_SPEC = 0x00800000U,
 KEY_ALTDIGIT = 0x01000000U,
 KEY_RSHIFT = 0x02000000U,
 KEY_CTRL = 0x04000000U,
 KEY_ALT = 0x08000000U,
 KEY_SHIFT = 0x10000000U,
 KEY_RCTRL = 0x20000000U,
 KEY_RALT = 0x40000000U,
 KEY_BRACKET = '[',
 KEY_BACKBRACKET = ']',
 KEY_COMMA = ',',
 KEY_QUOTE = '"',
 KEY_DOT = '.',
 KEY_SLASH = '/',
 KEY_COLON = ':',
 KEY_SEMICOLON = ';',
 KEY_BACKSLASH = '\\',
 KEY_BS = 0x00000008U,
 KEY_TAB = 0x00000009U,
 KEY_ENTER = 0x0000000DU,
 KEY_ESC = 0x0000001BU,
 KEY_SPACE = 0x00000020U,
 KEY_MASKF = 0x002FFFFFU,
 KEY_FKEY_BEGIN = 0x00200000U,
 KEY_BREAK = 0x00200000U + 0x03,
 KEY_PAUSE = 0x00200000U + 0x13,
 KEY_CAPSLOCK = 0x00200000U + 0x14,
 KEY_PGUP = 0x00200000U + 0x21,
 KEY_PGDN = 0x00200000U + 0x22,
 KEY_END = 0x00200000U + 0x23,
 KEY_HOME = 0x00200000U + 0x24,
 KEY_LEFT = 0x00200000U + 0x25,
 KEY_UP = 0x00200000U + 0x26,
 KEY_RIGHT = 0x00200000U + 0x27,
 KEY_DOWN = 0x00200000U + 0x28,
 KEY_PRNTSCRN = 0x00200000U + 0x2C,
 KEY_INS = 0x00200000U + 0x2D,
 KEY_DEL = 0x00200000U + 0x2E,
 KEY_LWIN = 0x00200000U + 0x5B,
 KEY_RWIN = 0x00200000U + 0x5C,
 KEY_APPS = 0x00200000U + 0x5D,
 KEY_STANDBY = 0x00200000U + 0x5F,
 KEY_NUMPAD0 = 0x00200000U + 0x60,
 KEY_NUMPAD1 = 0x00200000U + 0x61,
 KEY_NUMPAD2 = 0x00200000U + 0x62,
 KEY_NUMPAD3 = 0x00200000U + 0x63,
 KEY_NUMPAD4 = 0x00200000U + 0x64,
 KEY_NUMPAD5 = 0x00200000U + 0x65,
 KEY_CLEAR = KEY_NUMPAD5,
 KEY_NUMPAD6 = 0x00200000U + 0x66,
 KEY_NUMPAD7 = 0x00200000U + 0x67,
 KEY_NUMPAD8 = 0x00200000U + 0x68,
 KEY_NUMPAD9 = 0x00200000U + 0x69,
 KEY_MULTIPLY = 0x00200000U + 0x6A,
 KEY_ADD = 0x00200000U + 0x6B,
 KEY_SUBTRACT = 0x00200000U + 0x6D,
 KEY_DECIMAL = 0x00200000U + 0x6E,
 KEY_DIVIDE = 0x00200000U + 0x6F,
 KEY_F1 = 0x00200000U + 0x70,
 KEY_F2 = 0x00200000U + 0x71,
 KEY_F3 = 0x00200000U + 0x72,
 KEY_F4 = 0x00200000U + 0x73,
 KEY_F5 = 0x00200000U + 0x74,
 KEY_F6 = 0x00200000U + 0x75,
 KEY_F7 = 0x00200000U + 0x76,
 KEY_F8 = 0x00200000U + 0x77,
 KEY_F9 = 0x00200000U + 0x78,
 KEY_F10 = 0x00200000U + 0x79,
 KEY_F11 = 0x00200000U + 0x7A,
 KEY_F12 = 0x00200000U + 0x7B,
 KEY_F13 = 0x00200000U + 0x7C,
 KEY_F14 = 0x00200000U + 0x7D,
 KEY_F15 = 0x00200000U + 0x7E,
 KEY_F16 = 0x00200000U + 0x7F,
 KEY_F17 = 0x00200000U + 0x80,
 KEY_F18 = 0x00200000U + 0x81,
 KEY_F19 = 0x00200000U + 0x82,
 KEY_F20 = 0x00200000U + 0x83,
 KEY_F21 = 0x00200000U + 0x84,
 KEY_F22 = 0x00200000U + 0x85,
 KEY_F23 = 0x00200000U + 0x86,
 KEY_F24 = 0x00200000U + 0x87,
 KEY_NUMLOCK = 0x00200000U + 0x90,
 KEY_SCROLLLOCK = 0x00200000U + 0x91,
 KEY_BROWSER_BACK = 0x00200000U + 0xA6,
 KEY_BROWSER_FORWARD = 0x00200000U + 0xA7,
 KEY_BROWSER_REFRESH = 0x00200000U + 0xA8,
 KEY_BROWSER_STOP = 0x00200000U + 0xA9,
 KEY_BROWSER_SEARCH = 0x00200000U + 0xAA,
 KEY_BROWSER_FAVORITES = 0x00200000U + 0xAB,
 KEY_BROWSER_HOME = 0x00200000U + 0xAC,
 KEY_VOLUME_MUTE = 0x00200000U + 0xAD,
 KEY_VOLUME_DOWN = 0x00200000U + 0xAE,
 KEY_VOLUME_UP = 0x00200000U + 0xAF,
 KEY_MEDIA_NEXT_TRACK = 0x00200000U + 0xB0,
 KEY_MEDIA_PREV_TRACK = 0x00200000U + 0xB1,
 KEY_MEDIA_STOP = 0x00200000U + 0xB2,
 KEY_MEDIA_PLAY_PAUSE = 0x00200000U + 0xB3,
 KEY_LAUNCH_MAIL = 0x00200000U + 0xB4,
 KEY_LAUNCH_MEDIA_SELECT = 0x00200000U + 0xB5,
 KEY_LAUNCH_APP1 = 0x00200000U + 0xB6,
 KEY_LAUNCH_APP2 = 0x00200000U + 0xB7,
 KEY_CTRLALTSHIFTPRESS = (0x00200000U + 0x00080000U) + 1,
 KEY_CTRLALTSHIFTRELEASE = (0x00200000U + 0x00080000U) + 2,
 KEY_MSWHEEL_UP = (0x00200000U + 0x00080000U) + 3,
 KEY_MSWHEEL_DOWN = (0x00200000U + 0x00080000U) + 4,
 KEY_RCTRLALTSHIFTPRESS = (0x00200000U + 0x00080000U) + 7,
 KEY_RCTRLALTSHIFTRELEASE = (0x00200000U + 0x00080000U) + 8,
 KEY_NUMDEL = (0x00200000U + 0x00080000U) + 9,
 KEY_NUMENTER = (0x00200000U + 0x00080000U) + 0xB,
 KEY_MSWHEEL_LEFT = (0x00200000U + 0x00080000U) + 0xC,
 KEY_MSWHEEL_RIGHT = (0x00200000U + 0x00080000U) + 0xD,
 KEY_MSLCLICK = (0x00200000U + 0x00080000U) + 0xF,
 KEY_MSRCLICK = (0x00200000U + 0x00080000U) + 0x10,
 KEY_MSM1CLICK = (0x00200000U + 0x00080000U) + 0x11,
 KEY_MSM2CLICK = (0x00200000U + 0x00080000U) + 0x12,
 KEY_MSM3CLICK = (0x00200000U + 0x00080000U) + 0x13,
 KEY_VK_0xFF_BEGIN = 0x00200000U + 0x00000100,
 KEY_VK_0xFF_END = 0x00200000U + 0x000001FF,
 KEY_END_FKEY = 0x00200000U + 0x00080000U,
 KEY_NONE = (0x00200000U + 2 * 0x00080000U) + 1,
 KEY_IDLE = (0x00200000U + 2 * 0x00080000U) + 2,
 KEY_DRAGCOPY = (0x00200000U + 2 * 0x00080000U) + 3,
 KEY_DRAGMOVE = (0x00200000U + 2 * 0x00080000U) + 4,
 KEY_KILLFOCUS = (0x00200000U + 2 * 0x00080000U) + 6,
 KEY_GOTFOCUS = (0x00200000U + 2 * 0x00080000U) + 7,
 KEY_CONSOLE_BUFFER_RESIZE = (0x00200000U + 2 * 0x00080000U) + 8,
 KEY_OP_BASE = (0x00200000U + 2 * 0x00080000U) + 0x100,
 KEY_OP_XLAT = KEY_OP_BASE + 0,
 KEY_OP_PLAINTEXT = KEY_OP_BASE + 2,
 KEY_OP_SELWORD = KEY_OP_BASE + 3,
 KEY_OP_ENDBASE = (0x00200000U + 2 * 0x00080000U) + 0x1FF,
 KEY_END_SKEY = (0x00200000U + 2 * 0x00080000U) + (0x00080000U - 1),
 KEY_LAST_BASE = KEY_END_SKEY,
 KEY_MACRO_BASE = (0x00200000U + 3 * 0x00080000U),
 KEY_MACRO_OP_BASE = (0x00200000U + 3 * 0x00080000U) + 0x0000,
 KEY_MACRO_C_BASE = (0x00200000U + 3 * 0x00080000U) + 0x0400,
 KEY_MACRO_V_BASE = (0x00200000U + 3 * 0x00080000U) + 0x0800,
 KEY_MACRO_F_BASE = (0x00200000U + 3 * 0x00080000U) + 0x0C00,
 KEY_MACRO_U_BASE = (0x00200000U + 3 * 0x00080000U) + 0x8000,
 KEY_MACRO_ENDBASE = KEY_MACRO_BASE + (0x00080000U - 1),
 KEY_INVALID = -1
};
enum AddDefKeyboard
{
 KEY_CTRLSHIFT = KEY_CTRL | KEY_SHIFT,
 KEY_ALTSHIFT = KEY_ALT | KEY_SHIFT,
 KEY_CTRLALT = KEY_CTRL | KEY_ALT,
 KEY_CTRL0 = KEY_CTRL + '0',
 KEY_CTRL1 = KEY_CTRL + '1',
 KEY_CTRL2 = KEY_CTRL + '2',
 KEY_CTRL3 = KEY_CTRL + '3',
 KEY_CTRL4 = KEY_CTRL + '4',
 KEY_CTRL5 = KEY_CTRL + '5',
 KEY_CTRL6 = KEY_CTRL + '6',
 KEY_CTRL7 = KEY_CTRL + '7',
 KEY_CTRL8 = KEY_CTRL + '8',
 KEY_CTRL9 = KEY_CTRL + '9',
 KEY_RCTRL0 = KEY_RCTRL + '0',
 KEY_RCTRL1 = KEY_RCTRL + '1',
 KEY_RCTRL2 = KEY_RCTRL + '2',
 KEY_RCTRL3 = KEY_RCTRL + '3',
 KEY_RCTRL4 = KEY_RCTRL + '4',
 KEY_RCTRL5 = KEY_RCTRL + '5',
 KEY_RCTRL6 = KEY_RCTRL + '6',
 KEY_RCTRL7 = KEY_RCTRL + '7',
 KEY_RCTRL8 = KEY_RCTRL + '8',
 KEY_RCTRL9 = KEY_RCTRL + '9',
 KEY_CTRLA = KEY_CTRL + 'A',
 KEY_CTRLB = KEY_CTRL + 'B',
 KEY_CTRLC = KEY_CTRL + 'C',
 KEY_CTRLD = KEY_CTRL + 'D',
 KEY_CTRLE = KEY_CTRL + 'E',
 KEY_CTRLF = KEY_CTRL + 'F',
 KEY_CTRLG = KEY_CTRL + 'G',
 KEY_CTRLH = KEY_CTRL + 'H',
 KEY_CTRLI = KEY_CTRL + 'I',
 KEY_CTRLJ = KEY_CTRL + 'J',
 KEY_CTRLK = KEY_CTRL + 'K',
 KEY_CTRLL = KEY_CTRL + 'L',
 KEY_CTRLM = KEY_CTRL + 'M',
 KEY_CTRLN = KEY_CTRL + 'N',
 KEY_CTRLO = KEY_CTRL + 'O',
 KEY_CTRLP = KEY_CTRL + 'P',
 KEY_CTRLQ = KEY_CTRL + 'Q',
 KEY_CTRLR = KEY_CTRL + 'R',
 KEY_CTRLS = KEY_CTRL + 'S',
 KEY_CTRLT = KEY_CTRL + 'T',
 KEY_CTRLU = KEY_CTRL + 'U',
 KEY_CTRLV = KEY_CTRL + 'V',
 KEY_CTRLW = KEY_CTRL + 'W',
 KEY_CTRLX = KEY_CTRL + 'X',
 KEY_CTRLY = KEY_CTRL + 'Y',
 KEY_CTRLZ = KEY_CTRL + 'Z',
 KEY_CTRLBRACKET = KEY_CTRL | KEY_BRACKET,
 KEY_CTRLBACKBRACKET = KEY_CTRL | KEY_BACKBRACKET,
 KEY_CTRLCOMMA = KEY_CTRL | KEY_COMMA,
 KEY_CTRLQUOTE = KEY_CTRL | KEY_QUOTE,
 KEY_CTRLDOT = KEY_CTRL | KEY_DOT,
 KEY_ALT0 = KEY_ALT + '0',
 KEY_ALT1 = KEY_ALT + '1',
 KEY_ALT2 = KEY_ALT + '2',
 KEY_ALT3 = KEY_ALT + '3',
 KEY_ALT4 = KEY_ALT + '4',
 KEY_ALT5 = KEY_ALT + '5',
 KEY_ALT6 = KEY_ALT + '6',
 KEY_ALT7 = KEY_ALT + '7',
 KEY_ALT8 = KEY_ALT + '8',
 KEY_ALT9 = KEY_ALT + '9',
 KEY_ALTADD = KEY_ALT | KEY_ADD,
 KEY_ALTDOT = KEY_ALT | KEY_DOT,
 KEY_ALTCOMMA = KEY_ALT | KEY_COMMA,
 KEY_ALTMULTIPLY = KEY_ALT | KEY_MULTIPLY,
 KEY_ALTA = KEY_ALT + 'A',
 KEY_ALTB = KEY_ALT + 'B',
 KEY_ALTC = KEY_ALT + 'C',
 KEY_ALTD = KEY_ALT + 'D',
 KEY_ALTE = KEY_ALT + 'E',
 KEY_ALTF = KEY_ALT + 'F',
 KEY_ALTG = KEY_ALT + 'G',
 KEY_ALTH = KEY_ALT + 'H',
 KEY_ALTI = KEY_ALT + 'I',
 KEY_ALTJ = KEY_ALT + 'J',
 KEY_ALTK = KEY_ALT + 'K',
 KEY_ALTL = KEY_ALT + 'L',
 KEY_ALTM = KEY_ALT + 'M',
 KEY_ALTN = KEY_ALT + 'N',
 KEY_ALTO = KEY_ALT + 'O',
 KEY_ALTP = KEY_ALT + 'P',
 KEY_ALTQ = KEY_ALT + 'Q',
 KEY_ALTR = KEY_ALT + 'R',
 KEY_ALTS = KEY_ALT + 'S',
 KEY_ALTT = KEY_ALT + 'T',
 KEY_ALTU = KEY_ALT + 'U',
 KEY_ALTV = KEY_ALT + 'V',
 KEY_ALTW = KEY_ALT + 'W',
 KEY_ALTX = KEY_ALT + 'X',
 KEY_ALTY = KEY_ALT + 'Y',
 KEY_ALTZ = KEY_ALT + 'Z',
 KEY_CTRLSHIFTADD = KEY_CTRL | KEY_SHIFT | KEY_ADD,
 KEY_CTRLSHIFTSUBTRACT = KEY_CTRL | KEY_SHIFT | KEY_SUBTRACT,
 KEY_CTRLSHIFTDOT = KEY_CTRL | KEY_SHIFT | KEY_DOT,
 KEY_CTRLSHIFTSLASH = KEY_CTRL | KEY_SHIFT | KEY_SLASH,
 KEY_CTRLSHIFT0 = (KEY_CTRL | KEY_SHIFT) + '0',
 KEY_CTRLSHIFT1 = (KEY_CTRL | KEY_SHIFT) + '1',
 KEY_CTRLSHIFT2 = (KEY_CTRL | KEY_SHIFT) + '2',
 KEY_CTRLSHIFT3 = (KEY_CTRL | KEY_SHIFT) + '3',
 KEY_CTRLSHIFT4 = (KEY_CTRL | KEY_SHIFT) + '4',
 KEY_CTRLSHIFT5 = (KEY_CTRL | KEY_SHIFT) + '5',
 KEY_CTRLSHIFT6 = (KEY_CTRL | KEY_SHIFT) + '6',
 KEY_CTRLSHIFT7 = (KEY_CTRL | KEY_SHIFT) + '7',
 KEY_CTRLSHIFT8 = (KEY_CTRL | KEY_SHIFT) + '8',
 KEY_CTRLSHIFT9 = (KEY_CTRL | KEY_SHIFT) + '9',
 KEY_RCTRLSHIFT0 = KEY_RCTRL + KEY_SHIFT + '0',
 KEY_RCTRLSHIFT1 = KEY_RCTRL + KEY_SHIFT + '1',
 KEY_RCTRLSHIFT2 = KEY_RCTRL + KEY_SHIFT + '2',
 KEY_RCTRLSHIFT3 = KEY_RCTRL + KEY_SHIFT + '3',
 KEY_RCTRLSHIFT4 = KEY_RCTRL + KEY_SHIFT + '4',
 KEY_RCTRLSHIFT5 = KEY_RCTRL + KEY_SHIFT + '5',
 KEY_RCTRLSHIFT6 = KEY_RCTRL + KEY_SHIFT + '6',
 KEY_RCTRLSHIFT7 = KEY_RCTRL + KEY_SHIFT + '7',
 KEY_RCTRLSHIFT8 = KEY_RCTRL + KEY_SHIFT + '8',
 KEY_RCTRLSHIFT9 = KEY_RCTRL + KEY_SHIFT + '9',
 KEY_CTRLSHIFTA = (KEY_CTRL | KEY_SHIFT) + 'A',
 KEY_CTRLSHIFTB = (KEY_CTRL | KEY_SHIFT) + 'B',
 KEY_CTRLSHIFTC = (KEY_CTRL | KEY_SHIFT) + 'C',
 KEY_CTRLSHIFTD = (KEY_CTRL | KEY_SHIFT) + 'D',
 KEY_CTRLSHIFTE = (KEY_CTRL | KEY_SHIFT) + 'E',
 KEY_CTRLSHIFTF = (KEY_CTRL | KEY_SHIFT) + 'F',
 KEY_CTRLSHIFTG = (KEY_CTRL | KEY_SHIFT) + 'G',
 KEY_CTRLSHIFTH = (KEY_CTRL | KEY_SHIFT) + 'H',
 KEY_CTRLSHIFTI = (KEY_CTRL | KEY_SHIFT) + 'I',
 KEY_CTRLSHIFTJ = (KEY_CTRL | KEY_SHIFT) + 'J',
 KEY_CTRLSHIFTK = (KEY_CTRL | KEY_SHIFT) + 'K',
 KEY_CTRLSHIFTL = (KEY_CTRL | KEY_SHIFT) + 'L',
 KEY_CTRLSHIFTM = (KEY_CTRL | KEY_SHIFT) + 'M',
 KEY_CTRLSHIFTN = (KEY_CTRL | KEY_SHIFT) + 'N',
 KEY_CTRLSHIFTO = (KEY_CTRL | KEY_SHIFT) + 'O',
 KEY_CTRLSHIFTP = (KEY_CTRL | KEY_SHIFT) + 'P',
 KEY_CTRLSHIFTQ = (KEY_CTRL | KEY_SHIFT) + 'Q',
 KEY_CTRLSHIFTR = (KEY_CTRL | KEY_SHIFT) + 'R',
 KEY_CTRLSHIFTS = (KEY_CTRL | KEY_SHIFT) + 'S',
 KEY_CTRLSHIFTT = (KEY_CTRL | KEY_SHIFT) + 'T',
 KEY_CTRLSHIFTU = (KEY_CTRL | KEY_SHIFT) + 'U',
 KEY_CTRLSHIFTV = (KEY_CTRL | KEY_SHIFT) + 'V',
 KEY_CTRLSHIFTW = (KEY_CTRL | KEY_SHIFT) + 'W',
 KEY_CTRLSHIFTX = (KEY_CTRL | KEY_SHIFT) + 'X',
 KEY_CTRLSHIFTY = (KEY_CTRL | KEY_SHIFT) + 'Y',
 KEY_CTRLSHIFTZ = (KEY_CTRL | KEY_SHIFT) + 'Z',
 KEY_CTRLSHIFTBRACKET = KEY_CTRL | KEY_SHIFT | KEY_BRACKET,
 KEY_CTRLSHIFTBACKSLASH = KEY_CTRL | KEY_SHIFT | KEY_BACKSLASH,
 KEY_CTRLSHIFTBACKBRACKET = KEY_CTRL | KEY_SHIFT | KEY_BACKBRACKET,
 KEY_ALTSHIFT0 = (KEY_ALT | KEY_SHIFT) + '0',
 KEY_ALTSHIFT1 = (KEY_ALT | KEY_SHIFT) + '1',
 KEY_ALTSHIFT2 = (KEY_ALT | KEY_SHIFT) + '2',
 KEY_ALTSHIFT3 = (KEY_ALT | KEY_SHIFT) + '3',
 KEY_ALTSHIFT4 = (KEY_ALT | KEY_SHIFT) + '4',
 KEY_ALTSHIFT5 = (KEY_ALT | KEY_SHIFT) + '5',
 KEY_ALTSHIFT6 = (KEY_ALT | KEY_SHIFT) + '6',
 KEY_ALTSHIFT7 = (KEY_ALT | KEY_SHIFT) + '7',
 KEY_ALTSHIFT8 = (KEY_ALT | KEY_SHIFT) + '8',
 KEY_ALTSHIFT9 = (KEY_ALT | KEY_SHIFT) + '9',
 KEY_ALTSHIFTA = (KEY_ALT | KEY_SHIFT) + 'A',
 KEY_ALTSHIFTB = (KEY_ALT | KEY_SHIFT) + 'B',
 KEY_ALTSHIFTC = (KEY_ALT | KEY_SHIFT) + 'C',
 KEY_ALTSHIFTD = (KEY_ALT | KEY_SHIFT) + 'D',
 KEY_ALTSHIFTE = (KEY_ALT | KEY_SHIFT) + 'E',
 KEY_ALTSHIFTF = (KEY_ALT | KEY_SHIFT) + 'F',
 KEY_ALTSHIFTG = (KEY_ALT | KEY_SHIFT) + 'G',
 KEY_ALTSHIFTH = (KEY_ALT | KEY_SHIFT) + 'H',
 KEY_ALTSHIFTI = (KEY_ALT | KEY_SHIFT) + 'I',
 KEY_ALTSHIFTJ = (KEY_ALT | KEY_SHIFT) + 'J',
 KEY_ALTSHIFTK = (KEY_ALT | KEY_SHIFT) + 'K',
 KEY_ALTSHIFTL = (KEY_ALT | KEY_SHIFT) + 'L',
 KEY_ALTSHIFTM = (KEY_ALT | KEY_SHIFT) + 'M',
 KEY_ALTSHIFTN = (KEY_ALT | KEY_SHIFT) + 'N',
 KEY_ALTSHIFTO = (KEY_ALT | KEY_SHIFT) + 'O',
 KEY_ALTSHIFTP = (KEY_ALT | KEY_SHIFT) + 'P',
 KEY_ALTSHIFTQ = (KEY_ALT | KEY_SHIFT) + 'Q',
 KEY_ALTSHIFTR = (KEY_ALT | KEY_SHIFT) + 'R',
 KEY_ALTSHIFTS = (KEY_ALT | KEY_SHIFT) + 'S',
 KEY_ALTSHIFTT = (KEY_ALT | KEY_SHIFT) + 'T',
 KEY_ALTSHIFTU = (KEY_ALT | KEY_SHIFT) + 'U',
 KEY_ALTSHIFTV = (KEY_ALT | KEY_SHIFT) + 'V',
 KEY_ALTSHIFTW = (KEY_ALT | KEY_SHIFT) + 'W',
 KEY_ALTSHIFTX = (KEY_ALT | KEY_SHIFT) + 'X',
 KEY_ALTSHIFTY = (KEY_ALT | KEY_SHIFT) + 'Y',
 KEY_ALTSHIFTZ = (KEY_ALT | KEY_SHIFT) + 'Z',
 KEY_ALTSHIFTBRACKET = KEY_ALT | KEY_SHIFT | KEY_BRACKET,
 KEY_ALTSHIFTBACKBRACKET = KEY_ALT | KEY_SHIFT | KEY_BACKBRACKET,
 KEY_CTRLALT0 = (KEY_CTRL | KEY_ALT) + '0',
 KEY_CTRLALT1 = (KEY_CTRL | KEY_ALT) + '1',
 KEY_CTRLALT2 = (KEY_CTRL | KEY_ALT) + '2',
 KEY_CTRLALT3 = (KEY_CTRL | KEY_ALT) + '3',
 KEY_CTRLALT4 = (KEY_CTRL | KEY_ALT) + '4',
 KEY_CTRLALT5 = (KEY_CTRL | KEY_ALT) + '5',
 KEY_CTRLALT6 = (KEY_CTRL | KEY_ALT) + '6',
 KEY_CTRLALT7 = (KEY_CTRL | KEY_ALT) + '7',
 KEY_CTRLALT8 = (KEY_CTRL | KEY_ALT) + '8',
 KEY_CTRLALT9 = (KEY_CTRL | KEY_ALT) + '9',
 KEY_CTRLALTA = (KEY_CTRL | KEY_ALT) + 'A',
 KEY_CTRLALTB = (KEY_CTRL | KEY_ALT) + 'B',
 KEY_CTRLALTC = (KEY_CTRL | KEY_ALT) + 'C',
 KEY_CTRLALTD = (KEY_CTRL | KEY_ALT) + 'D',
 KEY_CTRLALTE = (KEY_CTRL | KEY_ALT) + 'E',
 KEY_CTRLALTF = (KEY_CTRL | KEY_ALT) + 'F',
 KEY_CTRLALTG = (KEY_CTRL | KEY_ALT) + 'G',
 KEY_CTRLALTH = (KEY_CTRL | KEY_ALT) + 'H',
 KEY_CTRLALTI = (KEY_CTRL | KEY_ALT) + 'I',
 KEY_CTRLALTJ = (KEY_CTRL | KEY_ALT) + 'J',
 KEY_CTRLALTK = (KEY_CTRL | KEY_ALT) + 'K',
 KEY_CTRLALTL = (KEY_CTRL | KEY_ALT) + 'L',
 KEY_CTRLALTM = (KEY_CTRL | KEY_ALT) + 'M',
 KEY_CTRLALTN = (KEY_CTRL | KEY_ALT) + 'N',
 KEY_CTRLALTO = (KEY_CTRL | KEY_ALT) + 'O',
 KEY_CTRLALTP = (KEY_CTRL | KEY_ALT) + 'P',
 KEY_CTRLALTQ = (KEY_CTRL | KEY_ALT) + 'Q',
 KEY_CTRLALTR = (KEY_CTRL | KEY_ALT) + 'R',
 KEY_CTRLALTS = (KEY_CTRL | KEY_ALT) + 'S',
 KEY_CTRLALTT = (KEY_CTRL | KEY_ALT) + 'T',
 KEY_CTRLALTU = (KEY_CTRL | KEY_ALT) + 'U',
 KEY_CTRLALTV = (KEY_CTRL | KEY_ALT) + 'V',
 KEY_CTRLALTW = (KEY_CTRL | KEY_ALT) + 'W',
 KEY_CTRLALTX = (KEY_CTRL | KEY_ALT) + 'X',
 KEY_CTRLALTY = (KEY_CTRL | KEY_ALT) + 'Y',
 KEY_CTRLALTZ = (KEY_CTRL | KEY_ALT) + 'Z',
 KEY_CTRLALTBRACKET = KEY_CTRL | KEY_ALT | KEY_BRACKET,
 KEY_CTRLALTBACKBRACKET = KEY_CTRL | KEY_ALT | KEY_BACKBRACKET,
 KEY_CTRLF1 = KEY_CTRL | KEY_F1,
 KEY_CTRLF2 = KEY_CTRL | KEY_F2,
 KEY_CTRLF3 = KEY_CTRL | KEY_F3,
 KEY_CTRLF4 = KEY_CTRL | KEY_F4,
 KEY_CTRLF5 = KEY_CTRL | KEY_F5,
 KEY_CTRLF6 = KEY_CTRL | KEY_F6,
 KEY_CTRLF7 = KEY_CTRL | KEY_F7,
 KEY_CTRLF8 = KEY_CTRL | KEY_F8,
 KEY_CTRLF9 = KEY_CTRL | KEY_F9,
 KEY_CTRLF10 = KEY_CTRL | KEY_F10,
 KEY_CTRLF11 = KEY_CTRL | KEY_F11,
 KEY_CTRLF12 = KEY_CTRL | KEY_F12,
 KEY_SHIFTF1 = KEY_SHIFT | KEY_F1,
 KEY_SHIFTF2 = KEY_SHIFT | KEY_F2,
 KEY_SHIFTF3 = KEY_SHIFT | KEY_F3,
 KEY_SHIFTF4 = KEY_SHIFT | KEY_F4,
 KEY_SHIFTF5 = KEY_SHIFT | KEY_F5,
 KEY_SHIFTF6 = KEY_SHIFT | KEY_F6,
 KEY_SHIFTF7 = KEY_SHIFT | KEY_F7,
 KEY_SHIFTF8 = KEY_SHIFT | KEY_F8,
 KEY_SHIFTF9 = KEY_SHIFT | KEY_F9,
 KEY_SHIFTF10 = KEY_SHIFT | KEY_F10,
 KEY_SHIFTF11 = KEY_SHIFT | KEY_F11,
 KEY_SHIFTF12 = KEY_SHIFT | KEY_F12,
 KEY_ALTF1 = KEY_ALT | KEY_F1,
 KEY_ALTF2 = KEY_ALT | KEY_F2,
 KEY_ALTF3 = KEY_ALT | KEY_F3,
 KEY_ALTF4 = KEY_ALT | KEY_F4,
 KEY_ALTF5 = KEY_ALT | KEY_F5,
 KEY_ALTF6 = KEY_ALT | KEY_F6,
 KEY_ALTF7 = KEY_ALT | KEY_F7,
 KEY_ALTF8 = KEY_ALT | KEY_F8,
 KEY_ALTF9 = KEY_ALT | KEY_F9,
 KEY_ALTF10 = KEY_ALT | KEY_F10,
 KEY_ALTF11 = KEY_ALT | KEY_F11,
 KEY_ALTF12 = KEY_ALT | KEY_F12,
 KEY_CTRLSHIFTF1 = KEY_CTRL | KEY_SHIFT | KEY_F1,
 KEY_CTRLSHIFTF2 = KEY_CTRL | KEY_SHIFT | KEY_F2,
 KEY_CTRLSHIFTF3 = KEY_CTRL | KEY_SHIFT | KEY_F3,
 KEY_CTRLSHIFTF4 = KEY_CTRL | KEY_SHIFT | KEY_F4,
 KEY_CTRLSHIFTF5 = KEY_CTRL | KEY_SHIFT | KEY_F5,
 KEY_CTRLSHIFTF6 = KEY_CTRL | KEY_SHIFT | KEY_F6,
 KEY_CTRLSHIFTF7 = KEY_CTRL | KEY_SHIFT | KEY_F7,
 KEY_CTRLSHIFTF8 = KEY_CTRL | KEY_SHIFT | KEY_F8,
 KEY_CTRLSHIFTF9 = KEY_CTRL | KEY_SHIFT | KEY_F9,
 KEY_CTRLSHIFTF10 = KEY_CTRL | KEY_SHIFT | KEY_F10,
 KEY_CTRLSHIFTF11 = KEY_CTRL | KEY_SHIFT | KEY_F11,
 KEY_CTRLSHIFTF12 = KEY_CTRL | KEY_SHIFT | KEY_F12,
 KEY_ALTSHIFTF1 = KEY_ALT | KEY_SHIFT | KEY_F1,
 KEY_ALTSHIFTF2 = KEY_ALT | KEY_SHIFT | KEY_F2,
 KEY_ALTSHIFTF3 = KEY_ALT | KEY_SHIFT | KEY_F3,
 KEY_ALTSHIFTF4 = KEY_ALT | KEY_SHIFT | KEY_F4,
 KEY_ALTSHIFTF5 = KEY_ALT | KEY_SHIFT | KEY_F5,
 KEY_ALTSHIFTF6 = KEY_ALT | KEY_SHIFT | KEY_F6,
 KEY_ALTSHIFTF7 = KEY_ALT | KEY_SHIFT | KEY_F7,
 KEY_ALTSHIFTF8 = KEY_ALT | KEY_SHIFT | KEY_F8,
 KEY_ALTSHIFTF9 = KEY_ALT | KEY_SHIFT | KEY_F9,
 KEY_ALTSHIFTF10 = KEY_ALT | KEY_SHIFT | KEY_F10,
 KEY_ALTSHIFTF11 = KEY_ALT | KEY_SHIFT | KEY_F11,
 KEY_ALTSHIFTF12 = KEY_ALT | KEY_SHIFT | KEY_F12,
 KEY_CTRLALTF1 = KEY_CTRL | KEY_ALT | KEY_F1,
 KEY_CTRLALTF2 = KEY_CTRL | KEY_ALT | KEY_F2,
 KEY_CTRLALTF3 = KEY_CTRL | KEY_ALT | KEY_F3,
 KEY_CTRLALTF4 = KEY_CTRL | KEY_ALT | KEY_F4,
 KEY_CTRLALTF5 = KEY_CTRL | KEY_ALT | KEY_F5,
 KEY_CTRLALTF6 = KEY_CTRL | KEY_ALT | KEY_F6,
 KEY_CTRLALTF7 = KEY_CTRL | KEY_ALT | KEY_F7,
 KEY_CTRLALTF8 = KEY_CTRL | KEY_ALT | KEY_F8,
 KEY_CTRLALTF9 = KEY_CTRL | KEY_ALT | KEY_F9,
 KEY_CTRLALTF10 = KEY_CTRL | KEY_ALT | KEY_F10,
 KEY_CTRLALTF11 = KEY_CTRL | KEY_ALT | KEY_F11,
 KEY_CTRLALTF12 = KEY_CTRL | KEY_ALT | KEY_F12,
 KEY_CTRLHOME = KEY_CTRL | KEY_HOME,
 KEY_CTRLUP = KEY_CTRL | KEY_UP,
 KEY_CTRLPGUP = KEY_CTRL | KEY_PGUP,
 KEY_CTRLLEFT = KEY_CTRL | KEY_LEFT,
 KEY_CTRLRIGHT = KEY_CTRL | KEY_RIGHT,
 KEY_CTRLEND = KEY_CTRL | KEY_END,
 KEY_CTRLDOWN = KEY_CTRL | KEY_DOWN,
 KEY_CTRLPGDN = KEY_CTRL | KEY_PGDN,
 KEY_CTRLINS = KEY_CTRL | KEY_INS,
 KEY_CTRLDEL = KEY_CTRL | KEY_DEL,
 KEY_CTRLNUMDEL = KEY_CTRL | KEY_NUMDEL,
 KEY_CTRLDECIMAL = KEY_CTRL | KEY_DECIMAL,
 KEY_SHIFTHOME = KEY_SHIFT | KEY_HOME,
 KEY_SHIFTUP = KEY_SHIFT | KEY_UP,
 KEY_SHIFTPGUP = KEY_SHIFT | KEY_PGUP,
 KEY_SHIFTLEFT = KEY_SHIFT | KEY_LEFT,
 KEY_SHIFTRIGHT = KEY_SHIFT | KEY_RIGHT,
 KEY_SHIFTEND = KEY_SHIFT | KEY_END,
 KEY_SHIFTDOWN = KEY_SHIFT | KEY_DOWN,
 KEY_SHIFTPGDN = KEY_SHIFT | KEY_PGDN,
 KEY_SHIFTINS = KEY_SHIFT | KEY_INS,
 KEY_SHIFTDEL = KEY_SHIFT | KEY_DEL,
 KEY_SHIFTNUMDEL = KEY_SHIFT | KEY_NUMDEL,
 KEY_SHIFTDECIMAL = KEY_SHIFT | KEY_DECIMAL,
 KEY_ALTHOME = KEY_ALT | KEY_HOME,
 KEY_ALTUP = KEY_ALT | KEY_UP,
 KEY_ALTPGUP = KEY_ALT | KEY_PGUP,
 KEY_ALTLEFT = KEY_ALT | KEY_LEFT,
 KEY_ALTRIGHT = KEY_ALT | KEY_RIGHT,
 KEY_ALTEND = KEY_ALT | KEY_END,
 KEY_ALTDOWN = KEY_ALT | KEY_DOWN,
 KEY_ALTPGDN = KEY_ALT | KEY_PGDN,
 KEY_ALTINS = KEY_ALT | KEY_INS,
 KEY_ALTDEL = KEY_ALT | KEY_DEL,
 KEY_ALTNUMDEL = KEY_ALT | KEY_NUMDEL,
 KEY_ALTDECIMAL = KEY_ALT | KEY_DECIMAL,
 KEY_CTRLSHIFTHOME = KEY_CTRL | KEY_SHIFT | KEY_HOME,
 KEY_CTRLSHIFTUP = KEY_CTRL | KEY_SHIFT | KEY_UP,
 KEY_CTRLSHIFTPGUP = KEY_CTRL | KEY_SHIFT | KEY_PGUP,
 KEY_CTRLSHIFTLEFT = KEY_CTRL | KEY_SHIFT | KEY_LEFT,
 KEY_CTRLSHIFTRIGHT = KEY_CTRL | KEY_SHIFT | KEY_RIGHT,
 KEY_CTRLSHIFTEND = KEY_CTRL | KEY_SHIFT | KEY_END,
 KEY_CTRLSHIFTDOWN = KEY_CTRL | KEY_SHIFT | KEY_DOWN,
 KEY_CTRLSHIFTPGDN = KEY_CTRL | KEY_SHIFT | KEY_PGDN,
 KEY_CTRLSHIFTINS = KEY_CTRL | KEY_SHIFT | KEY_INS,
 KEY_CTRLSHIFTDEL = KEY_CTRL | KEY_SHIFT | KEY_DEL,
 KEY_CTRLSHIFTNUMDEL = KEY_CTRL | KEY_SHIFT | KEY_NUMDEL,
 KEY_CTRLSHIFTDECIMAL = KEY_CTRL | KEY_SHIFT | KEY_DECIMAL,
 KEY_ALTSHIFTHOME = KEY_ALT | KEY_SHIFT | KEY_HOME,
 KEY_ALTSHIFTUP = KEY_ALT | KEY_SHIFT | KEY_UP,
 KEY_ALTSHIFTPGUP = KEY_ALT | KEY_SHIFT | KEY_PGUP,
 KEY_ALTSHIFTLEFT = KEY_ALT | KEY_SHIFT | KEY_LEFT,
 KEY_ALTSHIFTRIGHT = KEY_ALT | KEY_SHIFT | KEY_RIGHT,
 KEY_ALTSHIFTEND = KEY_ALT | KEY_SHIFT | KEY_END,
 KEY_ALTSHIFTDOWN = KEY_ALT | KEY_SHIFT | KEY_DOWN,
 KEY_ALTSHIFTPGDN = KEY_ALT | KEY_SHIFT | KEY_PGDN,
 KEY_ALTSHIFTINS = KEY_ALT | KEY_SHIFT | KEY_INS,
 KEY_ALTSHIFTDEL = KEY_ALT | KEY_SHIFT | KEY_DEL,
 KEY_ALTSHIFTNUMDEL = KEY_ALT | KEY_SHIFT | KEY_NUMDEL,
 KEY_ALTSHIFTDECIMAL = KEY_ALT | KEY_SHIFT | KEY_DECIMAL,
 KEY_CTRLALTHOME = KEY_CTRL | KEY_ALT | KEY_HOME,
 KEY_CTRLALTUP = KEY_CTRL | KEY_ALT | KEY_UP,
 KEY_CTRLALTPGUP = KEY_CTRL | KEY_ALT | KEY_PGUP,
 KEY_CTRLALTLEFT = KEY_CTRL | KEY_ALT | KEY_LEFT,
 KEY_CTRLALTRIGHT = KEY_CTRL | KEY_ALT | KEY_RIGHT,
 KEY_CTRLALTEND = KEY_CTRL | KEY_ALT | KEY_END,
 KEY_CTRLALTDOWN = KEY_CTRL | KEY_ALT | KEY_DOWN,
 KEY_CTRLALTPGDN = KEY_CTRL | KEY_ALT | KEY_PGDN,
 KEY_CTRLALTINS = KEY_CTRL | KEY_ALT | KEY_INS,
 KEY_CTRLNUMPAD0 = KEY_CTRL | KEY_NUMPAD0,
 KEY_CTRLNUMPAD1 = KEY_CTRL | KEY_NUMPAD1,
 KEY_CTRLNUMPAD2 = KEY_CTRL | KEY_NUMPAD2,
 KEY_CTRLNUMPAD3 = KEY_CTRL | KEY_NUMPAD3,
 KEY_CTRLNUMPAD4 = KEY_CTRL | KEY_NUMPAD4,
 KEY_CTRLNUMPAD5 = KEY_CTRL | KEY_NUMPAD5,
 KEY_CTRLNUMPAD6 = KEY_CTRL | KEY_NUMPAD6,
 KEY_CTRLNUMPAD7 = KEY_CTRL | KEY_NUMPAD7,
 KEY_CTRLNUMPAD8 = KEY_CTRL | KEY_NUMPAD8,
 KEY_CTRLNUMPAD9 = KEY_CTRL | KEY_NUMPAD9,
 KEY_SHIFTNUMPAD0 = KEY_SHIFT | KEY_NUMPAD0,
 KEY_SHIFTNUMPAD1 = KEY_SHIFT | KEY_NUMPAD1,
 KEY_SHIFTNUMPAD2 = KEY_SHIFT | KEY_NUMPAD2,
 KEY_SHIFTNUMPAD3 = KEY_SHIFT | KEY_NUMPAD3,
 KEY_SHIFTNUMPAD4 = KEY_SHIFT | KEY_NUMPAD4,
 KEY_SHIFTNUMPAD5 = KEY_SHIFT | KEY_NUMPAD5,
 KEY_SHIFTNUMPAD6 = KEY_SHIFT | KEY_NUMPAD6,
 KEY_SHIFTNUMPAD7 = KEY_SHIFT | KEY_NUMPAD7,
 KEY_SHIFTNUMPAD8 = KEY_SHIFT | KEY_NUMPAD8,
 KEY_SHIFTNUMPAD9 = KEY_SHIFT | KEY_NUMPAD9,
 KEY_CTRLSHIFTNUMPAD0 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD0,
 KEY_CTRLSHIFTNUMPAD1 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD1,
 KEY_CTRLSHIFTNUMPAD2 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD2,
 KEY_CTRLSHIFTNUMPAD3 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD3,
 KEY_CTRLSHIFTNUMPAD4 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD4,
 KEY_CTRLSHIFTNUMPAD5 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD5,
 KEY_CTRLSHIFTNUMPAD6 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD6,
 KEY_CTRLSHIFTNUMPAD7 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD7,
 KEY_CTRLSHIFTNUMPAD8 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD8,
 KEY_CTRLSHIFTNUMPAD9 = KEY_CTRL | KEY_SHIFT | KEY_NUMPAD9,
 KEY_CTRLALTNUMPAD0 = KEY_CTRL | KEY_ALT | KEY_NUMPAD0,
 KEY_CTRLALTNUMPAD1 = KEY_CTRL | KEY_ALT | KEY_NUMPAD1,
 KEY_CTRLALTNUMPAD2 = KEY_CTRL | KEY_ALT | KEY_NUMPAD2,
 KEY_CTRLALTNUMPAD3 = KEY_CTRL | KEY_ALT | KEY_NUMPAD3,
 KEY_CTRLALTNUMPAD4 = KEY_CTRL | KEY_ALT | KEY_NUMPAD4,
 KEY_CTRLALTNUMPAD5 = KEY_CTRL | KEY_ALT | KEY_NUMPAD5,
 KEY_CTRLALTNUMPAD6 = KEY_CTRL | KEY_ALT | KEY_NUMPAD6,
 KEY_CTRLALTNUMPAD7 = KEY_CTRL | KEY_ALT | KEY_NUMPAD7,
 KEY_CTRLALTNUMPAD8 = KEY_CTRL | KEY_ALT | KEY_NUMPAD8,
 KEY_CTRLALTNUMPAD9 = KEY_CTRL | KEY_ALT | KEY_NUMPAD9,
 KEY_ALTSHIFTNUMPAD0 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD0,
 KEY_ALTSHIFTNUMPAD1 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD1,
 KEY_ALTSHIFTNUMPAD2 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD2,
 KEY_ALTSHIFTNUMPAD3 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD3,
 KEY_ALTSHIFTNUMPAD4 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD4,
 KEY_ALTSHIFTNUMPAD5 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD5,
 KEY_ALTSHIFTNUMPAD6 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD6,
 KEY_ALTSHIFTNUMPAD7 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD7,
 KEY_ALTSHIFTNUMPAD8 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD8,
 KEY_ALTSHIFTNUMPAD9 = KEY_ALT | KEY_SHIFT | KEY_NUMPAD9,
 KEY_CTRLSLASH = KEY_CTRL | KEY_SLASH,
 KEY_CTRLBACKSLASH = KEY_CTRL | KEY_BACKSLASH,
 KEY_CTRLCLEAR = KEY_CTRL | KEY_CLEAR,
 KEY_CTRLSHIFTCLEAR = KEY_CTRL | KEY_SHIFT | KEY_CLEAR,
 KEY_CTRLALTCLEAR = KEY_CTRL | KEY_ALT | KEY_CLEAR,
 KEY_CTRLADD = KEY_CTRL | KEY_ADD,
 KEY_SHIFTADD = KEY_SHIFT | KEY_ADD,
 KEY_CTRLSUBTRACT = KEY_CTRL | KEY_SUBTRACT,
 KEY_ALTSUBTRACT = KEY_ALT | KEY_SUBTRACT,
 KEY_SHIFTSUBTRACT = KEY_SHIFT | KEY_SUBTRACT,
 KEY_CTRLMULTIPLY = KEY_CTRL | KEY_MULTIPLY,
 KEY_CTRLBS = KEY_CTRL | KEY_BS,
 KEY_ALTBS = KEY_ALT | KEY_BS,
 KEY_CTRLSHIFTBS = KEY_CTRL | KEY_SHIFT | KEY_BS,
 KEY_SHIFTBS = KEY_SHIFT | KEY_BS,
 KEY_CTRLSHIFTTAB = KEY_CTRL | KEY_SHIFT | KEY_TAB,
 KEY_CTRLTAB = KEY_CTRL | KEY_TAB,
 KEY_SHIFTTAB = KEY_SHIFT | KEY_TAB,
 KEY_CTRLENTER = KEY_CTRL | KEY_ENTER,
 KEY_SHIFTENTER = KEY_SHIFT | KEY_ENTER,
 KEY_ALTSHIFTENTER = KEY_ALT | KEY_SHIFT | KEY_ENTER,
 KEY_CTRLALTENTER = KEY_CTRL | KEY_ALT | KEY_ENTER,
 KEY_CTRLSHIFTENTER = KEY_CTRL | KEY_SHIFT | KEY_ENTER,
 KEY_CTRLNUMENTER = KEY_CTRL | KEY_NUMENTER,
 KEY_SHIFTNUMENTER = KEY_SHIFT | KEY_NUMENTER,
 KEY_ALTSHIFTNUMENTER = KEY_ALT | KEY_SHIFT | KEY_NUMENTER,
 KEY_CTRLALTNUMENTER = KEY_CTRL | KEY_ALT | KEY_NUMENTER,
 KEY_CTRLSHIFTNUMENTER = KEY_CTRL | KEY_SHIFT | KEY_NUMENTER,
 KEY_CTRLAPPS = KEY_CTRL | KEY_APPS,
 KEY_ALTAPPS = KEY_ALT | KEY_APPS,
 KEY_SHIFTAPPS = KEY_SHIFT | KEY_APPS,
 KEY_CTRLSHIFTAPPS = KEY_CTRL | KEY_SHIFT | KEY_APPS,
 KEY_ALTSHIFTAPPS = KEY_ALT | KEY_SHIFT | KEY_APPS,
 KEY_CTRLALTAPPS = KEY_CTRL | KEY_ALT | KEY_APPS,
 KEY_CTRLSPACE = KEY_CTRL | KEY_SPACE,
 KEY_SHIFTSPACE = KEY_SHIFT | KEY_SPACE,
 KEY_CTRLSHIFTSPACE = KEY_CTRL | KEY_SHIFT | KEY_SPACE,
 KEY_ALT_BASE = KEY_ALT,
 KEY_ALTSHIFT_BASE = KEY_ALTSHIFT
};
 typedef HANDLE (*WINPORT_ForkConsole) (HANDLE hParentConsole);
 typedef VOID (*WINPORT_JoinConsole) (HANDLE hParentConsole, HANDLE hConsole);
 typedef VOID (*WINPORT_DiscardConsole) (HANDLE hConsole);
 typedef COORD (*WINPORT_GetLargestConsoleWindowSize) (HANDLE hConsoleOutput);
 typedef BOOL (*WINPORT_SetConsoleWindowInfo) (HANDLE hConsoleOutput, BOOL bAbsolute, const SMALL_RECT *lpConsoleWindow);
 typedef BOOL (*WINPORT_SetConsoleTitle) (HANDLE hConsoleOutput, const WCHAR *title);
 typedef DWORD (*WINPORT_GetConsoleTitle) (HANDLE hConsoleOutput, WCHAR *title, DWORD max_size);
 typedef BOOL (*WINPORT_SetConsoleScreenBufferSize) (HANDLE hConsoleOutput,COORD dwSize);
 typedef BOOL (*WINPORT_GetConsoleScreenBufferInfo) (HANDLE hConsoleOutput,CONSOLE_SCREEN_BUFFER_INFO *lpConsoleScreenBufferInfo);
 typedef BOOL (*WINPORT_SetConsoleCursorPosition) (HANDLE hConsoleOutput,COORD dwCursorPosition);
 typedef BOOL (*WINPORT_SetConsoleCursorInfo) (HANDLE hConsoleOutput,const CONSOLE_CURSOR_INFO *lpConsoleCursorInfon);
 typedef BOOL (*WINPORT_GetConsoleCursorInfo) (HANDLE hConsoleOutput,CONSOLE_CURSOR_INFO *lpConsoleCursorInfon);
 typedef BOOL (*WINPORT_GetConsoleMode) (HANDLE hConsoleHandle,LPDWORD lpMode);
 typedef BOOL (*WINPORT_SetConsoleMode) (HANDLE hConsoleHandle, DWORD dwMode);
 typedef BOOL (*WINPORT_ScrollConsoleScreenBuffer) (HANDLE hConsoleOutput, const SMALL_RECT *lpScrollRectangle, const SMALL_RECT *lpClipRectangle, COORD dwDestinationOrigin, const CHAR_INFO *lpFill);
 typedef BOOL (*WINPORT_SetConsoleTextAttribute) (HANDLE hConsoleOutput, DWORD64 qAttributes);
 typedef COMP_CHAR (*WINPORT_CompositeCharRegister) (const WCHAR *lpSequence);
 typedef const WCHAR * (*WINPORT_CompositeCharLookup) (COMP_CHAR CompositeChar);
 typedef BOOL (*WINPORT_WriteConsole) (HANDLE hConsoleOutput, const WCHAR *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID lpReserved);
 typedef BOOL (*WINPORT_WriteConsoleOutput) (HANDLE hConsoleOutput,const CHAR_INFO *lpBuffer,COORD dwBufferSize,COORD dwBufferCoord,PSMALL_RECT lpScreenRegion);
 typedef BOOL (*WINPORT_WriteConsoleOutputCharacter) (HANDLE hConsoleOutput, const WCHAR *lpCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
 typedef BOOL (*WINPORT_WaitConsoleInput) (HANDLE hConsoleInput, DWORD dwTimeout);
 typedef BOOL (*WINPORT_ReadConsoleOutput) (HANDLE hConsoleOutput, CHAR_INFO *lpBuffer, COORD dwBufferSize, COORD dwBufferCoord, PSMALL_RECT lpScreenRegion);
 typedef BOOL (*WINPORT_FillConsoleOutputAttribute) (HANDLE hConsoleOutput, DWORD64 qAttributes, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfAttrsWritten);
 typedef BOOL (*WINPORT_FillConsoleOutputCharacter) (HANDLE hConsoleOutput, WCHAR cCharacter, DWORD nLength, COORD dwWriteCoord, LPDWORD lpNumberOfCharsWritten);
 typedef BOOL (*WINPORT_SetConsoleActiveScreenBuffer) (HANDLE hConsoleOutput);
 typedef VOID (*WINPORT_SetConsoleCursorBlinkTime) (HANDLE hConsoleOutput, DWORD dwMilliseconds );
 typedef BOOL (*WINPORT_FlushConsoleInputBuffer) (HANDLE hConsoleInput);
 typedef BOOL (*WINPORT_GetNumberOfConsoleInputEvents) (HANDLE hConsoleInput, LPDWORD lpcNumberOfEvents);
 typedef BOOL (*WINPORT_PeekConsoleInput) (HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
 typedef BOOL (*WINPORT_ReadConsoleInput) (HANDLE hConsoleInput, PINPUT_RECORD lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsRead);
 typedef DWORD (*WINPORT_ReadConsoleInputBacktrace) (HANDLE hConsoleInput, CHAR *lpBuffer, DWORD nLength);
 typedef BOOL (*WINPORT_WriteConsoleInput) (HANDLE hConsoleInput, const INPUT_RECORD *lpBuffer, DWORD nLength, LPDWORD lpNumberOfEventsWritten);
 typedef DWORD (*WINPORT_CheckForKeyPress) (HANDLE hConsoleInput, const WORD *KeyCodes, DWORD KeyCodesCount, DWORD Flags);
 typedef BOOL (*WINPORT_SetConsoleDisplayMode) (DWORD ModeFlags);
 typedef BOOL (*WINPORT_GetConsoleDisplayMode) (LPDWORD lpModeFlags);
 typedef VOID (*WINPORT_SetConsoleWindowMaximized) (BOOL Maximized);
 typedef BYTE (*WINPORT_GetConsoleColorPalette) (HANDLE hConsoleOutput);
 typedef VOID (*WINPORT_GetConsoleBasePalette) (HANDLE hConsoleOutput, void *p);
 typedef BOOL (*WINPORT_SetConsoleBasePalette) (HANDLE hConsoleOutput, void *p);
 typedef BOOL (*WINPORT_GenerateConsoleCtrlEvent) (DWORD dwCtrlEvent, DWORD dwProcessGroupId );
 typedef BOOL (*WINPORT_SetConsoleCtrlHandler) (PHANDLER_ROUTINE HandlerRoutine, BOOL Add );
 typedef VOID (*WINPORT_SetConsoleScrollRegion) (HANDLE hConsoleOutput, SHORT top, SHORT bottom);
 typedef VOID (*WINPORT_GetConsoleScrollRegion) (HANDLE hConsoleOutput, SHORT *top, SHORT *bottom);
 typedef VOID (*WINPORT_SetConsoleScrollCallback) (HANDLE hConsoleOutput, PCONSOLE_SCROLL_CALLBACK pCallback, PVOID pContext);
 typedef BOOL (*WINPORT_BeginConsoleAdhocQuickEdit) ();
 typedef DWORD64 (*WINPORT_SetConsoleTweaks) (DWORD64 tweaks);
 typedef VOID (*WINPORT_SaveConsoleWindowState) ();
 typedef VOID (*WINPORT_ConsoleChangeFont) ();
 typedef BOOL (*WINPORT_IsConsoleActive) ();
 typedef VOID (*WINPORT_ConsoleDisplayNotification) (const WCHAR *title, const WCHAR *text);
 typedef BOOL (*WINPORT_ConsoleBackgroundMode) (BOOL TryEnterBackgroundMode);
 typedef BOOL (*WINPORT_SetConsoleFKeyTitles) (HANDLE hConsoleOutput, const CHAR **titles);
 typedef VOID (*WINPORT_OverrideConsoleColor) (HANDLE hConsoleOutput, DWORD Index, DWORD *ColorFG, DWORD *ColorBK);
 typedef VOID (*WINPORT_SetConsoleRepaintsDefer) (HANDLE hConsoleOutput, BOOL Deferring);
 typedef BOOL (*WINPORT_GetConsoleImageCaps) (HANDLE hConsoleOutput, size_t sizeof_wgi, WinportGraphicsInfo *wgi);
 typedef BOOL (*WINPORT_SetConsoleImage) (HANDLE hConsoleOutput, const char *id, DWORD64 flags, const SMALL_RECT *area, DWORD width, DWORD height, const void *buffer);
 typedef BOOL (*WINPORT_TransformConsoleImage) (HANDLE hConsoleOutput, const char *id, const SMALL_RECT *area, uint16_t tf);
 typedef BOOL (*WINPORT_DeleteConsoleImage) (HANDLE hConsoleOutput, const char *id);
 typedef LONG (*WINPORT_RegOpenKeyEx) (HKEY hKey,LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult);
 typedef LONG (*WINPORT_RegCreateKeyEx) (HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);
 typedef LONG (*WINPORT_RegCloseKey) (HKEY hKey);
 typedef LONG (*WINPORT_RegDeleteKey) (HKEY hKey, LPCWSTR lpSubKey);
 typedef LONG (*WINPORT_RegDeleteValue) (HKEY hKey, LPCWSTR lpValueName);
 typedef LONG (*WINPORT_RegSetValueEx) (HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE *lpData, DWORD cbData);
 typedef LONG (*WINPORT_RegEnumKeyEx) (HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcName, LPDWORD lpReserved, LPWSTR lpClass,LPDWORD lpcClass, PFILETIME lpftLastWriteTime);
 typedef LONG (*WINPORT_RegEnumKey) (HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cchName);
 typedef LONG (*WINPORT_RegEnumValue) (HKEY hKey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcchValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
 typedef LONG (*WINPORT_RegQueryValueEx) (HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
 typedef LONG (*WINPORT_RegQueryInfoKey) (HKEY hKey, LPWSTR lpClass, LPDWORD lpcClass, LPDWORD lpReserved, LPDWORD lpcSubKeys, LPDWORD lpcMaxSubKeyLen, LPDWORD lpcMaxClassLen, LPDWORD lpcValues, LPDWORD lpcMaxValueNameLen, LPDWORD lpcMaxValueLen, LPDWORD lpcbSecurityDescriptor, PFILETIME lpftLastWriteTime);
 typedef VOID (*WINPORT_RegWipeBegin) ();
 typedef VOID (*WINPORT_RegWipeEnd) ();
 typedef DWORD (*WINPORT_GetLastError) ();
 typedef VOID (*WINPORT_SetLastError) (DWORD code);
 typedef DWORD (*WINPORT_GetCurrentProcessId) ();
 typedef DWORD (*WINPORT_GetDoubleClickTime) ();
 typedef BOOL (*WINPORT_CreateDirectory) (LPCWSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes );
 typedef BOOL (*WINPORT_RemoveDirectory) ( LPCWSTR lpDirName);
 typedef BOOL (*WINPORT_DeleteFile) ( LPCWSTR lpFileName);
 typedef HANDLE (*WINPORT_CreateFile) ( LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, const DWORD *UnixMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
 typedef int (*WINPORT_GetFileDescriptor) (HANDLE hFile);
 typedef BOOL (*WINPORT_CloseHandle) (HANDLE hObject);
 typedef BOOL (*WINPORT_MoveFile) (LPCWSTR ExistingFileName, LPCWSTR NewFileName );
 typedef BOOL (*WINPORT_MoveFileEx) (LPCWSTR ExistingFileName, LPCWSTR NewFileName,DWORD dwFlags);
 typedef DWORD (*WINPORT_GetCurrentDirectory) (DWORD nBufferLength, LPWSTR lpBuffer);
 typedef BOOL (*WINPORT_SetCurrentDirectory) (LPCWSTR lpPathName);
 typedef BOOL (*WINPORT_GetFileSizeEx) ( HANDLE hFile, PLARGE_INTEGER lpFileSize);
 typedef DWORD (*WINPORT_GetFileSize) ( HANDLE hFile, LPDWORD lpFileSizeHigh);
 typedef DWORD64 (*WINPORT_GetFileSize64) ( HANDLE hFile);
 typedef BOOL (*WINPORT_ReadFile) ( HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
 typedef BOOL (*WINPORT_WriteFile) ( HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
 typedef BOOL (*WINPORT_SetFilePointerEx) ( HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod);
 typedef VOID (*WINPORT_FileAllocationHint) (HANDLE hFile, DWORD64 HintFileSize);
 typedef BOOL (*WINPORT_FileAllocationRequire) (HANDLE hFile, DWORD64 RequireFileSize);
 typedef DWORD (*WINPORT_SetFilePointer) ( HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod);
 typedef BOOL (*WINPORT_GetFileTime) ( HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime);
 typedef BOOL (*WINPORT_SetFileTime) ( HANDLE hFile, const FILETIME *lpCreationTime, const FILETIME *lpLastAccessTime, const FILETIME *lpLastWriteTime);
 typedef BOOL (*WINPORT_SetEndOfFile) ( HANDLE hFile);
 typedef BOOL (*WINPORT_FlushFileBuffers) ( HANDLE hFile);
 typedef DWORD (*WINPORT_GetFileType) ( HANDLE hFile);
 typedef DWORD (*WINPORT_GetFileAttributes) (LPCWSTR lpFileName);
 typedef DWORD (*WINPORT_SetFileAttributes) (LPCWSTR lpFileName, DWORD dwAttributes);
 typedef HANDLE (*WINPORT_FindFirstFileWithFlags) (LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData, DWORD dwFlags);
 typedef HANDLE (*WINPORT_FindFirstFile) (LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);
 typedef BOOL (*WINPORT_FindNextFile) (HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);
 typedef BOOL (*WINPORT_FindClose) (HANDLE hFindFile);
 typedef UINT (*WINPORT_GetTempFileName) ( LPCWSTR path, LPCWSTR prefix, UINT unique, LPWSTR buffer );
 typedef DWORD (*WINPORT_GetFullPathName) (LPCTSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart);
 typedef DWORD (*WINPORT_EvaluateAttributes) ( uint32_t unix_mode, const WCHAR *name );
 typedef DWORD (*WINPORT_EvaluateAttributesA) ( uint32_t unix_mode, const char *name );
 typedef VOID (*WINPORT_Sleep) (DWORD dwMilliseconds);
 typedef DWORD (*WINPORT_GetTickCount) ();
 typedef VOID (*WINPORT_GetLocalTime) (LPSYSTEMTIME lpSystemTime);
 typedef VOID (*WINPORT_GetSystemTime) (LPSYSTEMTIME lpSystemTime);
 typedef BOOL (*WINPORT_SystemTimeToFileTime) (const SYSTEMTIME *lpSystemTime, LPFILETIME lpFileTime);
 typedef BOOL (*WINPORT_LocalFileTimeToFileTime) (const FILETIME *lpLocalFileTime, LPFILETIME lpFileTime);
 typedef LONG (*WINPORT_CompareFileTime) (const FILETIME *lpFileTime1, const FILETIME *lpFileTime2);
 typedef BOOL (*WINPORT_FileTimeToLocalFileTime) (const FILETIME *lpFileTime, LPFILETIME lpLocalFileTime);
 typedef BOOL (*WINPORT_FileTimeToSystemTime) (const FILETIME *lpFileTime, LPSYSTEMTIME lpSystemTime);
 typedef VOID (*WINPORT_GetSystemTimeAsFileTime) (FILETIME *lpFileTime);
 typedef BOOL (*WINPORT_FileTimeToDosDateTime) (const FILETIME *lpFileTime, LPWORD lpFatDate, LPWORD lpFatTime);
 typedef BOOL (*WINPORT_DosDateTimeToFileTime) ( WORD fatdate, WORD fattime, LPFILETIME ft);
 typedef INT (*WINPORT_LCMapString) (LCID lcid, DWORD flags, LPCWSTR src, INT srclen, LPWSTR dst, INT dstlen);
 typedef DWORD (*WINPORT_CharUpperBuff) (LPWSTR lpsz, DWORD cchLength);
 typedef DWORD (*WINPORT_CharLowerBuff) (LPWSTR lpsz, DWORD cchLength);
 typedef BOOL (*WINPORT_IsCharLower) (WCHAR ch);
 typedef BOOL (*WINPORT_IsCharUpper) (WCHAR ch);
 typedef BOOL (*WINPORT_IsCharAlpha) (WCHAR ch);
 typedef BOOL (*WINPORT_IsCharAlphaNumeric) (WCHAR ch);
 typedef int (*WINPORT_CompareString) ( LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2);
 typedef int (*WINPORT_CompareStringA) ( LCID Locale, DWORD dwCmpFlags, LPCSTR lpString1, int cchCount1, LPCSTR lpString2, int cchCount2);
 typedef int (*WINPORT_WideCharToMultiByte) ( UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
 typedef int (*WINPORT_MultiByteToWideChar) ( UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
 typedef LPWSTR (*WINPORT_CharUpper) (LPWSTR lpsz);
 typedef LPWSTR (*WINPORT_CharLower) (LPWSTR lpsz);
 typedef UINT (*WINPORT_GetOEMCP) ();
 typedef UINT (*WINPORT_GetACP) ();
 typedef BOOL (*WINPORT_GetCPInfo) (UINT CodePage, LPCPINFO lpCPInfo);
 typedef BOOL (*WINPORT_GetCPInfoEx) (UINT codepage, DWORD dwFlags, LPCPINFOEX cpinfo);
 typedef BOOL (*WINPORT_EnumSystemCodePages) (CODEPAGE_ENUMPROCW lpfnCodePageEnum, DWORD flags);
 typedef BOOL (*WINPORT_MatchDecomposedUTF8) (int flags, const char *s1, size_t l1, const char *s2, size_t l2);
 typedef UINT (*WINPORT_RegisterClipboardFormat) (LPCWSTR lpszFormat);
 typedef BOOL (*WINPORT_OpenClipboard) (PVOID Reserved);
 typedef BOOL (*WINPORT_CloseClipboard) ();
 typedef BOOL (*WINPORT_EmptyClipboard) ();
 typedef BOOL (*WINPORT_IsClipboardFormatAvailable) (UINT format);
 typedef PVOID (*WINPORT_GetClipboardData) (UINT format);
 typedef PVOID (*WINPORT_SetClipboardData) (UINT format, HANDLE mem);
 typedef INT (*WINPORT_ChooseClipboard) (INT type);
 typedef PVOID (*WINPORT_ClipboardAlloc) (SIZE_T len);
 typedef SIZE_T (*WINPORT_ClipboardSize) (PVOID mem);
 typedef VOID (*WINPORT_ClipboardFree) (PVOID mem);
 typedef VOID (*WINPORT_PrintTextFragment) (LPCWSTR lpszJobName, LPCWSTR lpszTextFragment);
 typedef VOID (*WINPORT_PrintHtmlFragment) (LPCWSTR lpszJobName, LPCWSTR lpszTextFragment);
 typedef VOID (*WINPORT_PrintTextFile) (LPCWSTR lpszFileName);
 typedef VOID (*WINPORT_PrintHtmlFile) (LPCWSTR lpszFileName);
 typedef VOID (*WINPORT_PrintPreviewTextFragment) (LPCWSTR lpszJobName, LPCWSTR lpszTextFragment);
 typedef VOID (*WINPORT_PrintPreviewHtmlFragment) (LPCWSTR lpszJobName, LPCWSTR lpszTextFragment);
 typedef VOID (*WINPORT_PrintPreviewTextFile) (LPCWSTR lpszFileName);
 typedef VOID (*WINPORT_PrintPreviewHtmlFile) (LPCWSTR lpszFileName);
 typedef VOID (*WINPORT_PrintSettingsDialog) ();
 typedef BOOL (*WINPORT_PrintIsPreviewSupported) ();
 typedef BOOL (*WINPORT_PrintIsHTMLSupported) ();
 typedef BOOL (*WINPORT_PrintIsSettingsDialogSupported) ();
 typedef int (*WINPORT_GetKeyboardLayoutList) (int nBuff, HKL *lpList);
 typedef UINT (*WINPORT_MapVirtualKey) (UINT uCode, UINT uMapType);
 typedef SHORT (*WINPORT_VkKeyScan) (WCHAR ch);
 typedef int (*WINPORT_ToUnicodeEx) (UINT wVirtKey, UINT wScanCode, const BYTE *lpKeyState, LPWSTR pwszBuff, int cchBuff, UINT wFlags, HKL dwhkl);
 typedef BOOL (*WINPORT_GlobalMemoryStatusEx) (LPMEMORYSTATUSEX lpBuffer);
 typedef EXECUTION_STATE (*WINPORT_SetThreadExecutionState) (EXECUTION_STATE es);
 typedef BOOL (*WINPORT_SetThreadPriority) (int nPriority);
struct WINPORTDECL{
 WINPORT_ForkConsole ForkConsole;
 WINPORT_JoinConsole JoinConsole;
 WINPORT_DiscardConsole DiscardConsole;
 WINPORT_GetLargestConsoleWindowSize GetLargestConsoleWindowSize;
 WINPORT_SetConsoleWindowInfo SetConsoleWindowInfo;
 WINPORT_SetConsoleTitle SetConsoleTitle;
 WINPORT_GetConsoleTitle GetConsoleTitle;
 WINPORT_SetConsoleScreenBufferSize SetConsoleScreenBufferSize;
 WINPORT_GetConsoleScreenBufferInfo GetConsoleScreenBufferInfo;
 WINPORT_SetConsoleCursorPosition SetConsoleCursorPosition;
 WINPORT_SetConsoleCursorInfo SetConsoleCursorInfo;
 WINPORT_GetConsoleCursorInfo GetConsoleCursorInfo;
 WINPORT_GetConsoleMode GetConsoleMode;
 WINPORT_SetConsoleMode SetConsoleMode;
 WINPORT_ScrollConsoleScreenBuffer ScrollConsoleScreenBuffer;
 WINPORT_SetConsoleTextAttribute SetConsoleTextAttribute;
 WINPORT_CompositeCharRegister CompositeCharRegister;
 WINPORT_CompositeCharLookup CompositeCharLookup;
 WINPORT_WriteConsole WriteConsole;
 WINPORT_WriteConsoleOutput WriteConsoleOutput;
 WINPORT_WriteConsoleOutputCharacter WriteConsoleOutputCharacter;
 WINPORT_WaitConsoleInput WaitConsoleInput;
 WINPORT_ReadConsoleOutput ReadConsoleOutput;
 WINPORT_FillConsoleOutputAttribute FillConsoleOutputAttribute;
 WINPORT_FillConsoleOutputCharacter FillConsoleOutputCharacter;
 WINPORT_SetConsoleActiveScreenBuffer SetConsoleActiveScreenBuffer;
 WINPORT_SetConsoleCursorBlinkTime SetConsoleCursorBlinkTime;
 WINPORT_FlushConsoleInputBuffer FlushConsoleInputBuffer;
 WINPORT_GetNumberOfConsoleInputEvents GetNumberOfConsoleInputEvents;
 WINPORT_PeekConsoleInput PeekConsoleInput;
 WINPORT_ReadConsoleInput ReadConsoleInput;
 WINPORT_ReadConsoleInputBacktrace ReadConsoleInputBacktrace;
 WINPORT_WriteConsoleInput WriteConsoleInput;
 WINPORT_CheckForKeyPress CheckForKeyPress;
 WINPORT_SetConsoleDisplayMode SetConsoleDisplayMode;
 WINPORT_GetConsoleDisplayMode GetConsoleDisplayMode;
 WINPORT_SetConsoleWindowMaximized SetConsoleWindowMaximized;
 WINPORT_GetConsoleColorPalette GetConsoleColorPalette;
 WINPORT_GetConsoleBasePalette GetConsoleBasePalette;
 WINPORT_SetConsoleBasePalette SetConsoleBasePalette;
 WINPORT_GenerateConsoleCtrlEvent GenerateConsoleCtrlEvent;
 WINPORT_SetConsoleCtrlHandler SetConsoleCtrlHandler;
 WINPORT_SetConsoleScrollRegion SetConsoleScrollRegion;
 WINPORT_GetConsoleScrollRegion GetConsoleScrollRegion;
 WINPORT_SetConsoleScrollCallback SetConsoleScrollCallback;
 WINPORT_BeginConsoleAdhocQuickEdit BeginConsoleAdhocQuickEdit;
 WINPORT_SetConsoleTweaks SetConsoleTweaks;
 WINPORT_SaveConsoleWindowState SaveConsoleWindowState;
 WINPORT_ConsoleChangeFont ConsoleChangeFont;
 WINPORT_IsConsoleActive IsConsoleActive;
 WINPORT_ConsoleDisplayNotification ConsoleDisplayNotification;
 WINPORT_ConsoleBackgroundMode ConsoleBackgroundMode;
 WINPORT_SetConsoleFKeyTitles SetConsoleFKeyTitles;
 WINPORT_OverrideConsoleColor OverrideConsoleColor;
 WINPORT_SetConsoleRepaintsDefer SetConsoleRepaintsDefer;
 WINPORT_GetConsoleImageCaps GetConsoleImageCaps;
 WINPORT_SetConsoleImage SetConsoleImage;
 WINPORT_TransformConsoleImage TransformConsoleImage;
 WINPORT_DeleteConsoleImage DeleteConsoleImage;
 WINPORT_RegOpenKeyEx RegOpenKeyEx;
 WINPORT_RegCreateKeyEx RegCreateKeyEx;
 WINPORT_RegCloseKey RegCloseKey;
 WINPORT_RegDeleteKey RegDeleteKey;
 WINPORT_RegDeleteValue RegDeleteValue;
 WINPORT_RegSetValueEx RegSetValueEx;
 WINPORT_RegEnumKeyEx RegEnumKeyEx;
 WINPORT_RegEnumKey RegEnumKey;
 WINPORT_RegEnumValue RegEnumValue;
 WINPORT_RegQueryValueEx RegQueryValueEx;
 WINPORT_RegQueryInfoKey RegQueryInfoKey;
 WINPORT_RegWipeBegin RegWipeBegin;
 WINPORT_RegWipeEnd RegWipeEnd;
 WINPORT_GetLastError GetLastError;
 WINPORT_SetLastError SetLastError;
 WINPORT_GetCurrentProcessId GetCurrentProcessId;
 WINPORT_GetDoubleClickTime GetDoubleClickTime;
 WINPORT_CreateDirectory CreateDirectory;
 WINPORT_RemoveDirectory RemoveDirectory;
 WINPORT_DeleteFile DeleteFile;
 WINPORT_CreateFile CreateFile;
 WINPORT_GetFileDescriptor GetFileDescriptor;
 WINPORT_CloseHandle CloseHandle;
 WINPORT_MoveFile MoveFile;
 WINPORT_MoveFileEx MoveFileEx;
 WINPORT_GetCurrentDirectory GetCurrentDirectory;
 WINPORT_SetCurrentDirectory SetCurrentDirectory;
 WINPORT_GetFileSizeEx GetFileSizeEx;
 WINPORT_GetFileSize GetFileSize;
 WINPORT_GetFileSize64 GetFileSize64;
 WINPORT_ReadFile ReadFile;
 WINPORT_WriteFile WriteFile;
 WINPORT_SetFilePointerEx SetFilePointerEx;
 WINPORT_FileAllocationHint FileAllocationHint;
 WINPORT_FileAllocationRequire FileAllocationRequire;
 WINPORT_SetFilePointer SetFilePointer;
 WINPORT_GetFileTime GetFileTime;
 WINPORT_SetFileTime SetFileTime;
 WINPORT_SetEndOfFile SetEndOfFile;
 WINPORT_FlushFileBuffers FlushFileBuffers;
 WINPORT_GetFileType GetFileType;
 WINPORT_GetFileAttributes GetFileAttributes;
 WINPORT_SetFileAttributes SetFileAttributes;
 WINPORT_FindFirstFileWithFlags FindFirstFileWithFlags;
 WINPORT_FindFirstFile FindFirstFile;
 WINPORT_FindNextFile FindNextFile;
 WINPORT_FindClose FindClose;
 WINPORT_GetTempFileName GetTempFileName;
 WINPORT_GetFullPathName GetFullPathName;
 WINPORT_EvaluateAttributes EvaluateAttributes;
 WINPORT_EvaluateAttributesA EvaluateAttributesA;
 WINPORT_Sleep Sleep;
 WINPORT_GetTickCount GetTickCount;
 WINPORT_GetLocalTime GetLocalTime;
 WINPORT_GetSystemTime GetSystemTime;
 WINPORT_SystemTimeToFileTime SystemTimeToFileTime;
 WINPORT_LocalFileTimeToFileTime LocalFileTimeToFileTime;
 WINPORT_CompareFileTime CompareFileTime;
 WINPORT_FileTimeToLocalFileTime FileTimeToLocalFileTime;
 WINPORT_FileTimeToSystemTime FileTimeToSystemTime;
 WINPORT_GetSystemTimeAsFileTime GetSystemTimeAsFileTime;
 WINPORT_FileTimeToDosDateTime FileTimeToDosDateTime;
 WINPORT_DosDateTimeToFileTime DosDateTimeToFileTime;
 WINPORT_LCMapString LCMapString;
 WINPORT_CharUpperBuff CharUpperBuff;
 WINPORT_CharLowerBuff CharLowerBuff;
 WINPORT_IsCharLower IsCharLower;
 WINPORT_IsCharUpper IsCharUpper;
 WINPORT_IsCharAlpha IsCharAlpha;
 WINPORT_IsCharAlphaNumeric IsCharAlphaNumeric;
 WINPORT_CompareString CompareString;
 WINPORT_CompareStringA CompareStringA;
 WINPORT_WideCharToMultiByte WideCharToMultiByte;
 WINPORT_MultiByteToWideChar MultiByteToWideChar;
 WINPORT_CharUpper CharUpper;
 WINPORT_CharLower CharLower;
 WINPORT_GetOEMCP GetOEMCP;
 WINPORT_GetACP GetACP;
 WINPORT_GetCPInfo GetCPInfo;
 WINPORT_GetCPInfoEx GetCPInfoEx;
 WINPORT_EnumSystemCodePages EnumSystemCodePages;
 WINPORT_MatchDecomposedUTF8 MatchDecomposedUTF8;
 WINPORT_RegisterClipboardFormat RegisterClipboardFormat;
 WINPORT_OpenClipboard OpenClipboard;
 WINPORT_CloseClipboard CloseClipboard;
 WINPORT_EmptyClipboard EmptyClipboard;
 WINPORT_IsClipboardFormatAvailable IsClipboardFormatAvailable;
 WINPORT_GetClipboardData GetClipboardData;
 WINPORT_SetClipboardData SetClipboardData;
 WINPORT_ChooseClipboard ChooseClipboard;
 WINPORT_ClipboardAlloc ClipboardAlloc;
 WINPORT_ClipboardSize ClipboardSize;
 WINPORT_ClipboardFree ClipboardFree;
 WINPORT_PrintTextFragment PrintTextFragment;
 WINPORT_PrintHtmlFragment PrintHtmlFragment;
 WINPORT_PrintTextFile PrintTextFile;
 WINPORT_PrintHtmlFile PrintHtmlFile;
 WINPORT_PrintPreviewTextFragment PrintPreviewTextFragment;
 WINPORT_PrintPreviewHtmlFragment PrintPreviewHtmlFragment;
 WINPORT_PrintPreviewTextFile PrintPreviewTextFile;
 WINPORT_PrintPreviewHtmlFile PrintPreviewHtmlFile;
 WINPORT_PrintSettingsDialog PrintSettingsDialog;
 WINPORT_PrintIsPreviewSupported PrintIsPreviewSupported;
 WINPORT_PrintIsHTMLSupported PrintIsHTMLSupported;
 WINPORT_PrintIsSettingsDialogSupported PrintIsSettingsDialogSupported;
 WINPORT_GetKeyboardLayoutList GetKeyboardLayoutList;
 WINPORT_MapVirtualKey MapVirtualKey;
 WINPORT_VkKeyScan VkKeyScan;
 WINPORT_ToUnicodeEx ToUnicodeEx;
 WINPORT_GlobalMemoryStatusEx GlobalMemoryStatusEx;
 WINPORT_SetThreadExecutionState SetThreadExecutionState;
 WINPORT_SetThreadPriority SetThreadPriority;
};
// WinCompat.h

#define CONSOLE_FULLSCREEN_MODE 1
#define CONSOLE_WINDOWED_MODE 2

//
// ControlKeyState flags
//

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002
#define MOUSE_WHEELED 0x0004
#define MOUSE_HWHEELED 0x0008

#define KEY_EVENT         0x0001 // Event contains key event record
#define MOUSE_EVENT       0x0002 // Event contains mouse event record
#define WINDOW_BUFFER_SIZE_EVENT 0x0004 // Event contains window change event record
#define MENU_EVENT 0x0008 // Event contains menu event record
#define FOCUS_EVENT 0x0010 // event contains focus change
#define BRACKETED_PASTE_EVENT 0x0020 // event contains bracketed paste state change
#define CALLBACK_EVENT 0x0040 // callback to be invoked when its record dequeued, its translated into NOOP_EVENT when invoked
#define NOOP_EVENT 0x0080 // nothing interesting, typically injected to kick events dispatcher

/*
 * Virtual Keys, Standard Set
 */
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */

/*
 * 0x07 : unassigned
 */

#define VK_BACK           0x08
#define VK_TAB            0x09

/*
 * 0x0A - 0x0B : reserved
 */

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VK_HANGUL         0x15
#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

/*
 * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
 * 0x40 : unassigned
 * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
 */

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

/*
 * 0x5E : reserved
 */

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

/*
 * 0x88 - 0x8F : unassigned
 */

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

/*
 * NEC PC-9800 kbd definitions
 */
#define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad

/*
 * Fujitsu/OASYS kbd definitions
 */
#define VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

/*
 * 0x97 - 0x9F : unassigned
 */

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

/*
 * 0xB8 - 0xB9 : reserved
 */

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

/*
 * 0xC1 - 0xD7 : reserved
 */

/*
 * 0xD8 - 0xDA : unassigned
 */

#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF

/*
 * 0xE0 : reserved
 */

/*
 * Various extended or enhanced keyboards
 */
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO

#define VK_PROCESSKEY     0xE5

#define VK_ICO_CLEAR      0xE6


#define VK_PACKET         0xE7

#define VK_UNASSIGNED     0xE8

/*
 * 0xE8 : unassigned
 */

/*
 * Nokia/Ericsson definitions
 */
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

#define KEYEVENTF_EXTENDEDKEY 0x0001
#define KEYEVENTF_KEYUP       0x0002
#define KEYEVENTF_UNICODE     0x0004
#define KEYEVENTF_SCANCODE    0x0008

#define TICKS_PER_SECOND        10000000
#define EPOCH_DIFFERENCE        11644473600

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020
#define FILE_ATTRIBUTE_DEVICE           0x00000040
#define FILE_ATTRIBUTE_NORMAL           0x00000080
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100
#define FILE_ATTRIBUTE_SPARSE_FILE      0x00000200
#define FILE_ATTRIBUTE_REPARSE_POINT    0x00000400
#define FILE_ATTRIBUTE_COMPRESSED       0x00000800
#define FILE_ATTRIBUTE_OFFLINE          0x00001000
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x00002000
#define FILE_ATTRIBUTE_ENCRYPTED        0x00004000
#define FILE_ATTRIBUTE_INTEGRITY_STREAM 0x00008000
#define FILE_ATTRIBUTE_VIRTUAL          0x00010000
#define FILE_ATTRIBUTE_NO_SCRUB_DATA    0x00020000
#define FILE_ATTRIBUTE_BROKEN           0x00200000
#define FILE_ATTRIBUTE_EXECUTABLE       0x00400000
#define FILE_ATTRIBUTE_DEVICE_CHAR          0x00800000
#define FILE_ATTRIBUTE_DEVICE_FIFO          0x01000000
#define FILE_ATTRIBUTE_DEVICE_SOCK          0x02000000
#define FILE_ATTRIBUTE_HARDLINKS            0x08000000

// WinPort.h

#define CFKP_KEEP_MATCHED_KEY_EVENTS    0x001
#define CFKP_KEEP_UNMATCHED_KEY_EVENTS  0x002
#define CFKP_KEEP_MOUSE_EVENTS          0x004
#define CFKP_KEEP_OTHER_EVENTS          0x100

#define EXCLUSIVE_CTRL_LEFT			0x00000001
#define EXCLUSIVE_CTRL_RIGHT		0x00000002
#define EXCLUSIVE_ALT_LEFT			0x00000004
#define EXCLUSIVE_ALT_RIGHT			0x00000008
#define EXCLUSIVE_WIN_LEFT			0x00000010
#define EXCLUSIVE_WIN_RIGHT			0x00000020

#define CONSOLE_PAINT_SHARP			0x00010000
#define CONSOLE_OSC52CLIP_SET		0x00020000

#define CONSOLE_TTY_PALETTE_OVERRIDE	0x00040000

#define TWEAK_STATUS_SUPPORT_EXCLUSIVE_KEYS	0x01
#define TWEAK_STATUS_SUPPORT_PAINT_SHARP	0x02
#define TWEAK_STATUS_SUPPORT_OSC52CLIP_SET	0x04
#define TWEAK_STATUS_SUPPORT_CHANGE_FONT	0x08
#define TWEAK_STATUS_SUPPORT_TTY_PALETTE	0x10
#define TWEAK_STATUS_SUPPORT_BLINK_RATE		0x20
