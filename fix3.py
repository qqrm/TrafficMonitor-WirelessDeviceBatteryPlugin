root = "/home/qqrm/wb-plugin-ui"
# add afxcmn.h for CListCtrl
p = f"{root}/framework.h"
s = open(p, encoding="utf-8", errors="surrogateescape").read()
if "afxcmn.h" not in s:
    s = s.replace("#include <afxdisp.h>        // MFC 自动化类",
                  "#include <afxdisp.h>        // MFC 自动化类\n#include <afxcmn.h>         // MFC common controls (CListCtrl)")
open(p, "w", encoding="utf-8", errors="surrogateescape").write(s)
print("fix3")
