
// HctModelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HctModel.h"
#include "HctModelDlg.h"
#include "afxdialogex.h"

#include "SemtSegm2.h"

using namespace SemtSegm;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace InferenceEngine;

#if defined(ENABLE_UNICODE_PATH_SUPPORT) && defined(_WIN32)
	#define tcout std::wcout
	#define file_name_t std::wstring
	#define imread_t imreadW
	#define ClassificationResult_t ClassificationResultW
#else
	#define tcout std::cout
	#define file_name_t std::string
	#define imread_t cv::imread
	#define ClassificationResult_t ClassificationResult
#endif


#if defined(ENABLE_UNICODE_PATH_SUPPORT) && defined(_WIN32)
std::string simpleConvert(const std::wstring & wstr) {
	std::string str;
	for (auto && wc : wstr)
		str += static_cast<char>(wc);
	return str;
}

#endif 

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data	
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHctModelDlg dialog



CHctModelDlg::CHctModelDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_HCTMODEL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHctModelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHctModelDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CHctModelDlg message handlers

BOOL CHctModelDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	auto bscan = std::make_shared<OpticDiscBscan>();
	// bscan->performAnalysis();

	// TODO: Add extra initialization here
	CNNNetwork network2;
	Core ie;

	std::vector<std::string> availableDevices = ie.GetAvailableDevices();

	const file_name_t input_model{ _T("saved_model.xml") };

	CNNNetwork network = ie.ReadNetwork(input_model);

	/** Take information about all topology inputs **/
	InferenceEngine::InputsDataMap input_info = network.getInputsInfo();
	/** Take information about all topology outputs **/
	InferenceEngine::OutputsDataMap output_info = network.getOutputsInfo();

	if (network.getOutputsInfo().size() != 1) throw std::logic_error("Sample supports topologies with 1 output only");
	if (network.getInputsInfo().size() != 1) throw std::logic_error("Sample supports topologies with 1 input only");

		/** Iterate over all input info**/
	for (auto &item : input_info) {
		auto input_data = item.second;
		input_data->setPrecision(InferenceEngine::Precision::FP32);
		input_data->setLayout(InferenceEngine::Layout::NCHW);
		// input_data->getPreProcess().setResizeAlgorithm(InferenceEngine::RESIZE_BILINEAR);
		// input_data->getPreProcess().setColorFormat(InferenceEngine::ColorFormat::RGB);
	}

	/** Iterate over all output info**/
	for (auto &item : output_info) {
		auto output_data = item.second;
		output_data->setPrecision(InferenceEngine::Precision::FP32);
		// output_data->setLayout(InferenceEngine::Layout::NC);
	}

	ExecutableNetwork executable_network = ie.LoadNetwork(network, "CPU");

	auto infer_request = executable_network.CreateInferRequest();

	/** Iterate over all input blobs **/
	for (auto & item : input_info) {
		auto input_name = item.first;
		/** Get input blob **/
		auto input = infer_request.GetBlob(input_name);
		/** Fill input tensor with planes. First b channel, then g and r channels **/
		//     ...
		auto desc = input->getTensorDesc();

		int a = 0;
		a = 1;
	}

	/** Iterate over all input blobs **/
	for (auto & item : input_info) {
		auto input_data = item.second;
		/** Create input blob **/
		InferenceEngine::TBlob<float>::Ptr input;
		auto dims = input_data->getTensorDesc().getDims();
		auto layout = input_data->getTensorDesc().getLayout();
		// assuming input precision was asked to be U8 in prev step
		input = InferenceEngine::make_shared_blob<float>(
			InferenceEngine::TensorDesc(InferenceEngine::Precision::FP32, 
				dims, layout));
		input->allocate();
		infer_request.SetBlob(item.first, input);

		/** Fill input tensor with planes. First b channel, then g and r channels **/
		//     ...
	}

	infer_request.Infer();

	for (auto &item : output_info) {
		auto output_name = item.first;
		auto output = infer_request.GetBlob(output_name);
		auto desc = output->getTensorDesc();
		{
			auto const memLocker = output->cbuffer(); // use const memory locker
													  // output_buffer is valid as long as the lifetime of memLocker
			const float *output_buffer = memLocker.as<const float *>();
			/** output_buffer[] - accessing output blob data **/
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHctModelDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHctModelDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHctModelDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

