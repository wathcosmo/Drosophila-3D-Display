// EasySerialAssistantDlg.cpp : implementation file
///////////////////////////////////////////////////////////////////////////////////////
///���̴߳���ͨ��ʵ���ļ�
///BUG������                                                                  
//�˳��򲻵�������ҵ��;
//�����̲߳��ֳ���ο�������,��ǣ���Ȩ,����������ϵ
///2009-6-10 V1.0
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EasySerialAssistant.h"
#include "EasySerialAssistantDlg.h"
#include <gl\glut.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include "simple3DGraphics.h"
#include "objectControllers.h"
#include "coordinateTrans.h"
#include "BMPLoader.h"
using namespace std;

bool mouseLeftActive = false;
bool mouseMiddleActive = false;
bool mouseRightActive = false;
ArcBall arcBall = ArcBall(true);
ArcBall wing_r = ArcBall(true);
ArcBall wing_l = ArcBall(true);
MoveController2D moveController = MoveController2D(false);
ScaleController scaleController = ScaleController(true);
CoordinateTrans coordinateTrans = CoordinateTrans();

GLuint *textureID;
int textureCount = 4;
string textureFile[4] = {"bg.bmp", "wood2.bmp", "stone.bmp", "stone2.bmp"};

int sliceCount = 20;	//�����Ƭ��

ofstream fout("1.txt"); // for debugging


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEasySerialAssistantDlg dialog

CEasySerialAssistantDlg::CEasySerialAssistantDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEasySerialAssistantDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEasySerialAssistantDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	pStc = nullptr;
	pDC = nullptr;
	capture = nullptr;
}

void CEasySerialAssistantDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEasySerialAssistantDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CEasySerialAssistantDlg, CDialog)
	//{{AFX_MSG_MAP(CEasySerialAssistantDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_PORTOPEN, OnBtnPortopen)
	ON_CBN_SELCHANGE(IDC_BODY_PORT, OnSelchangeBodyPort)
	ON_CBN_SELCHANGE(IDC_LWING_PORT, OnSelchangeLWingPort)
	ON_CBN_SELCHANGE(IDC_RWING_PORT, OnSelchangeRWingPort)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CEasySerialAssistantDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_LWING_ERR, &CEasySerialAssistantDlg::OnBnClickedLwingErr)
	ON_BN_CLICKED(IDC_RWING_ERR, &CEasySerialAssistantDlg::OnBnClickedRwingErr)
	ON_BN_CLICKED(IDC_VIDEO_BTN, &CEasySerialAssistantDlg::OnBnClickedVideoBtn)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEasySerialAssistantDlg message handlers

void display(void);
void changeSize(int, int);
void init(void);
void textureInit(GLuint**, int, string*, GLenum = GL_RGB);

UINT openglProc(LPVOID lpParameter)
{
	int argc = 1;
	char *argv[] = {"good"};
	glutInit(&argc, argv);	//glut��ʼ��
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	//�趨��ʾģʽ
    glutInitWindowPosition(300, 100);	//�趨��������Ļ�е�λ��
    glutInitWindowSize(400, 400);	//�趨���ڿ��
    glutCreateWindow("OpenGL");	//��������
	init();	//��ʼ��
	textureInit(&textureID, textureCount, textureFile, GL_RGB);	//��ʼ������

	glutReshapeFunc(&changeSize);	//ע�ᴰ�ڴ�С�ı亯��
    glutDisplayFunc(&display);	//ע����Ⱦ����
	glutIdleFunc(&display);	//ע�����¼�����ʱ�ĺ���
    glutMainLoop();	//��ʼ

    return 0;
}

