// EasySerialAssistantDlg.h : header file
//

#if !defined(AFX_EASYSERIALASSISTANTDLG_H__F873D2E8_C976_40BE_BD42_FBCA885E8406__INCLUDED_)
#define AFX_EASYSERIALASSISTANTDLG_H__F873D2E8_C976_40BE_BD42_FBCA885E8406__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <opencv2\opencv.hpp>
#include "CvvImage.h"
#include "ThreadSerial.h" //加入串口通信头文件

/////////////////////////////////////////////////////////////////////////////
// CEasySerialAssistantDlg dialog

class CEasySerialAssistantDlg : public CDialog
{
// Construction
public:
	CEasySerialAssistantDlg(CWnd* pParent = NULL);	// standard constructor

	void SendData(void); //数据发送函数
	static UINT ProDispThread(LPVOID pParam);//数据显示线程

// Dialog Data
	//{{AFX_DATA(CEasySerialAssistantDlg)
	enum { IDD = IDD_EASYSERIALASSISTANT_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasySerialAssistantDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CEasySerialAssistantDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnPortopen();
	afx_msg void OnSelchangeBodyPort();
	afx_msg void OnSelchangeLWingPort();
	afx_msg void OnSelchangeRWingPort();
	//afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


//以下为自定义变量///////////////////////////////////////////////////////////////
public:
	CComPort body_ComPort, lwing_ComPort, rwing_ComPort;

	unsigned char body_RecvBuf[0xffff];
	unsigned char lwing_RecvBuf[0xffff];
	unsigned char rwing_RecvBuf[0xffff];
	unsigned int  body_LastIndex;
	unsigned int  lwing_LastIndex;
	unsigned int  rwing_LastIndex;

	BOOL				m_bPortOpen;
	BOOL				body_bAutoClearn;
	BOOL				lwing_bAutoClearn;
	BOOL				rwing_bAutoClearn;
	BOOL				body_bThread;
	BOOL				lwing_bThread;
	BOOL				rwing_bThread;
	BOOL				body_bDispRecv;
	BOOL				lwing_bDispRecv;
	BOOL				rwing_bDispRecv;

	unsigned long       body_RecvCount;
	unsigned long       lwing_RecvCount;
	unsigned long       rwing_RecvCount;
	unsigned int		body_nRecvTimes;
	unsigned int		lwing_nRecvTimes;
	unsigned int		rwing_nRecvTimes;
	unsigned int		body_nCurLen;
	unsigned int		lwing_nCurLen;
	unsigned int		rwing_nCurLen;

	int					body_Port;
	int					lwing_Port;
	int					rwing_Port;
	DWORD				m_DwBaud;
	BYTE				m_DataBits;
	CComPort::Parity	m_Parity;
	CComPort::StopBits	m_Stopbits;


	CWinThread*			m_pWinThread;		//线程指针
	CWinThread*         openglThread;
	//CWinThread*			videoThread;
	//afx_msg void OnStnClickedStaticRx();
	//afx_msg void OnBnClickedFreq();
	//afx_msg void OnBnClickedDefualt();
	//afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnBnClickedaltZero5();
	//afx_msg void OnBnClickedcalib();
	//afx_msg void CallibStart();
	//afx_msg void CallibSave();
	//afx_msg void OnCbnSelchangeCombo1();
	//afx_msg void OnCbnSelchangeComboFreq();
	//afx_msg void OnDeltaposSpinYawseroff(NMHDR *pNMHDR, LRESULT *pResult);
	
	//图像显示
	void ShowImage( IplImage* img, UINT ID );
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedLwingErr();
	afx_msg void OnBnClickedRwingErr();
	afx_msg void OnBnClickedVideoBtn();
};
//数据接收事件
void OnBodyReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount);
void OnLWingReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount);
void OnRWingReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount);
//串口线程打开或关闭事件
void OnComBreak(LPVOID pSender,DWORD dwMask,COMSTAT stat);


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYSERIALASSISTANTDLG_H__F873D2E8_C976_40BE_BD42_FBCA885E8406__INCLUDED_)
