#pragma once
#include "WirelessDeviceBattery.h"

class COptionsDlg : public CDialog
{
    DECLARE_DYNAMIC(COptionsDlg)
public:
    COptionsDlg(CWnd* pParent = nullptr);
    virtual ~COptionsDlg();

    SettingData m_data;
    BatterySnapshot m_snapshot;

    enum { IDD = IDD_OPTIONS_DIALOG };
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
};
