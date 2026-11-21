root = "/home/qqrm/wb-plugin-ui"
# DPI should be const in header (definition already const)
p = f"{root}/WirelessDeviceBattery.h"
s = open(p, encoding="utf-8").read().replace("int DPI(int pixel);", "int DPI(int pixel) const;")
open(p, "w", encoding="utf-8").write(s)

# get DPI from plugin via const method is fine, but m_app is private and DPI uses it - OK now.
# item: avoid std::min/std::max macro clash by local vars
p = f"{root}/WirelessDeviceBatteryItem.cpp"
s = open(p, encoding="utf-8").read()
s = s.replace("int markSize = std::min(h - 2, DEVICE_MARK_SIZE);",
              "int markSize = (h - 2) < DEVICE_MARK_SIZE ? (h - 2) : DEVICE_MARK_SIZE;")
s = s.replace("int barH = std::max(3, (h - 12) / 2);",
              "int barH = ((h - 12) / 2) > 3 ? ((h - 12) / 2) : 3;")
open(p, "w", encoding="utf-8").write(s)

# OptionsDlg.cpp: maybe stale copy - ensure list declared in OnOK
p = f"{root}/OptionsDlg.cpp"
s = open(p, encoding="utf-8").read()
if "void COptionsDlg::OnOK()" in s and "CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_DEVICE_LIST);" not in s.split("void COptionsDlg::OnOK()",1)[1]:
    s = s.replace("    m_data.hidden_keys.clear();\n", "    m_data.hidden_keys.clear();\n    CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_DEVICE_LIST);\n", 1)
open(p, "w", encoding="utf-8").write(s)
print("fix2")