BOOL CEasySerialAssistantDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here

	//��ʼ��Ĭ�ϲ���
	body_Port = 1;
	lwing_Port = 2;
	rwing_Port = 3;
	m_DwBaud=115200;
	m_DataBits=8;
	m_Parity=CComPort::NoParity;
	m_Stopbits=CComPort::OneStopBit;
	
	body_ComPort.SetReceiveFunc((FOnReceiveData)OnBodyReceiveData,this); 
	lwing_ComPort.SetReceiveFunc((FOnReceiveData)OnLWingReceiveData,this);
	rwing_ComPort.SetReceiveFunc((FOnReceiveData)OnRWingReceiveData,this); 
    body_ComPort.SetBreakHandleFunc(OnComBreak); 
	lwing_ComPort.SetBreakHandleFunc(OnComBreak); 
	rwing_ComPort.SetBreakHandleFunc(OnComBreak); 
	//m_ComPort.SetTimeOut(1);


	//SetDlgItemInt(IDC_EDIT_TIME,10);
	unsigned char i=0;

	//��Ӷ˿ں�
	CString szComPort="";
	for(i=0;i<20;i++)
	{
		szComPort.Format("COM%d",i+1);
		((CComboBox *)GetDlgItem(IDC_BODY_PORT))->InsertString(i,szComPort);
		((CComboBox *)GetDlgItem(IDC_LWING_PORT))->InsertString(i,szComPort);
		((CComboBox *)GetDlgItem(IDC_RWING_PORT))->InsertString(i,szComPort);
	}

	//Ĭ��ѡ��
	((CComboBox *)GetDlgItem(IDC_BODY_PORT))->SetCurSel(body_Port-1);
	((CComboBox *)GetDlgItem(IDC_LWING_PORT))->SetCurSel(lwing_Port-1);
	((CComboBox *)GetDlgItem(IDC_RWING_PORT))->SetCurSel(rwing_Port-1); 

	//�������ݱ�����ʼ��
	m_bPortOpen=false;
	body_bAutoClearn=false;
	lwing_bAutoClearn=false;
	rwing_bAutoClearn=false;
	body_bThread=false;
	lwing_bThread=false;
	rwing_bThread=false;
	body_bDispRecv=true;
	lwing_bDispRecv=true;
	rwing_bDispRecv=true;

	memset(body_RecvBuf,0,4096);
	memset(lwing_RecvBuf,0,4096);
	memset(rwing_RecvBuf,0,4096);
	body_LastIndex=0;
	lwing_LastIndex=0;
	rwing_LastIndex=0;
	body_nRecvTimes=0;
	lwing_nRecvTimes=0;
	rwing_nRecvTimes=0;
	body_nCurLen=0;
	lwing_nCurLen=0;
	rwing_nCurLen=0;

	//��Ƶ�ؼ���ʼ��
	pStc = (CStatic *)GetDlgItem(IDC_IMG_DISPLAY); //��ȡPicture�ؼ�
    pStc->GetClientRect(&rect); //��CWind��ͻ���������㴫������
    pDC = pStc->GetDC(); //�õ�Picture�ؼ��豸������
    hDC = pDC->GetSafeHdc(); //�õ��ؼ��豸�����ĵľ�� 

	//�����߳�
	m_pWinThread = AfxBeginThread(ProDispThread,this); //����UI�߳�
	m_pWinThread->m_bAutoDelete = true;

	openglThread = AfxBeginThread(openglProc,GetSafeHwnd()); //����3Dģ���߳�
    openglThread->m_bAutoDelete = true;


	return TRUE;  // return TRUE  unless you set the focus to a control
}



void CEasySerialAssistantDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
		CDialog::OnSysCommand(nID, lParam);
	
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEasySerialAssistantDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEasySerialAssistantDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void OnComBreak(LPVOID pSender,DWORD dwMask,COMSTAT stat)
{
	//deal with the break of com here
	//��ͨ��رմ���ͨ��ʱ����������Ϣ
	//::AfxMessageBox("Break!");
}

