
// HctReportDoc.cpp : implementation of the CHctReportDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "HctReport.h"
#endif

#include "HctReportDoc.h"
#include "MainFrm.h"

#include <boost/format.hpp>

#include "CppUtil2.h"
#include "SemtSegm2.h"

using namespace CppUtil;
using namespace std;

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CHctReportDoc

IMPLEMENT_DYNCREATE(CHctReportDoc, CDocument)

BEGIN_MESSAGE_MAP(CHctReportDoc, CDocument)
END_MESSAGE_MAP()


// CHctReportDoc construction/destruction

CHctReportDoc::CHctReportDoc()
{
	// TODO: add one-time construction code here

}


CHctReportDoc::~CHctReportDoc()
{
}


BOOL CHctReportDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)
	auto p = (CMainFrame*)AfxGetMainWnd();
	auto name = p->patternName;
	auto path = p->patternPath;
	auto lineSize = p->lineSize;
	auto numLines = p->numLines;
	_docType = p->_docType;

	// TODO:  Add your specialized creation code here
	if (isScanSegment()) {
		if (name == PatternName::Macular3D || name == PatternName::Disc3D || name == PatternName::Anterior3D ||
			name == PatternName::MacularAngio) {
			openCubeScanData(path);
		}
		else if (name == PatternName::MacularRadial || name == PatternName::DiscRadial || name == PatternName::AnteriorRadial) {
			openRadialScanData(path, lineSize, numLines);
		}
		else {
			openLineScanData(path);
		}
	}
	else if (isWideAnterior()) {
		openWideAnteriorData(path);
	}
	else if (isAnteriorLens()) {
		openAnteriorLensData(path);
	}
	else if (isLensThickness()) {
		openLensThicknessData(path);
	}
	return TRUE;
}


// CHctReportDoc serialization

void CHctReportDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

#ifdef SHARED_HANDLERS

// Support for thumbnails
void CHctReportDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// Modify this code to draw the document's data
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// Support for Search Handlers
void CHctReportDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// Set search contents from document's data. 
	// The content parts should be separated by ";"

	// For example:  strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CHctReportDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CHctReportDoc diagnostics

#ifdef _DEBUG
void CHctReportDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHctReportDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CHctReportDoc commands


void CHctReportDoc::openWideAnteriorData(LPCTSTR path)
{
	auto pMain = (CMainFrame*)AfxGetMainWnd();

	auto path1 = wstring(path) + L"\\cornea";
	auto path2 = wstring(path) + L"\\chamber";
	auto path3 = wstring(path) + L"\\outs";
	auto files1 = SystemFunc::findFilesInDirectory(path1, _T("*.jpg"));
	auto files2 = SystemFunc::findFilesInDirectory(path2, _T("*.jpg"));

	int size = min(files1.size(), files2.size());
	for (int i = 0; i < size; i++) {
		LogD() << "Wide anterior images: " << wtoa(files1[i]) << ", " << wtoa(files2[i]);
		auto fname = files1[i].substr(files1[i].rfind('\\')+1);

		CorTopo::WideCreator creator;
		if (creator.loadImages(files1[i], files2[i])) {
			creator.process();
			creator.saveWideAnteriorImage(path3, fname);
			_wides.push_back(move(creator));
		}
	}
	return;
}


void CHctReportDoc::openAnteriorLensData(LPCTSTR path)
{
	auto pMain = (CMainFrame*)AfxGetMainWnd();

	auto path1 = wstring(path) + L"\\cornea";
	auto path2 = wstring(path) + L"\\retina";
	auto files1 = SystemFunc::findFilesInDirectory(path1, _T("*.jpg"));
	auto files2 = SystemFunc::findFilesInDirectory(path2, _T("*.jpg"));

	for (int i = 0; i < files1.size(); i++) {
		LogD() << "Anterior lens images: " << wtoa(files1[i]);
		auto fname = files1[i].substr(files1[i].rfind('\\') + 1);

		CorTopo::AnteriorLens anterior;
		if (anterior.loadCorneaImage(files1[i])) {
			anterior.makeupAnteriorCorneaBorder();
			_anteriorLens.push_back(move(anterior));
		}

		CorTopo::AxialMeasure measure;
		if (measure.loadRetinaImage(files2[i], 0) && 
			measure.loadCorneaImage(files1[i], 1000)) {
			measure.performCalculation();
			measure.getAxialLength();
		}
	}
	return;
}


void CHctReportDoc::openLensThicknessData(LPCTSTR path)
{
	auto pMain = (CMainFrame*)AfxGetMainWnd();

	auto path1 = wstring(path) + L"\\cornea";
	auto path2 = wstring(path) + L"\\front";
	auto path3 = wstring(path) + L"\\back";
	auto files1 = SystemFunc::findFilesInDirectory(path1, _T("*.jpg"));
	auto files2 = SystemFunc::findFilesInDirectory(path2, _T("*.jpg"));
	auto files3 = SystemFunc::findFilesInDirectory(path3, _T("*.jpg"));

	for (int i = 0; i < files1.size(); i++) {
		LogD() << "Lens thickness images: " << wtoa(files1[i]);
		auto fname = files1[i].substr(files1[i].rfind('\\') + 1);

		CorTopo::AnteriorLens anterior;
		if (anterior.loadCorneaImage(files3[i], true)) {
			anterior.makeupPosteriorLensBorder();
			_anteriorLens.push_back(move(anterior));
		}

		CorTopo::LensMeasure measure;
		if (measure.loadLensBackImage(files3[i], 3000) &&
			measure.loadLensFrontImage(files2[i], 2000) &&
			measure.loadCorneaImage(files1[i], 1000)) {
			measure.performCalculation();
			measure.getLensThickness();
		}
	}
	return;
}


