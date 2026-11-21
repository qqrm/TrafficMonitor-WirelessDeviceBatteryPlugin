#include "pch.h"
#include "WirelessDeviceBattery.h"
#include "OptionsDlg.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(COptionsDlg, CDialog)

COptionsDlg::COptionsDlg(CWnd* pParent) : CDialog(IDD_OPTIONS_DIALOG, pParent) {}
COptionsDlg::~COptionsDlg() {}

void COptionsDlg::DoDataExchange(CDataExchange* pDX) { CDialog::DoDataExchange(pDX); }

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_DEVICE_LIST);
    if (list) {
        list->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
        CRect r; list->GetClientRect(r);
        list->InsertColumn(0, L"Device", LVCFMT_LEFT, r.Width() - 60);
        list->InsertColumn(1, L"Charge", LVCFMT_RIGHT, 56);
        int row = 0;
        for (const auto& d : m_snapshot.devices) {
            std::wstring name = d.name.empty() ? L"(unnamed)" : d.name;
            if (!d.via.empty()) name += L"  [" + d.via + L"]";
            list->InsertItem(row, name.c_str());
            std::wstring pct = d.percent < 0 ? L"?" : std::to_wstring(d.percent) + L"%";
            list->SetItemText(row, 1, pct.c_str());
            bool hidden = std::find(m_data.hidden_keys.begin(), m_data.hidden_keys.end(), d.key)
                          != m_data.hidden_keys.end();
            list->SetCheck(row, hidden ? FALSE : TRUE);
            list->SetItemData(row, row);
            ++row;
        }
    }
    ((CButton*)GetDlgItem(IDC_SHOW_UNKNOWN))->SetCheck(m_data.show_unknown ? TRUE : FALSE);
    ((CButton*)GetDlgItem(IDC_SHOW_CHARGING))->SetCheck(m_data.show_charging ? TRUE : FALSE);
    return TRUE;
}

void COptionsDlg::OnOK()
{
    m_data.hidden_keys.clear();
    CListCtrl* list = (CListCtrl*)GetDlgItem(IDC_DEVICE_LIST);
    if (list) {
        for (int i = 0; i < list->GetItemCount(); ++i) {
            if (list->GetCheck(i)) continue;
            int row = int(list->GetItemData(i));
            if (row >= 0 && size_t(row) < m_snapshot.devices.size())
                m_data.hidden_keys.push_back(m_snapshot.devices[row].key);
        }
    }
    m_data.show_unknown = ((CButton*)GetDlgItem(IDC_SHOW_UNKNOWN))->GetCheck() != FALSE;
    m_data.show_charging = ((CButton*)GetDlgItem(IDC_SHOW_CHARGING))->GetCheck() != FALSE;
    CDialog::OnOK();
}