//���ڽ����¼��ж�
void OnBodyReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount)
{
	CEasySerialAssistantDlg* pDlg = (CEasySerialAssistantDlg*) pSender;	
	pDlg->body_nRecvTimes++;
	pDlg->body_nCurLen=InBufferCount;
	
	if(pDlg->body_bDispRecv == FALSE)
	{
		pDlg->body_RecvCount+=InBufferCount;
		pDlg->body_LastIndex+=InBufferCount;
		return;
	}

	if(InBufferCount<0xffff && pDlg->body_LastIndex<0xffff)
	{
		if(pDlg->body_bAutoClearn == false) //û��ѡ���Զ����
		{
			//memcpy(&(pDlg->m_RecvBuf[pDlg->m_LastIndex]),pBuf,InBufferCount);
			//pDlg->m_RecvCount+=InBufferCount;
			//pDlg->m_LastIndex+=InBufferCount;
			memcpy(pDlg->body_RecvBuf,pBuf,InBufferCount);
			pDlg->body_RecvCount+=InBufferCount;
			pDlg->body_LastIndex=InBufferCount;
		}
		else
		{
			memcpy(pDlg->body_RecvBuf,pBuf,InBufferCount);
			pDlg->body_RecvCount+=InBufferCount;
			pDlg->body_LastIndex=InBufferCount;		
		}
	}
	else
	{
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"���ջ������!");
		pDlg->body_LastIndex=0;
		return;
	}
	
	//PostMessage(pDlg->m_hWnd,WM_PCOM,InBufferCount,pDlg->m_nRecvTimes);
}


void OnLWingReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount)
{
	CEasySerialAssistantDlg* pDlg = (CEasySerialAssistantDlg*) pSender;	
	pDlg->lwing_nRecvTimes++;
	pDlg->lwing_nCurLen=InBufferCount;
	
	if(pDlg->lwing_bDispRecv == FALSE)
	{
		pDlg->lwing_RecvCount+=InBufferCount;
		pDlg->lwing_LastIndex+=InBufferCount;
		return;
	}

	if(InBufferCount<0xffff && pDlg->lwing_LastIndex<0xffff)
	{
		if(pDlg->lwing_bAutoClearn == false) //û��ѡ���Զ����
		{
			//memcpy(&(pDlg->m_RecvBuf[pDlg->m_LastIndex]),pBuf,InBufferCount);
			//pDlg->m_RecvCount+=InBufferCount;
			//pDlg->m_LastIndex+=InBufferCount;
			memcpy(pDlg->lwing_RecvBuf,pBuf,InBufferCount);
			pDlg->lwing_RecvCount+=InBufferCount;
			pDlg->lwing_LastIndex=InBufferCount;
		}
		else
		{
			memcpy(pDlg->lwing_RecvBuf,pBuf,InBufferCount);
			pDlg->lwing_RecvCount+=InBufferCount;
			pDlg->lwing_LastIndex=InBufferCount;		
		}
	}
	else
	{
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"���ջ������!");
		pDlg->lwing_LastIndex=0;
		return;
	}
	
	//PostMessage(pDlg->m_hWnd,WM_PCOM,InBufferCount,pDlg->m_nRecvTimes);
}


void OnRWingReceiveData(LPVOID pSender,void* pBuf,DWORD InBufferCount)
{
	CEasySerialAssistantDlg* pDlg = (CEasySerialAssistantDlg*) pSender;	
	pDlg->rwing_nRecvTimes++;
	pDlg->rwing_nCurLen=InBufferCount;
	
	if(pDlg->rwing_bDispRecv == FALSE)
	{
		pDlg->rwing_RecvCount+=InBufferCount;
		pDlg->rwing_LastIndex+=InBufferCount;
		return;
	}

	if(InBufferCount<0xffff && pDlg->rwing_LastIndex<0xffff)
	{
		if(pDlg->rwing_bAutoClearn == false) //û��ѡ���Զ����
		{
			//memcpy(&(pDlg->m_RecvBuf[pDlg->m_LastIndex]),pBuf,InBufferCount);
			//pDlg->m_RecvCount+=InBufferCount;
			//pDlg->m_LastIndex+=InBufferCount;
			memcpy(pDlg->rwing_RecvBuf,pBuf,InBufferCount);
			pDlg->rwing_RecvCount+=InBufferCount;
			pDlg->rwing_LastIndex=InBufferCount;
		}
		else
		{
			memcpy(pDlg->rwing_RecvBuf,pBuf,InBufferCount);
			pDlg->rwing_RecvCount+=InBufferCount;
			pDlg->rwing_LastIndex=InBufferCount;		
		}
	}
	else
	{
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"���ջ������!");
		pDlg->rwing_LastIndex=0;
		return;
	}
	
	//PostMessage(pDlg->m_hWnd,WM_PCOM,InBufferCount,pDlg->m_nRecvTimes);
}

