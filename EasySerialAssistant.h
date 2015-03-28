// EasySerialAssistant.h : main header file for the EASYSERIALASSISTANT application
//

#if !defined(AFX_EASYSERIALASSISTANT_H__DBE82974_6FD6_4D97_BA14_38558D39170B__INCLUDED_)
#define AFX_EASYSERIALASSISTANT_H__DBE82974_6FD6_4D97_BA14_38558D39170B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CEasySerialAssistantApp:
// See EasySerialAssistant.cpp for the implementation of this class
//

class CEasySerialAssistantApp : public CWinApp
{
public:
	CEasySerialAssistantApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEasySerialAssistantApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CEasySerialAssistantApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EASYSERIALASSISTANT_H__DBE82974_6FD6_4D97_BA14_38558D39170B__INCLUDED_)
