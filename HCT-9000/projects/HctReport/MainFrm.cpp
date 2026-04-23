
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "HctReport.h"

#include "MainFrm.h"
#include "HctReportDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	ON_WM_CREATE()
	ON_COMMAND(ID_SEGMENT_LOADLINESCAN, &CMainFrame::OnSegmentLoadlinescan)
	ON_COMMAND(ID_SEGMENT_LOADCUBESCAN, &CMainFrame::OnSegmentLoadcubescan)
	ON_COMMAND(ID_LOADCUBESCAN_256X256, &CMainFrame::OnLoadcubescan256x256)
	ON_COMMAND(ID_LOADCUBESCAN_512X128, &CMainFrame::OnLoadcubescan512x128)
	ON_COMMAND(ID_LOADCUBESCAN_512X96, &CMainFrame::OnLoadcubescan512x96)
	ON_COMMAND(ID_DISCCUBESCAN_512X96, &CMainFrame::OnDisccubescan512x96)
	ON_COMMAND(ID_DISCCUBESCAN_512X128, &CMainFrame::OnDisccubescan512x128)
	ON_COMMAND(ID_DISCCUBESCAN_256X256, &CMainFrame::OnDisccubescan256x256)
	ON_COMMAND(ID_DISCCUBESCAN_1025X64, &CMainFrame::OnDisccubescan1025x64)
	ON_COMMAND(ID_CORNEACUBESCAN_512X96, &CMainFrame::OnCorneacubescan512x96)
	ON_COMMAND(ID_CORNEACUBESCAN_512X128, &CMainFrame::OnCorneacubescan512x128)
	ON_COMMAND(ID_CORNEARADIALSCAN_512X12, &CMainFrame::OnCornearadialscan512x12)
	ON_COMMAND(ID_CORNEARADIALSCAN_1024X12, &CMainFrame::OnCornearadialscan1024x12)
	ON_COMMAND(ID_DISCCUBESCAN_512X64, &CMainFrame::OnDisccubescan512x64)
	ON_COMMAND(ID_DISCRADIALSCAN_512X12, &CMainFrame::OnDiscradialscan512x12)
	ON_COMMAND(ID_DISCRADIALSCAN_1024X12, &CMainFrame::OnDiscradialscan1024x12)
	ON_COMMAND(ID_CORNEACUBESCAN_512X64, &CMainFrame::OnCorneacubescan512x64)
	ON_COMMAND(ID_MACULARWIDESCAN_512X96, &CMainFrame::OnMacularwidescan512x96)
	ON_COMMAND(ID_DISCWIDESCAN_512X96, &CMainFrame::OnDiscwidescan512x96)
	ON_COMMAND(ID_MACULARRADIALSCAN_512X12, &CMainFrame::OnMacularradialscan512x12)
	ON_COMMAND(ID_MACULARRADIALSCAN_1024X12, &CMainFrame::OnMacularradialscan1024x12)
	ON_COMMAND(ID_MACULARANGIOSCAN_384X384, &CMainFrame::OnMacularangioscan384x384)
	ON_COMMAND(ID_BIOMETER_WIDEANTERIOR, &CMainFrame::OnBiometerWideanterior)
	ON_COMMAND(ID_MACULARANGIOSCAN_512X512, &CMainFrame::OnMacularangioscan512x512)
	ON_COMMAND(ID_DISCANGIOSCAN_384X384, &CMainFrame::OnDiscangioscan384x384)
	ON_COMMAND(ID_DISCANGIOSCAN_512X512, &CMainFrame::OnDiscangioscan512x512)
	ON_COMMAND(ID_BIOMETER_ANTERIORLENS, &CMainFrame::OnBiometerAnteriorlens)
	ON_COMMAND(ID_BIOMETER_LENSTHICKNESS, &CMainFrame::OnBiometerLensthickness)
	ON_COMMAND(ID_MACULARWIDESCAN_512X128, &CMainFrame::OnMacularwidescan512x128)
	ON_COMMAND(ID_DISCWIDESCAN_512X128, &CMainFrame::OnDiscwidescan512x128)
	ON_COMMAND(ID_MACULARCUBESCAN_1024X128, &CMainFrame::OnMacularcubescan1024x128)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);


	return 0;
}


void CMainFrame::openNewDocumentOfPattern(PatternName name, int lineSize, int numLines, float rangeX, float rangeY)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_READONLY);

	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		CString strFolderPath = str.Mid(0, str.ReverseFind('\\'));

		this->patternName = name;
		this->lineSize = lineSize;
		this->numLines = numLines;
		this->rangeX = rangeX;
		this->rangeY = rangeY;
		this->patternPath = strFolderPath;
		this->_docType = CHctReportDoc::DocType::DOC_SCAN_SEGMENT;

		POSITION pos = theApp.GetFirstDocTemplatePosition();
		CDocTemplate *pTemplate = theApp.GetNextDocTemplate(pos);

		BeginWaitCursor();
		CHctReportDoc* pDoc = (CHctReportDoc*)pTemplate->OpenDocumentFile(NULL);
		EndWaitCursor();

		// New document is initialized with the file of given path via OnOpenDocument() called. 
		// CHctReportDoc* pDoc = (CHctReportDoc*)pTemplate;
		// CHctReportDoc *pTestDoc = pDoc->OpenDocumentFile(strFolderPath, lineSize, numLines);
	}
	return;
}