void CEasySerialAssistantDlg::OnBtnPortopen() 
{
	// TODO: Add your control notification handler code here
	if(m_bPortOpen == TRUE)
	{
		body_ComPort.Close();
		lwing_ComPort.Close();
		rwing_ComPort.Close();
		m_bPortOpen=false;
		HICON hIcon=::LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_RED));
		((CStatic *)GetDlgItem(IDC_STATIC_COMSTATE))->SetIcon(hIcon);
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"���ڹر�");
		SetDlgItemText(IDC_BTN_PORTOPEN,"�ѹر�");
		return;
	}
	else
	{
		m_bPortOpen=true;
		SetDlgItemText(IDC_BTN_PORTOPEN,"�Ѵ�");
	}

	if(!(body_ComPort.Open(body_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) || 
		!(lwing_ComPort.Open(lwing_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) ||
		!(rwing_ComPort.Open(rwing_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) )
	{
		m_bPortOpen=false;
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"���ڴ�ʧ��");
		::MessageBox(::GetForegroundWindow(), "���ڴ�ʧ��", "��ʾ��Ϣ", MB_ICONWARNING | MB_OK);
		
		HICON hIcon=::LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_RED));
		((CStatic *)GetDlgItem(IDC_STATIC_COMSTATE))->SetIcon(hIcon);
	}
	else
	{
		m_bPortOpen=true;
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"���ڴ򿪳ɹ�");
		HICON hIcon=::LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_GREEN));
		((CStatic *)GetDlgItem(IDC_STATIC_COMSTATE))->SetIcon(hIcon);
	}
}

void CEasySerialAssistantDlg::OnSelchangeBodyPort() 
{
	// TODO: Add your control notification handler code here
	body_Port = (((CComboBox *)GetDlgItem(IDC_BODY_PORT))->GetCurSel())+1;

}

void CEasySerialAssistantDlg::OnSelchangeLWingPort() 
{
	// TODO: Add your control notification handler code here
	lwing_Port = (((CComboBox *)GetDlgItem(IDC_LWING_PORT))->GetCurSel())+1;

}

void CEasySerialAssistantDlg::OnSelchangeRWingPort() 
{
	// TODO: Add your control notification handler code here
	rwing_Port = (((CComboBox *)GetDlgItem(IDC_RWING_PORT))->GetCurSel())+1;

}

//---------------AHRS ���ݽӿڲ���------------------------------------------------

//uart reicer flag
#define b_uart_head  0x80  //�յ�A5 ͷ ��־λ
#define b_rx_over    0x40  //�յ�������֡��־
// USART Receiver buffer
#define RX_BUFFER_SIZE 100 //���ջ������ֽ���

//void Decode_frame(unsigned char data);
volatile unsigned char rx_buffer[RX_BUFFER_SIZE]; //�������ݻ�����
volatile unsigned char rx_wr_index; //����дָ��
volatile unsigned char RC_Flag;  //����״̬��־�ֽ�

//�����ĽǶ�ֵ
float yaw = 0,  //ƫ����
pitch = 0,//����
roll = 0; //��ת
float body_yaw, body_pitch, body_roll;
float lwing_yaw, lwing_pitch, lwing_roll;
float rwing_yaw, rwing_pitch, rwing_roll;
float body_yaw_err = 0, lwing_yaw_err = 0, rwing_yaw_err = 0; //ƫ�����


