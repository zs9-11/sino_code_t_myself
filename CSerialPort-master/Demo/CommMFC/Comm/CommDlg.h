
// CommDlg.h : ͷ�ļ�
//

#pragma once

//About CSerialPort start
#include "SerialPort.h"
#include "SerialPortInfo.h"
using namespace itas109;
//About CSerialPort end

using namespace std;


// CCommDlg �Ի���

	class CCommDlg : public CDialogEx, public has_slots<>//About CSerialPort 
{
// ����
public:
	CCommDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_COMM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	void OnSendMessage(unsigned char* str, int port, int str_len);
	void OnReceive();//About CSerialPort 

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnReceiveStr(WPARAM str, LPARAM commInfo);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CComboBox m_PortNr;
	CComboBox m_BaudRate;
	afx_msg void OnBnClickedButtonOpenClose();
	CButton m_OpenCloseCtrl;
	afx_msg void OnBnClickedButtonSend();
	CEdit m_Send;
	CEdit m_ReceiveCtrl;
	afx_msg void OnClose();
//	LONGLONG m_recvCount;
	CStatic m_recvCountCtrl;
	CStatic m_sendCountCtrl;
	afx_msg void OnBnClickedButtonClear();

private:
	CSerialPort m_SerialPort;//About CSerialPort 

	int rx; 
	int tx;
public:
	CComboBox m_Parity;
	CComboBox m_Stop;
	CComboBox m_DataBits;
	};