void CMainFrame::openNewDocumentOfWideAnterior(void)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_READONLY);

	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		CString strFolderPath = str.Mid(0, str.ReverseFind('\\'));

		this->patternPath = strFolderPath;
		this->_docType = CHctReportDoc::DocType::DOC_WIDE_ANTERIOR;

		POSITION pos = theApp.GetFirstDocTemplatePosition();
		CDocTemplate *pTemplate = theApp.GetNextDocTemplate(pos);

		BeginWaitCursor();
		CHctReportDoc* pDoc = (CHctReportDoc*)pTemplate->OpenDocumentFile(NULL);
		EndWaitCursor();
	}
	return;
}


void CMainFrame::openNewDocumentOfAnteriorLens(void)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_READONLY);

	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		CString strFolderPath = str.Mid(0, str.ReverseFind('\\'));

		this->patternPath = strFolderPath;
		this->_docType = CHctReportDoc::DocType::DOC_ANTERIOR_LENS;

		POSITION pos = theApp.GetFirstDocTemplatePosition();
		CDocTemplate *pTemplate = theApp.GetNextDocTemplate(pos);

		BeginWaitCursor();
		CHctReportDoc* pDoc = (CHctReportDoc*)pTemplate->OpenDocumentFile(NULL);
		EndWaitCursor();
	}
	return;
}


void CMainFrame::openNewDocumentOfLensThickness(void)
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_READONLY);

	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		CString strFolderPath = str.Mid(0, str.ReverseFind('\\'));

		this->patternPath = strFolderPath;
		this->_docType = CHctReportDoc::DocType::DOC_LENS_THICKNESS;

		POSITION pos = theApp.GetFirstDocTemplatePosition();
		CDocTemplate *pTemplate = theApp.GetNextDocTemplate(pos);

		BeginWaitCursor();
		CHctReportDoc* pDoc = (CHctReportDoc*)pTemplate->OpenDocumentFile(NULL);
		EndWaitCursor();
	}
	return;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSegmentLoadlinescan()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::MacularLine, 512, 1);
}


void CMainFrame::OnSegmentLoadcubescan()
{
	// TODO: Add your command handler code here
	
}


void CMainFrame::OnLoadcubescan256x256()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 256, 256, 6.0f);
}


void CMainFrame::OnLoadcubescan512x128()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 512, 128, 6.0f);
}

void CMainFrame::OnMacularcubescan1024x128()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 1024, 128, 6.0f);
}


void CMainFrame::OnLoadcubescan512x96()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 512, 96);
}

void CMainFrame::OnDisccubescan512x64()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 512, 64);
}


void CMainFrame::OnDisccubescan512x96()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 512, 96);
}


void CMainFrame::OnDisccubescan512x128()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 512, 128);
}


void CMainFrame::OnDisccubescan256x256()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 256, 256, 9.0f);
}


void CMainFrame::OnDisccubescan1025x64()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 1024, 64);
}


void CMainFrame::OnCorneacubescan512x64()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Anterior3D, 512, 64);
}


void CMainFrame::OnCorneacubescan512x96()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Anterior3D, 512, 96);
}


void CMainFrame::OnCorneacubescan512x128()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Anterior3D, 512, 128);
}


void CMainFrame::OnCornearadialscan512x12()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::AnteriorRadial, 512, 12);
}


void CMainFrame::OnCornearadialscan1024x12()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::AnteriorRadial, 1024, 12);
}


void CMainFrame::OnDiscradialscan512x12()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::DiscRadial, 512, 12);
}


void CMainFrame::OnDiscradialscan1024x12()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::DiscRadial, 1024, 12);
}

void CMainFrame::OnMacularwidescan512x96()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 512, 96, 12.0f, 9.0f);
}

void CMainFrame::OnMacularwidescan512x128()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Macular3D, 512, 128, 12.0f, 9.0f);
}


void CMainFrame::OnDiscwidescan512x96()
{
	// TODO: Add your command handler code here
	openNewDocumentOfPattern(PatternName::Disc3D, 512, 96, 12.0f, 9.0f);
}


void CMainFrame::OnDiscwidescan512x128()
{
	openNewDocumentOfPattern(PatternName::Disc3D, 512, 128, 12.0f, 9.0f);
}



void CMainFrame::OnMacularradialscan512x12()
{
	openNewDocumentOfPattern(PatternName::MacularRadial, 512, 12);
}


void CMainFrame::OnMacularradialscan1024x12()
{
	openNewDocumentOfPattern(PatternName::MacularRadial, 1024, 12, 9.0f);
}


void CMainFrame::OnMacularangioscan384x384()
{
	openNewDocumentOfPattern(PatternName::MacularAngio, 384, 384, 4.5f);
}


void CMainFrame::OnMacularangioscan512x512()
{
	openNewDocumentOfPattern(PatternName::MacularAngio, 512, 512, 4.5f);
}


void CMainFrame::OnDiscangioscan384x384()
{
	openNewDocumentOfPattern(PatternName::DiscAngio, 384, 384, 4.5f);
}


void CMainFrame::OnDiscangioscan512x512()
{
	openNewDocumentOfPattern(PatternName::DiscAngio, 512, 512, 4.5f);
}


void CMainFrame::OnBiometerWideanterior()
{
	// TODO: Add your command handler code here
	openNewDocumentOfWideAnterior();
}


void CMainFrame::OnBiometerAnteriorlens()
{
	// TODO: Add your command handler code here
	openNewDocumentOfAnteriorLens();
}


void CMainFrame::OnBiometerLensthickness()
{
	// TODO: Add your command handler code here
	openNewDocumentOfLensThickness();
}