//�ڽ�����һ֡IMU��̬����󣬵�������ӳ�����ȡ����̬����
void UART2_Get_IMU(void)
{
	int temp;
	
	temp = 0;
	temp = rx_buffer[2];
	temp <<= 8;
	temp |= rx_buffer[3];
	if(temp&0x8000){
	temp = 0-(temp&0x7fff);
	}else temp = (temp&0x7fff);
	yaw=(float)temp / 10.0f; //ƫ����
	
	
	temp = 0;
	temp = rx_buffer[4];
	temp <<= 8;
	temp |= rx_buffer[5];
	if(temp&0x8000){
	temp = 0-(temp&0x7fff);
	}else temp = (temp&0x7fff);
	pitch=(float)temp / 10.0f;//����
	
	temp = 0;
	temp = rx_buffer[6];
	temp <<= 8;
	temp |= rx_buffer[7];
	if(temp&0x8000){
	temp = 0-(temp&0x7fff);
	}else temp = (temp&0x7fff);
	roll=(float)temp / 10.0f;//��ת
}



//--У�鵱ǰ���յ���һ֡�����Ƿ� ��֡У���ֽ�һ��
unsigned char Sum_check(void)
{ 
	unsigned char i;
	unsigned int checksum=0; 
	for(i=0;i<rx_buffer[0]-2;i++)
		checksum+=rx_buffer[i];
	if((checksum%256)==rx_buffer[rx_buffer[0]-2])
		return(0x01); //Checksum successful
	else
		return(0x00); //Checksum error
}


void OnAhrs_Data(unsigned char data){
	
	if(data==0xa5) 
	{ 
		RC_Flag|=b_uart_head; //������յ�A5 ��λ֡ͷ��רλ
		rx_buffer[rx_wr_index++]=data; //��������ֽ�.
	}
	else if(data==0x5a)
	{ 
		if(RC_Flag&b_uart_head) //�����һ���ֽ���A5 ��ô�϶� �����֡��ʼ�ֽ�
		{ rx_wr_index=0;  //���� ������ָ��
		RC_Flag&=~b_rx_over; //���֡�Ÿոտ�ʼ��
		}
		else //��һ���ֽڲ���A5
			rx_buffer[rx_wr_index++]=data;
		RC_Flag&=~b_uart_head; //��֡ͷ��־
	}
	   else
	   { rx_buffer[rx_wr_index++]=data;
	   RC_Flag&=~b_uart_head;
	   if(rx_wr_index==rx_buffer[0]) //�չ����ֽ���.
	   {  
		   RC_Flag|=b_rx_over; //��λ ����������һ֡����
		  
	   }
	   }
	   
	   if(rx_wr_index==RX_BUFFER_SIZE) //��ֹ���������
  rx_wr_index--;


}
//---------------------------------------------------------------------
//======================================================================

