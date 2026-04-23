
// MainFrm.h : interface of the CMainFrame class
//

#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <sstream>

using namespace std;

#include "HctReportDoc.h"


class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	PatternName patternName;
	CString patternPath;
	int lineSize;
	int numLines;
	float rangeX;
	float rangeY;

	CHctReportDoc::DocType _docType;

// Operations
public:
	void openNewDocumentOfPattern(PatternName name, int lineSize, int numLines, float rangeX=6.0f, float rangeY=6.0f);
	void openNewDocumentOfWideAnterior(void);
	void openNewDocumentOfAnteriorLens(void);
	void openNewDocumentOfLensThickness(void);

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CToolBar          m_wndToolBar;
	CStatusBar        m_wndStatusBar;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSegmentLoadlinescan();
	afx_msg void OnSegmentLoadcubescan();
	afx_msg void OnLoadcubescan256x256();
	afx_msg void OnLoadcubescan512x128();
	afx_msg void OnLoadcubescan512x96();
	afx_msg void OnDisccubescan512x96();
	afx_msg void OnDisccubescan512x128();
	afx_msg void OnDisccubescan256x256();
	afx_msg void OnDisccubescan1025x64();
	afx_msg void OnCorneacubescan512x96();
	afx_msg void OnCorneacubescan512x128();
	afx_msg void OnCornearadialscan512x12();
	afx_msg void OnCornearadialscan1024x12();
	afx_msg void OnDisccubescan512x64();
	afx_msg void OnDiscradialscan512x12();
	afx_msg void OnDiscradialscan1024x12();
	afx_msg void OnCorneacubescan512x64();
	afx_msg void OnMacularwidescan512x96();
	afx_msg void OnDiscwidescan512x96();
	afx_msg void OnMacularradialscan512x12();
	afx_msg void OnMacularradialscan1024x12();
	afx_msg void OnMacularangioscan384x384();
	afx_msg void OnBiometerWideanterior();
	afx_msg void OnMacularangioscan512x512();
	afx_msg void OnDiscangioscan384x384();
	afx_msg void OnDiscangioscan512x512();
	afx_msg void OnBiometerAnteriorlens();
	afx_msg void OnBiometerLensthickness();
	afx_msg void OnMacularwidescan512x128();
	afx_msg void OnDiscwidescan512x128();
	afx_msg void OnMacularcubescan1024x128();
};


