root = "/home/qqrm/wb-plugin-ui"
p = f"{root}/WirelessDeviceBattery.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
if "gdiplus.lib" not in s:
    s = s.replace('#include <gdiplus.h>', '#include <gdiplus.h>\n#pragma comment(lib, "gdiplus.lib")\n#pragma comment(lib, "lib\\\\x64\\\\wireless_battery.lib")')
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)

# Ensure IMPLEMENT_DYNAMIC present in OptionsDlg.cpp
p = f"{root}/OptionsDlg.cpp"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
if "IMPLEMENT_DYNAMIC" not in s:
    s = s.replace("COptionsDlg::COptionsDlg(CWnd* pParent)",
                  "IMPLEMENT_DYNAMIC(COptionsDlg, CDialog)\n\nCOptionsDlg::COptionsDlg(CWnd* pParent)")
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("fix4")