//��ʾ�߳�
UINT CEasySerialAssistantDlg::ProDispThread(LPVOID pParam)
{	
	CEasySerialAssistantDlg* pParlPro=(CEasySerialAssistantDlg*)pParam;
	pParlPro->body_bThread = TRUE;
	pParlPro->lwing_bThread = TRUE;
	pParlPro->rwing_bThread = TRUE;
	//CString c="",d="";
	//char b[4];

	while(1)
	{
		Sleep(1);
		
		if(pParlPro->body_LastIndex > 0) //�Ƿ������ݽ��յ�
		{
			//AHRS��֡����[��ʼ]
			for(int i=0;i<(int)pParlPro->body_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->body_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //�Ѿ�������һ֡?
					RC_Flag&=~b_rx_over; //���־��
					if(Sum_check()){ 
						//У��ͨ��
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU ������
							UART2_Get_IMU();	//ȡ����
							//������ʾ
							body_yaw = yaw;
							body_pitch = -pitch;
							body_roll = -roll;
							body_yaw -= body_yaw_err;
							if (body_yaw < 0) 
								body_yaw += 360;
							str.Format("������:%g",body_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch))->SetWindowText(str);
							str.Format("�෭��:%g",body_roll);
							(pParlPro->GetDlgItem(IDC_Roll))->SetWindowText(str);
							str.Format("ƫ����:%.1f",body_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw))->SetWindowText(str);
						}
					}//У���Ƿ�ͨ��?
				}
			}//AHRS��֡���� [����]
		}

		if(pParlPro->lwing_LastIndex > 0) //�Ƿ������ݽ��յ�
		{
			//AHRS��֡����[��ʼ]
			for(int i=0;i<(int)pParlPro->lwing_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->lwing_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //�Ѿ�������һ֡?
					RC_Flag&=~b_rx_over; //���־��
					if(Sum_check()){ 
						//У��ͨ��
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU ������
							UART2_Get_IMU();	//ȡ����
							//������ʾ
							lwing_yaw = yaw;
							lwing_pitch = pitch;
							lwing_roll = roll;
							lwing_yaw -= lwing_yaw_err;
							if (lwing_yaw < 0) 
								lwing_yaw += 360;
							str.Format("������:%g",lwing_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch2))->SetWindowText(str);
							str.Format("�෭��:%g",lwing_roll);
							(pParlPro->GetDlgItem(IDC_Roll2))->SetWindowText(str);
							str.Format("ƫ����:%.1f",lwing_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw2))->SetWindowText(str);
						}
					}//У���Ƿ�ͨ��?
				}
			}//AHRS��֡���� [����]
		}

		if(pParlPro->rwing_LastIndex > 0) //�Ƿ������ݽ��յ�
		{
			//AHRS��֡����[��ʼ]
			for(int i=0;i<(int)pParlPro->rwing_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->rwing_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //�Ѿ�������һ֡?
					RC_Flag&=~b_rx_over; //���־��
					if(Sum_check()){ 
						//У��ͨ��
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU ������
							UART2_Get_IMU();	//ȡ����
							//������ʾ
							rwing_yaw = yaw;
							rwing_pitch = pitch;
							rwing_roll = roll;
							rwing_yaw -= rwing_yaw_err;
							if (rwing_yaw < 0) 
								rwing_yaw += 360;
							str.Format("������:%g",rwing_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch3))->SetWindowText(str);
							str.Format("�෭��:%g",rwing_roll);
							(pParlPro->GetDlgItem(IDC_Roll3))->SetWindowText(str);
							str.Format("ƫ����:%.1f",rwing_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw3))->SetWindowText(str);
						}
					}//У���Ƿ�ͨ��?
				}
			}//AHRS��֡���� [����]
		}

	}
    
    pParlPro->body_bThread=false; //������־��λ
    pParlPro->lwing_bThread=false;
    pParlPro->rwing_bThread=false;
	delete pParlPro;
	return 0;
}

void CEasySerialAssistantDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if(m_bPortOpen == TRUE) 
	{
		body_ComPort.Close();
		lwing_ComPort.Close();
		rwing_ComPort.Close();
	}
	
	glDeleteTextures(textureCount, textureID);
	if (textureID != NULL)
		delete[] textureID;
}





/****************************OpenGL ��������********************************/

// ��ȡ��ת������yaw, pitch, roll Ϊ���ȡ�
void optRotate(ArcBall &arc, double yaw, double pitch, double roll)
{
	double x1, y1, z1, x2, y2, z2;
	//double ang1 = M_PI / 2 - yaw;
	
	x1 = cos(-yaw);
	y1 = sin(-yaw);
	z1 = 0;

	x2 = cos(M_PI_2 - yaw) * cos(pitch);
	y2 = sin(M_PI_2 - yaw) * cos(pitch);
	z2 = sin(pitch);

	arc.rotate(x2, y2, z2, roll);
	arc.rotate(x1, y1, z1, pitch);
	arc.rotate(0, 0, 1.0, -yaw);

	return;
}

void cor_rotate(double ux, double uy, double uz, double angle, double x, double y, double z, double *ans)
{
	//angle = -angle;
	double *newRotate = new double [9];
	double cosA = cos(angle);
	double _cosA = 1-cosA;
	double sinA = sin(angle);
	newRotate[0] = cosA + ux*ux*_cosA;
	newRotate[1] = ux*uy*_cosA - uz*sinA;
	newRotate[2] = ux*uz*_cosA + uy*sinA;
	newRotate[3] = ux*uy*_cosA + uz*sinA;
	newRotate[4] = cosA + uy*uy*_cosA;
	newRotate[5] = uy*uz*_cosA - ux*sinA;
	newRotate[6] = ux*uz*_cosA - uy*sinA;
	newRotate[7] = uy*uz*_cosA + ux*sinA;
	newRotate[8] = cosA + uz*uz*_cosA;

	ans[0] = newRotate[0] * x + newRotate[1] * y + newRotate[2] * z;
	ans[1] = newRotate[3] * x + newRotate[4] * y + newRotate[5] * z;
	ans[2] = newRotate[6] * x + newRotate[7] * y + newRotate[8] * z;

	delete[] newRotate;
}

