import io
p = "/home/qqrm/wb-plugin-ui/resource.h"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
s = s.replace("#define IDI_LOGITECH                    106",
              "#define IDI_LOGITECH                    106\n#define IDC_DEVICE_LIST                 1001\n#define IDC_SHOW_UNKNOWN                1002\n#define IDC_SHOW_CHARGING               1003\n#define IDC_STATIC_HINT                 1004")
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)

p = "/home/qqrm/wb-plugin-ui/WirelessDeviceBattery.rc"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
old = '''IDD_OPTIONS_DIALOG DIALOGEX 0, 0, 194, 135
STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "设置"
FONT 9, "微软雅黑", 400, 0, 0x0
BEGIN
    DEFPUSHBUTTON   "确定",IDOK,80,114,50,14
    PUSHBUTTON      "取消",IDCANCEL,137,114,50,14
END'''
new = '''IDD_OPTIONS_DIALOG DIALOGEX 0, 0, 340, 220
STYLE DS_SETFONT | DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "Wireless Battery — devices"
FONT 9, "Segoe UI", 400, 0, 0x0
BEGIN
    LTEXT           "Uncheck devices you do not want to see:",IDC_STATIC_HINT,8,8,320,12
    CONTROL         "",IDC_DEVICE_LIST,"SysListView32",LVS_REPORT | LVS_SINGLESEL | WS_BORDER | WS_TABSTOP,8,24,324,140
    AUTOCHECKBOX    "Show unknown charge",IDC_SHOW_UNKNOWN,8,172,130,12
    AUTOCHECKBOX    "Show charging state",IDC_SHOW_CHARGING,8,186,130,12
    DEFPUSHBUTTON   "OK",IDOK,208,194,56,16
    PUSHBUTTON      "Cancel",IDCANCEL,272,194,56,16
END'''
if old not in s:
    # fallback: replace by regex on IDD block
    import re
    s = re.sub(r'IDD_OPTIONS_DIALOG DIALOGEX.*?\nEND', new, s, count=1, flags=re.S)
else:
    s = s.replace(old, new)
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("rc/h patched")
