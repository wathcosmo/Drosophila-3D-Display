// EasySerialAssistantDlg.cpp : implementation file
///////////////////////////////////////////////////////////////////////////////////////
///多线程串口通信实现文件
///BUG修正中                                                                  
//此程序不得用于商业用途
//串口线程部分程序参考互联网,如牵涉版权,请与作者联系
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

int sliceCount = 20;	//曲面分片数

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
	glutInit(&argc, argv);	//glut初始化
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);	//设定显示模式
    glutInitWindowPosition(300, 100);	//设定窗口在屏幕中的位置
    glutInitWindowSize(400, 400);	//设定窗口宽高
    glutCreateWindow("OpenGL");	//创建窗口
	init();	//初始化
	textureInit(&textureID, textureCount, textureFile, GL_RGB);	//初始化纹理

	glutReshapeFunc(&changeSize);	//注册窗口大小改变函数
    glutDisplayFunc(&display);	//注册渲染函数
	glutIdleFunc(&display);	//注册无事件发生时的函数
    glutMainLoop();	//开始

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

	//初始化默认参数
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

	//添加端口号
	CString szComPort="";
	for(i=0;i<20;i++)
	{
		szComPort.Format("COM%d",i+1);
		((CComboBox *)GetDlgItem(IDC_BODY_PORT))->InsertString(i,szComPort);
		((CComboBox *)GetDlgItem(IDC_LWING_PORT))->InsertString(i,szComPort);
		((CComboBox *)GetDlgItem(IDC_RWING_PORT))->InsertString(i,szComPort);
	}

	//默认选项
	((CComboBox *)GetDlgItem(IDC_BODY_PORT))->SetCurSel(body_Port-1);
	((CComboBox *)GetDlgItem(IDC_LWING_PORT))->SetCurSel(lwing_Port-1);
	((CComboBox *)GetDlgItem(IDC_RWING_PORT))->SetCurSel(rwing_Port-1); 

	//串口数据变量初始化
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

	//视频控件初始化
	pStc = (CStatic *)GetDlgItem(IDC_IMG_DISPLAY); //获取Picture控件
    pStc->GetClientRect(&rect); //将CWind类客户区的坐标点传给矩形
    pDC = pStc->GetDC(); //得到Picture控件设备上下文
    hDC = pDC->GetSafeHdc(); //得到控件设备上下文的句柄 

	//创建线程
	m_pWinThread = AfxBeginThread(ProDispThread,this); //创建UI线程
	m_pWinThread->m_bAutoDelete = true;

	openglThread = AfxBeginThread(openglProc,GetSafeHwnd()); //创建3D模型线程
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
	//开通或关闭串口通信时会引发此消息
	//::AfxMessageBox("Break!");
}