void getpnt(double yaw, double pitch, double roll, double x, double y, double z, double *ans)
{
	double x1, y1, z1, x2, y2, z2;
	x1 = cos(-yaw);
	y1 = sin(-yaw);
	z1 = 0;

	x2 = cos(M_PI_2 - yaw) * cos(pitch);
	y2 = sin(M_PI_2 - yaw) * cos(pitch);
	z2 = sin(pitch);
	
	cor_rotate(0, 0, 1.0, -yaw, x, y, z, ans);
	cor_rotate(x1, y1, z1, pitch, ans[0], ans[1], ans[2], ans);
	cor_rotate(x2, y2, z2, roll, ans[0], ans[1], ans[2], ans);
}

//�Ƕ�ת����
auto DegreeToRadian = [](float degree) { return degree / 180.0 * M_PI; };

//���ƺ���
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//�������	
	glMatrixMode(GL_MODELVIEW);	//ѡ�����ģʽΪģ����ͼ����
	
	glLoadIdentity();	//����ģ�͹۲����
	gluLookAt(
		0, 0, 20,	//������ͼ�����۾�������
		0, 0, 0,			//�۾����������
		0, 1, 0//ͷ�����ϵķ���
	);			
	float ctr[3] = {0, 0, 0};
	double rbody[3], lbody[3], rwing[3], lwing[3];

	

	//��������
	arcBall.resetMatrix();
	optRotate(arcBall, DegreeToRadian(body_yaw), DegreeToRadian(body_pitch), DegreeToRadian(body_roll));

	glPushMatrix();
	glMultMatrixd(arcBall.rotateMatrix);
	{
		glBindTexture(GL_TEXTURE_2D, textureID[0]);
		glScaled(1.5, 3.0, 1.0);
		drawSphere(sliceCount, sliceCount, 1, ctr);
	}
	glPopMatrix();

	//��������
	wing_l.resetMatrix();
	optRotate(wing_l, DegreeToRadian(lwing_yaw), DegreeToRadian(lwing_pitch), DegreeToRadian(lwing_roll));
	getpnt(DegreeToRadian(body_yaw), DegreeToRadian(body_pitch), DegreeToRadian(body_roll), -1, 0, 0.7454, lbody);
	getpnt(DegreeToRadian(lwing_yaw), DegreeToRadian(lwing_pitch), DegreeToRadian(lwing_roll), 0, -2.0, 0, lwing);
	
	glPushMatrix();
	glTranslated(lbody[0]-lwing[0], lbody[1]-lwing[1], lbody[2]-lwing[2]);
	glMultMatrixd(wing_l.rotateMatrix);

	{
		glBindTexture(GL_TEXTURE_2D, textureID[1]);
		glScaled(1.0, 2.0, 0.2);
		drawSphere(sliceCount, sliceCount, 1, ctr);
	}
	glPopMatrix();

	//�����ҳ��
	wing_r.resetMatrix();
	optRotate(wing_r, DegreeToRadian(rwing_yaw), DegreeToRadian(rwing_pitch), DegreeToRadian(rwing_roll));
	getpnt(DegreeToRadian(body_yaw), DegreeToRadian(body_pitch), DegreeToRadian(body_roll), 1, 0, 0.7454, rbody);
	getpnt(DegreeToRadian(rwing_yaw), DegreeToRadian(rwing_pitch), DegreeToRadian(rwing_roll), 0, -2.0, 0, rwing);
	
	glPushMatrix();
	glTranslated(rbody[0]-rwing[0], rbody[1]-rwing[1], rbody[2]-rwing[2]);
	glMultMatrixd(wing_r.rotateMatrix);
	

	{
		glBindTexture(GL_TEXTURE_2D, textureID[1]);
		glScaled(1.0, 2.0, 0.2);
		drawSphere(sliceCount, sliceCount, 1, ctr);
	}
	glPopMatrix();
	

    glutSwapBuffers();	//��������
	return;
}

