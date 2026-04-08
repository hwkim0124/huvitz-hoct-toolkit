// DlgOriginalLive.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "HctToolkit.h"
#include "DlgOriginalLive.h"
#include "afxdialogex.h"

#include "OctSystem2.h"
#include "CppUtil2.h"
#include "OctResult2.h"
#include "ImgProc2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctResult;
using namespace ImgProc;
using namespace CppUtil;
using namespace std;


// DlgOriginalLive 대화 상자입니다.

IMPLEMENT_DYNAMIC(DlgOriginalLive, CDialogEx)

DlgOriginalLive::DlgOriginalLive(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLGORIGINALLIVE, pParent)
	, m_Size(_T(""))
	, m_Fps(_T(""))
{

}

DlgOriginalLive::~DlgOriginalLive()
{
}

void DlgOriginalLive::initControls(void)
{
	CRuntimeClass* pObject;
	pObject = RUNTIME_CLASS(ViewScrollColorCamera);
	liveView = (ViewScrollColorCamera*)pObject->CreateObject();

	//ratio = 100.0f;

	Controller::setColorCameraImageCallback(liveView->getImageCallbackFunction());
	Controller::setColorCameraFrameCallback(liveView->getFrameCallbackFunction());
	
	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();

	liveWidth = Controller::getMainBoard()->getColorCamera()->getFrameWidth();
	liveHeight = Controller::getMainBoard()->getColorCamera()->getFrameHeight();
	liveView->setInitSize(liveWidth, liveHeight);

	CRect rect;
	GetClientRect(rect);

	liveView->Create(0, 0, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW, CRect(5, 5, rect.right - 10, rect.bottom - 60), this, IDC_VIEW);
	liveView->ModifyStyle(0, SS_NOTIFY);

	liveView->ShowScrollBar(SB_BOTH);
	liveView->EnableScrollBar(SB_BOTH);

	//colorCamera.initialize();

	return;
}


void DlgOriginalLive::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ZOOM, m_Size);
	DDV_MaxChars(pDX, m_Size, 10);
	DDX_Text(pDX, IDC_FPS, m_Fps);
	DDV_MaxChars(pDX, m_Fps, 10);
}


BEGIN_MESSAGE_MAP(DlgOriginalLive, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_START, &DlgOriginalLive::OnBnClickedStart)
	ON_BN_CLICKED(IDC_ZOOMOUT, &DlgOriginalLive::OnBnClickedZoomout)
	ON_BN_CLICKED(IDC_ZOOMIN, &DlgOriginalLive::OnBnClickedZoomin)
END_MESSAGE_MAP()


// DlgOriginalLive 메시지 처리기입니다.


BOOL DlgOriginalLive::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	initControls();
	isLive = false;
	initiated = true;
	return TRUE;
}

void DlgOriginalLive::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	if (isLive) {
		KillTimer(1);
		isLive = false;
		//camera->pauseLiveMode();
		SetDlgItemText(IDC_START, _T("Start Live"));
	}
	Controller::setColorCameraImageCallback(nullptr);
	Controller::setColorCameraFrameCallback(nullptr);
	Controller::getMainBoard()->getColorCamera()->pauseOriginalMode();
	liveView->DestroyWindow();
	initControls();

	CDialogEx::OnClose();
}

void DlgOriginalLive::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	if (bShow) {
	}
	return;
}


void DlgOriginalLive::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (initiated) {
		liveView->MoveWindow(5, 5, cx - 10, cy - 60, true);
	}
}

void DlgOriginalLive::OnTimer(UINT_PTR nIDEvent)
{
	ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	//frameCount++;
	if (nIDEvent == 1) {
		camera->startOriginalMode();
		timeCount = (float)liveView->getTimeCount();
		fps = (float)(1 / (timeCount / 1000.f));
		m_Fps.Format(_T("%.2f fps"), fps);
		SetDlgItemText(IDC_FPS, m_Fps);
	}
	CDialogEx::OnTimer(nIDEvent);
}

BOOL DlgOriginalLive::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (zDelta < 0)
	{
		// 마우스 휠 아래로...
		liveView->zoomOut();
		m_Size.Format(_T("%.3f %%"), calcRatio());
		SetDlgItemText(IDC_ZOOM, m_Size);
	}

	if (zDelta > 0)
	{
		// 마우스 휠 위로...
		liveView->zoomIn();
		m_Size.Format(_T("%.3f %%"), calcRatio());
		SetDlgItemText(IDC_ZOOM, m_Size);
	}
	return TRUE;
}

void DlgOriginalLive::OnBnClickedStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BeginWaitCursor();
	//ColorCamera* camera = Controller::getMainBoard()->getColorCamera();
	if (isLive) {
		Controller::setColorCameraImageCallback(nullptr);
		Controller::setColorCameraFrameCallback(nullptr);
		liveView->useLiveMode(true, false);
		//camera->pauseLiveMode();
		Controller::getMainBoard()->getColorCamera()->pauseOriginalMode();
		SetDlgItemText(IDC_START, _T("Start Live"));
		m_Size.Format(_T("%.3f %%"), calcRatio());
		SetDlgItemText(IDC_ZOOM, m_Size);
		isLive = false;
		KillTimer(1);
	}
	else {
		Controller::setColorCameraImageCallback(liveView->getImageCallbackFunction());
		Controller::setColorCameraFrameCallback(liveView->getFrameCallbackFunction());
		liveView->useLiveMode(true, true);
		//camera->startOriginalMode();
		SetDlgItemText(IDC_START, _T("Pause Live"));
		m_Size.Format(_T("%.3f %%"), calcRatio());
		SetDlgItemText(IDC_ZOOM, m_Size);
		isLive = true;
		SetTimer(1, 1, NULL);
	}
	EndWaitCursor();
	return;
}

void DlgOriginalLive::OnBnClickedZoomout()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	liveView->zoomOut();
	m_Size.Format(_T("%.3f %%"), calcRatio());
	SetDlgItemText(IDC_ZOOM, m_Size);
}


void DlgOriginalLive::OnBnClickedZoomin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	liveView->zoomIn();
	m_Size.Format(_T("%.3f %%"), calcRatio());
	SetDlgItemText(IDC_ZOOM, m_Size);
}


void DlgOriginalLive::setParameter(bool flagMaskROI, int xMaskROI, int yMaskROI, int radiusMaskROI, bool flagWhiteBalance, float WBParam1, float WBParam2)
{
	liveView->useMaskROI(true, flagMaskROI, xMaskROI, yMaskROI, radiusMaskROI);
	liveView->useRadialColorCorrection(true, false);
	liveView->useAutoWhiteBalance(true, flagWhiteBalance);
	liveView->setWhiteBalanceParameters(WBParam1, WBParam2);
}

float DlgOriginalLive::calcRatio(void)
{
	float ratio = ((float)liveView->getViewWidth() * 100) / (float)liveWidth;
	return ratio;
}