//串口接收事件中断
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
		if(pDlg->body_bAutoClearn == false) //没有选中自动清空
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
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"接收缓冲溢出!");
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
		if(pDlg->lwing_bAutoClearn == false) //没有选中自动清空
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
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"接收缓冲溢出!");
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
		if(pDlg->rwing_bAutoClearn == false) //没有选中自动清空
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
		//pDlg->SetDlgItemText(IDC_STATIC_CUR_MSG,"接收缓冲溢出!");
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
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"串口关闭");
		SetDlgItemText(IDC_BTN_PORTOPEN,"已关闭");
		return;
	}
	else
	{
		m_bPortOpen=true;
		SetDlgItemText(IDC_BTN_PORTOPEN,"已打开");
	}

	if(!(body_ComPort.Open(body_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) || 
		!(lwing_ComPort.Open(lwing_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) ||
		!(rwing_ComPort.Open(rwing_Port,CComPort::AutoReceiveBySignal,m_DwBaud,m_Parity,m_DataBits,m_Stopbits)) )
	{
		m_bPortOpen=false;
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"串口打开失败");
		::MessageBox(::GetForegroundWindow(), "串口打开失败", "提示信息", MB_ICONWARNING | MB_OK);
		
		HICON hIcon=::LoadIcon(AfxGetResourceHandle(),MAKEINTRESOURCE(IDI_ICON_RED));
		((CStatic *)GetDlgItem(IDC_STATIC_COMSTATE))->SetIcon(hIcon);
	}
	else
	{
		m_bPortOpen=true;
		//SetDlgItemText(IDC_STATIC_CUR_MSG,"串口打开成功");
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

//---------------AHRS 数据接口部分------------------------------------------------

//uart reicer flag
#define b_uart_head  0x80  //收到A5 头 标志位
#define b_rx_over    0x40  //收到完整的帧标志
// USART Receiver buffer
#define RX_BUFFER_SIZE 100 //接收缓冲区字节数

//void Decode_frame(unsigned char data);
volatile unsigned char rx_buffer[RX_BUFFER_SIZE]; //接收数据缓冲区
volatile unsigned char rx_wr_index; //缓冲写指针
volatile unsigned char RC_Flag;  //接收状态标志字节

//解算后的角度值
float yaw = 0,  //偏航角
pitch = 0,//俯仰
roll = 0; //滚转
float body_yaw, body_pitch, body_roll;
float lwing_yaw, lwing_pitch, lwing_roll;
float rwing_yaw, rwing_pitch, rwing_roll;
float body_yaw_err = 0, lwing_yaw_err = 0, rwing_yaw_err = 0; //偏航误差


//在接收完一帧IMU姿态报告后，调用这个子程序来取出姿态数据
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
	yaw=(float)temp / 10.0f; //偏航角
	
	
	temp = 0;
	temp = rx_buffer[4];
	temp <<= 8;
	temp |= rx_buffer[5];
	if(temp&0x8000){
	temp = 0-(temp&0x7fff);
	}else temp = (temp&0x7fff);
	pitch=(float)temp / 10.0f;//俯仰
	
	temp = 0;
	temp = rx_buffer[6];
	temp <<= 8;
	temp |= rx_buffer[7];
	if(temp&0x8000){
	temp = 0-(temp&0x7fff);
	}else temp = (temp&0x7fff);
	roll=(float)temp / 10.0f;//滚转
}



//--校验当前接收到的一帧数据是否 与帧校验字节一致
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
		RC_Flag|=b_uart_head; //如果接收到A5 置位帧头标专位
		rx_buffer[rx_wr_index++]=data; //保存这个字节.
	}
	else if(data==0x5a)
	{ 
		if(RC_Flag&b_uart_head) //如果上一个字节是A5 那么认定 这个是帧起始字节
		{ rx_wr_index=0;  //重置 缓冲区指针
		RC_Flag&=~b_rx_over; //这个帧才刚刚开始收
		}
		else //上一个字节不是A5
			rx_buffer[rx_wr_index++]=data;
		RC_Flag&=~b_uart_head; //清帧头标志
	}
	   else
	   { rx_buffer[rx_wr_index++]=data;
	   RC_Flag&=~b_uart_head;
	   if(rx_wr_index==rx_buffer[0]) //收够了字节数.
	   {  
		   RC_Flag|=b_rx_over; //置位 接收完整的一帧数据
		  
	   }
	   }
	   
	   if(rx_wr_index==RX_BUFFER_SIZE) //防止缓冲区溢出
  rx_wr_index--;


}
//---------------------------------------------------------------------
//======================================================================

//显示线程
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
		
		if(pParlPro->body_LastIndex > 0) //是否有数据接收到
		{
			//AHRS解帧程序[开始]
			for(int i=0;i<(int)pParlPro->body_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->body_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //已经接收完一帧?
					RC_Flag&=~b_rx_over; //清标志先
					if(Sum_check()){ 
						//校验通过
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU 的数据
							UART2_Get_IMU();	//取数据
							//更新显示
							body_yaw = yaw;
							body_pitch = -pitch;
							body_roll = -roll;
							body_yaw -= body_yaw_err;
							if (body_yaw < 0) 
								body_yaw += 360;
							str.Format("俯仰角:%g",body_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch))->SetWindowText(str);
							str.Format("侧翻角:%g",body_roll);
							(pParlPro->GetDlgItem(IDC_Roll))->SetWindowText(str);
							str.Format("偏航角:%.1f",body_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw))->SetWindowText(str);
						}
					}//校验是否通过?
				}
			}//AHRS解帧程序 [结束]
		}

		if(pParlPro->lwing_LastIndex > 0) //是否有数据接收到
		{
			//AHRS解帧程序[开始]
			for(int i=0;i<(int)pParlPro->lwing_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->lwing_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //已经接收完一帧?
					RC_Flag&=~b_rx_over; //清标志先
					if(Sum_check()){ 
						//校验通过
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU 的数据
							UART2_Get_IMU();	//取数据
							//更新显示
							lwing_yaw = yaw;
							lwing_pitch = pitch;
							lwing_roll = roll;
							lwing_yaw -= lwing_yaw_err;
							if (lwing_yaw < 0) 
								lwing_yaw += 360;
							str.Format("俯仰角:%g",lwing_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch2))->SetWindowText(str);
							str.Format("侧翻角:%g",lwing_roll);
							(pParlPro->GetDlgItem(IDC_Roll2))->SetWindowText(str);
							str.Format("偏航角:%.1f",lwing_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw2))->SetWindowText(str);
						}
					}//校验是否通过?
				}
			}//AHRS解帧程序 [结束]
		}

		if(pParlPro->rwing_LastIndex > 0) //是否有数据接收到
		{
			//AHRS解帧程序[开始]
			for(int i=0;i<(int)pParlPro->rwing_LastIndex;i++)
			{
				OnAhrs_Data(pParlPro->rwing_RecvBuf[i]);
				CString str;
				if(RC_Flag&b_rx_over){  //已经接收完一帧?
					RC_Flag&=~b_rx_over; //清标志先
					if(Sum_check()){ 
						//校验通过
						if(rx_buffer[1]==0xA1){ //UART2_ReportIMU 的数据
							UART2_Get_IMU();	//取数据
							//更新显示
							rwing_yaw = yaw;
							rwing_pitch = pitch;
							rwing_roll = roll;
							rwing_yaw -= rwing_yaw_err;
							if (rwing_yaw < 0) 
								rwing_yaw += 360;
							str.Format("俯仰角:%g",rwing_pitch);
							(pParlPro->GetDlgItem(IDC_Pitch3))->SetWindowText(str);
							str.Format("侧翻角:%g",rwing_roll);
							(pParlPro->GetDlgItem(IDC_Roll3))->SetWindowText(str);
							str.Format("偏航角:%.1f",rwing_yaw);
							(pParlPro->GetDlgItem(IDC_Yaw3))->SetWindowText(str);
						}
					}//校验是否通过?
				}
			}//AHRS解帧程序 [结束]
		}

	}
    
    pParlPro->body_bThread=false; //工作标志复位
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