void CHctReportDoc::openLineScanData(LPCTSTR path)
{
	OctSystem::Analysis::clearAllScanData();
	OctSystem::Analysis::clearAllReports();

	auto data = OctSystem::Analysis::obtainLineScanData();
	// data->getDescript().setup(PatternName::MacularLine, 512, 1, 6.0f, 6.0f);
	data->getDescript().setup(PatternName::DiscLine, 512, 1, 6.0f, 6.0f);

	auto files = SystemFunc::findFilesInDirectory(path, _T("*.bmp"));
	data->importPatternImages(files);	
	data->importPreviewImages(path);

	bool result = data->getPreview()->getDescript().isPreviewCross();

	Analysis::doSegmentation(data);

	if (data->getDescript().isMacularScan()) {
		MacularReport2* report = OctSystem::Analysis::obtainMacularReport();
		report->setProtocolData(data, EyeSide::OD);
	}
	else {
		DiscReport2* report = OctSystem::Analysis::obtainDiscReport();
		report->setProtocolData(data, EyeSide::OD);
	}
	return;
}


void CHctReportDoc::openCubeScanData(LPCTSTR path)
{
	OctSystem::Analysis::clearAllScanData();
	OctSystem::Analysis::clearAllReports();

	auto p = (CMainFrame*)AfxGetMainWnd();
	auto name = p->patternName;
	auto lineSize = p->lineSize;
	auto numLines = p->numLines;
	auto rangeX = p->rangeX;
	auto rangeY = p->rangeY;

	auto data = OctSystem::Analysis::obtainCubeScanData();
	auto direct = 0;
	data->getDescript().setup(name, lineSize, numLines, rangeX, rangeY, direct);

	data->importPatternImages(path);
	data->importPreviewImages(path);

	// Analysis::doSegmentation(data, true, path);
	Analysis::doSegmentation(data);

	//data->importBsegmResults(path);
	data->exportBsegmResults();
	/*
	auto data = OctSystem::Analysis::obtainCubeScanData();
	auto data2 = OctSystem::Analysis::obtainCubeScanData(1);
	data->getDescript().setup(name, lineSize, numLines, rangeX, rangeY, 0, 10);
	data->importPatternImages(path);
	data->importPreviewImages(path);

	CppUtil::ClockTimer::start();
	// Analysis::doSegmentation(data, false);
	// Analysis::doDewarpingOfCorneaScan(data, false);
	
	Analysis::doSegmentation(data);
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Analyze layers segmentation elapsed: " << msec;


	data->updatePatternImages();
	data->updatePreviewImages();

	data->importPatternImages(path);
	data->importPreviewImages(path);
	*/

	/*
	CorTopo::AxialMeasure measure;
	measure.loadCorneaImage(_T("D:\\OCT data\\biometer\\axial length\\JEB\\001.jpg"), 200);
	measure.loadRetinaImage(_T("D:\\OCT data\\biometer\\axial length\\JEB\\000.jpg"), 2000);
	measure.performCalculation();

	if (measure.isResult()) {
		float axialLen = measure.getAxialLength();
	}
	*/

	// Analysis::doSegmentation(data);
	// double msec = CppUtil::ClockTimer::elapsedMsec();
	// LogD() << "Analyze layers segmentation elapsed: " << msec;

	EyeSide side = EyeSide::OD;
	if (data->getDescript().isMacularScan()) {
		MacularReport2* report = OctSystem::Analysis::obtainMacularReport();
		report->setProtocolData(data, side);
	}
	else if (data->getDescript().isDiscScan()) {
		DiscReport2* report = OctSystem::Analysis::obtainDiscReport();
		report->setProtocolData(data, side);
	}
	else if (data->getDescript().isCorneaScan()) {
		CorneaReport2* report = OctSystem::Analysis::obtainCorneaReport();
		report->setProtocolData(data, side);
	}
	return;
}


void CHctReportDoc::openRadialScanData(LPCTSTR path, int lineSize, int numLines)
{
	OctSystem::Analysis::clearAllScanData();
	OctSystem::Analysis::clearAllReports();

	auto p = (CMainFrame*)AfxGetMainWnd();
	auto name = p->patternName;

	auto data = OctSystem::Analysis::obtainRadialScanData();
	data->getDescript().setup(name, lineSize, numLines, 9.0f, 9.0f, 0, 10);
	data->importPatternImages(path);
	data->importPreviewImages(path);

	CppUtil::ClockTimer::start();
	Analysis::doSegmentation(data);
	// Analysis::doSegmentation(data);
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Analyze layers segmentation elapsed: " << msec;

	if (data->getDescript().isMacularScan()) {
		MacularReport2* report = OctSystem::Analysis::obtainMacularReport();
		report->setProtocolData(data, EyeSide::OD);
	}
	else if (data->getDescript().isDiscScan()) {
		DiscReport2* report = OctSystem::Analysis::obtainDiscReport();
		report->setProtocolData(data, EyeSide::OD);
	}
	else if (data->getDescript().isCorneaScan()) {
		CorneaReport2* report = OctSystem::Analysis::obtainCorneaReport();
		report->setProtocolData(data, EyeSide::OD);
	}
	return;
}


BOOL CHctReportDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	/*
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	*/

	/*
	auto p = (CMainFrame*)AfxGetMainWnd();
	auto name = p->patternName;

	// TODO:  Add your specialized creation code here
	if (name == PatternName::Macular3D || name == PatternName::Disc3D || name == PatternName::MacularAngio) {
		openCubeScanData(lpszPathName);
	}
	else {
		openLineScanData(lpszPathName);
	}
	return TRUE;
	*/

	return FALSE;
}