void changeSize(int w, int h)
{
	if (h==0)
		h = 1;
	float ratio = w*1.0/h;
	
	arcBall.resetWindowSize(w, h);	//������������
	scaleController.resetWindowSize(w, h);
	glMatrixMode(GL_PROJECTION);	//ѡ�����ģʽΪͶӰ����
	glLoadIdentity();	//���þ���
	glViewport(0, 0, w, h);	//�����ӿ�(�ӿ����½�λ��, �ӿھ��εĿ��)
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);	//�趨ͶӰ����(�Ƕȣ��Ӿ����߱ȣ�
												//��z�᷽���������֮�����Ľ�����...��Զ����
	glMatrixMode(GL_MODELVIEW);	//ѡ�����ģʽΪģ����ͼ����
	glLoadIdentity();	//���þ���
	return;
}

//��ʼ��
void init(void)
{
	glShadeModel(GL_SMOOTH);	//�趨ƽ����ɫ
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	//�趨����ɫ
	glClearDepth(1.0f);	//�趨��Ȼ���
	glEnable(GL_DEPTH_TEST);	//������Ȳ���
	glDepthFunc(GL_LEQUAL);	//�趨��Ȳ��Ե�����
	return;
}

//��ʼ������
void textureInit(GLuint **id, int n, string *filename, GLenum colorType)
{
	if (*id != NULL)
		delete[] *id;
	*id = new GLuint[n];
	glEnable(GL_TEXTURE_2D);
	glGenTextures(n, *id);	//��������
	int i;
	BMPLoader loader;	//��ȡ����
	for (i=0; i<n; i++)
	{
		glBindTexture(GL_TEXTURE_2D, (*id)[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//��ȡ����
		if (!loader.loadImage(filename[i].data()))
			return;
		if (colorType == GL_RGB)
		{
			loader.BGRToRGB();
			glTexImage2D(GL_TEXTURE_2D, 0, colorType, loader.imgWidth, loader.imgHeight,
				0, colorType, GL_UNSIGNED_BYTE, loader.pImg);
		}
		else if (colorType == GL_RGBA)
		{
			loader.BGRToRGBA();
			glTexImage2D(GL_TEXTURE_2D, 0, colorType, loader.imgWidth, loader.imgHeight,
				0, colorType, GL_UNSIGNED_BYTE, loader.pImg);
		}
		loader.freeImage();
	}
	if (colorType == GL_RGBA)
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	return;
}


void CEasySerialAssistantDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	body_yaw_err += body_yaw;
	if (body_yaw_err >= 360)
		body_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedLwingErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	lwing_yaw_err += lwing_yaw;
	if (lwing_yaw_err >= 360)
		lwing_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedRwingErr()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	rwing_yaw_err += rwing_yaw;
	if (rwing_yaw_err >= 360)
		rwing_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedVideoBtn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	capture = cvCaptureFromAVI("a.avi");
	if (!capture)
	{
		AfxMessageBox("��Ƶ��ʧ��");
		return;
	}
	int fps = (int) cvGetCaptureProperty(capture, CV_CAP_PROP_FPS);
	SetTimer(VIDEO_PLAYER_EVENT, fps, NULL);
}


void CEasySerialAssistantDlg::OnTimer(UINT nIDEvent) 
{
    // TODO: Add your message handler code here and/or call default
    if(nIDEvent == VIDEO_PLAYER_EVENT)
    {
        IplImage* img = 0;  
        img = cvQueryFrame(capture); //������ͷ�����ļ���ץȡ������һ֡
        CvvImage m_CvvImage;  
        m_CvvImage.CopyOf(img, 1); //���Ƹ�֡ͼ��    
        m_CvvImage.DrawToHDC(hDC, &rect); //��ʾ���豸�ľ��ο���
    }
    CDialog::OnTimer(nIDEvent);
}