/****************************OpenGL 函数定义********************************/

// 获取旋转参数。yaw, pitch, roll 为弧度。
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

//角度转弧度
auto DegreeToRadian = [](float degree) { return degree / 180.0 * M_PI; };

//绘制函数
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//清除缓存	
	glMatrixMode(GL_MODELVIEW);	//选择矩阵模式为模型视图矩阵
	
	glLoadIdentity();	//重置模型观察矩阵
	gluLookAt(
		0, 0, 20,	//定义视图矩阵，眼睛的坐标
		0, 0, 0,			//眼睛朝向的坐标
		0, 1, 0//头顶朝上的方向
	);			
	float ctr[3] = {0, 0, 0};
	double rbody[3], lbody[3], rwing[3], lwing[3];

	

	//绘制身体
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

	//绘制左翅膀
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

	//绘制右翅膀
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
	

    glutSwapBuffers();	//交换缓存
	return;
}

void changeSize(int w, int h)
{
	if (h==0)
		h = 1;
	float ratio = w*1.0/h;
	
	arcBall.resetWindowSize(w, h);	//重新设置中心
	scaleController.resetWindowSize(w, h);
	glMatrixMode(GL_PROJECTION);	//选择矩阵模式为投影矩阵
	glLoadIdentity();	//重置矩阵
	glViewport(0, 0, w, h);	//设置视口(视口左下角位置, 视口矩形的宽高)
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);	//设定投影矩阵(角度，视镜体宽高比，
												//沿z轴方向的两截面之间距离的近处，...的远处）
	glMatrixMode(GL_MODELVIEW);	//选择矩阵模式为模型视图矩阵
	glLoadIdentity();	//重置矩阵
	return;
}

//初始化
void init(void)
{
	glShadeModel(GL_SMOOTH);	//设定平滑着色
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	//设定背景色
	glClearDepth(1.0f);	//设定深度缓存
	glEnable(GL_DEPTH_TEST);	//开启深度测试
	glDepthFunc(GL_LEQUAL);	//设定深度测试的类型
	return;
}

//初始化纹理
void textureInit(GLuint **id, int n, string *filename, GLenum colorType)
{
	if (*id != NULL)
		delete[] *id;
	*id = new GLuint[n];
	glEnable(GL_TEXTURE_2D);
	glGenTextures(n, *id);	//生成纹理
	int i;
	BMPLoader loader;	//读取纹理
	for (i=0; i<n; i++)
	{
		glBindTexture(GL_TEXTURE_2D, (*id)[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//读取纹理
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
	// TODO: 在此添加控件通知处理程序代码
	body_yaw_err += body_yaw;
	if (body_yaw_err >= 360)
		body_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedLwingErr()
{
	// TODO: 在此添加控件通知处理程序代码
	lwing_yaw_err += lwing_yaw;
	if (lwing_yaw_err >= 360)
		lwing_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedRwingErr()
{
	// TODO: 在此添加控件通知处理程序代码
	rwing_yaw_err += rwing_yaw;
	if (rwing_yaw_err >= 360)
		rwing_yaw_err -= 360;
}


void CEasySerialAssistantDlg::OnBnClickedVideoBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	capture = cvCaptureFromAVI("a.avi");
	if (!capture)
	{
		AfxMessageBox("视频打开失败");
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
        img = cvQueryFrame(capture); //从摄像头或者文件中抓取并返回一帧
        CvvImage m_CvvImage;  
        m_CvvImage.CopyOf(img, 1); //复制该帧图像    
        m_CvvImage.DrawToHDC(hDC, &rect); //显示到设备的矩形框内
    }
    CDialog::OnTimer(nIDEvent);
}
