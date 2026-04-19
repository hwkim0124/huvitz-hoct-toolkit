
// HctReportView.cpp : implementation of the CHctReportView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "HctReport.h"
#endif

#include "HctReportDoc.h"
#include "HctReportView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHctReportView

IMPLEMENT_DYNCREATE(CHctReportView, CScrollView)

BEGIN_MESSAGE_MAP(CHctReportView, CScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CHctReportView construction/destruction

CHctReportView::CHctReportView()
{
	// TODO: add construction code here

}

CHctReportView::~CHctReportView()
{
}

BOOL CHctReportView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CScrollView::PreCreateWindow(cs);
}

// CHctReportView drawing

void CHctReportView::createBitmapOfPattern(void)
{
	auto data = Analysis::getProtocolData(0);

	if (!data) { 
		return;
	}

	// data->exportBsegmResults(L"./");
	for (int i = 0; i < data->countBscanData(); i++) {
		auto bscan = data->getBscanData(i);
		if (bscan) {
			auto image = bscan->getBsegmSource();
			// auto image = bscan->getBsegmSample();
			// auto image = bscan->getBsegmDenoised();
			// auto image = bscan->getBsegmGradients();
			// auto image = bscan->getBsegmAscends();
			// auto image = bscan->getBsegmDescends();

			if (image) {
				auto braw = image->getBitmap();
				if (braw) {
					_bscanObjects.push_back(bscan);
					_imageObjects.push_back(image);
					_imageBitmaps.push_back(braw);
					_imageTitles.push_back(bscan->getImageName());

					int width = image->getWidth();
					int height = image->getHeight();

					// Gdiplus SetPixel() takes a long time when the bitmap is 8bit gray indexed, 
					// which should be converted to 24bit color rgb before. 
					if (data->getDescript().isCorneaScan()) {
						auto epis = bscan->getLayerPoints(OcularLayerType::EPI, width, height);
						for (int j = 0; j < epis.size(); j++) {
							if (epis[j] >= 0) {
								braw->SetPixel(j, epis[j], Gdiplus::Color(0, 255, 192));
								braw->SetPixel(j, epis[j] + 1, Gdiplus::Color(0, 255, 192));
							}
						}

						auto ends = bscan->getLayerPoints(OcularLayerType::END, width, height);
						for (int j = 0; j < ends.size(); j++) {
							if (ends[j] >= 0) {
								braw->SetPixel(j, ends[j], Gdiplus::Color(255, 192, 0));
								braw->SetPixel(j, ends[j] + 1, Gdiplus::Color(255, 192, 0));
							}
						}
			
						auto bows = bscan->getLayerPoints(OcularLayerType::BOW, width, height);
						for (int j = 0; j < bows.size(); j++) {
							if (bows[j] >= 0) {
								braw->SetPixel(j, bows[j], Gdiplus::Color(192, 0, 255));
								braw->SetPixel(j, bows[j] + 1, Gdiplus::Color(192, 0, 255));
							}
						}
						/*
						auto inner = bscan->getLayerPoints(OcularLayerType::INNER, width, height);
						for (int j = 0; j < inner.size(); j++) {
							if (inner[j] >= 0) {
								braw->SetPixel(j, inner[j], Gdiplus::Color(0, 255, 0));
								braw->SetPixel(j, inner[j] + 1, Gdiplus::Color(0, 255, 0));
							}
						}
						auto outer = bscan->getLayerPoints(OcularLayerType::OUTER, width, height);
						for (int j = 0; j < outer.size(); j++) {
							if (outer[j] >= 0) {
								braw->SetPixel(j, outer[j], Gdiplus::Color(255, 0, 0));
								braw->SetPixel(j, outer[j] + 1, Gdiplus::Color(255, 0, 0));
							}
						}
						*/
						continue;
					}
					
					int x1, x2;
					if (bscan->getOpticNerveCupRange(x1, x2)) {
						LogD() << "Cup range: " << x1 << " - " << x2 << ", index: " << i;
					}


					if (bscan->getOpticNerveDiscRange(x1, x2)) {
						LogD() << "Disc range: " << x1 << " - " << x2 << ", index: " << i;
					}
					else {
						x1 = 9999; x2 = -9999;
					}

					auto thicks = bscan->getLayerThickness(OcularLayerType::ILM, OcularLayerType::RPE, width, height);


					auto inner = bscan->getLayerPoints(OcularLayerType::INNER, width, height);
					for (int j = 0; j < inner.size(); j++) {
						if (inner[j] >= 0) {
							braw->SetPixel(j, inner[j], Gdiplus::Color(0, 0, 255));
							braw->SetPixel(j, inner[j]+1, Gdiplus::Color(0, 0, 255));
						}
					}
					auto outer = bscan->getLayerPoints(OcularLayerType::OUTER, width, height);
					for (int j = 0; j < outer.size(); j++) {
						if (outer[j] >= 0) {
							braw->SetPixel(j, outer[j], Gdiplus::Color(224, 224, 224));
							// braw->SetPixel(j, outer[j] + 1, Gdiplus::Color(224, 224, 224));
						}
					}

					auto ilms = bscan->getLayerPoints(OcularLayerType::ILM, width, height);
					for (int j = 0; j < ilms.size(); j++) {
						if (ilms[j] >= 0) {
							braw->SetPixel(j, ilms[j], Gdiplus::Color(0, 255, 255));
							braw->SetPixel(j, ilms[j] + 1, Gdiplus::Color(0, 255, 255));
						}
					}

					auto nfls = bscan->getLayerPoints(OcularLayerType::NFL, width, height);
					for (int j = 0; j < nfls.size(); j++) {
						if (nfls[j] >= 0) {
							braw->SetPixel(j, nfls[j], Gdiplus::Color(255, 0, 128));
							braw->SetPixel(j, nfls[j] + 1, Gdiplus::Color(255, 0, 128));
						}
					}

					auto ipls = bscan->getLayerPoints(OcularLayerType::IPL, width, height);
					for (int j = 0; j < ipls.size(); j++) {
						if (ipls[j] >= 0) {
							braw->SetPixel(j, ipls[j], Gdiplus::Color(0, 255, 128));
							braw->SetPixel(j, ipls[j] + 1, Gdiplus::Color(0, 255, 128));
						}
					}
					auto opls = bscan->getLayerPoints(OcularLayerType::OPL, width, height);
					for (int j = 0; j < opls.size(); j++) {
						if (opls[j] >= 0) {
							braw->SetPixel(j, opls[j], Gdiplus::Color(255, 255, 64));
							braw->SetPixel(j, opls[j] + 1, Gdiplus::Color(255, 255, 64));
						}
					}

					auto ioss = bscan->getLayerPoints(OcularLayerType::IOS, width, height);
					for (int j = 0; j < ioss.size(); j++) {
						if (ioss[j] >= 0) {
							braw->SetPixel(j, ioss[j], Gdiplus::Color(64, 192, 255));
							braw->SetPixel(j, ioss[j] + 1, Gdiplus::Color(64, 192, 255));
						}
					}

					auto rpes = bscan->getLayerPoints(OcularLayerType::RPE, width, height);
					for (int j = 0; j < rpes.size(); j++) {
						if (rpes[j] >= 0) {
							braw->SetPixel(j, rpes[j], Gdiplus::Color(255, 192, 64));
							braw->SetPixel(j, rpes[j] + 1, Gdiplus::Color(255, 192, 64));
						}
					}

					auto brm = bscan->getLayerPoints(OcularLayerType::BRM, width, height);
					for (int j = 0; j < brm.size(); j++) {
						if (brm[j] >= 0) {
							braw->SetPixel(j, brm[j], Gdiplus::Color(192, 64, 255));
							braw->SetPixel(j, brm[j] + 1, Gdiplus::Color(192, 64, 255));
						}
					}
					/*
					auto oprs = bscan->getLayerPoints(OcularLayerType::OPR, width, height);
					for (int j = 0; j < oprs.size(); j++) {
						if (oprs[j] >= 0) {
							braw->SetPixel(j, oprs[j], Gdiplus::Color(255, 128, 0));
							braw->SetPixel(j, oprs[j] + 1, Gdiplus::Color(255, 128, 0));
						}
					}
					*/
				}
			}
		}
	}
	return;
}


void CHctReportView::createBitmapOfWideAnterior(void)
{
	CHctReportDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	for (int i = 0; i < pDoc->_wides.size(); i++) {
		auto& wide = pDoc->_wides[i];
		auto image = wide.imageCornea();
		auto image2 = wide.imageChamber();
		auto image3 = wide.imageAnterior();
		if (image) {
			auto draw = image->getBitmap();
			auto draw2 = image2->getBitmap();
			auto draw3 = image3->getBitmap();

			if (draw) {
				_imageObjects.push_back(image);
				_imageObjects.push_back(image2);
				_imageObjects.push_back(image3);
				_imageBitmaps.push_back(draw);
				_imageBitmaps.push_back(draw2);
				_imageBitmaps.push_back(draw3);
				_imageTitles.push_back(wide.corneaImageName());
				_imageTitles.push_back(wide.chamberImageName());
				_imageTitles.push_back(wide.chamberImageName());

				auto inner = wide.corneaAnteriorLine();
				for (int j = 0; j < inner.size(); j++) {
					if (inner[j] >= 0) {
						draw->SetPixel(j, inner[j], Gdiplus::Color(0, 255, 0));
						draw->SetPixel(j, inner[j] + 1, Gdiplus::Color(0, 255, 0));
					}
				}

				auto outer = wide.corneaPosteriorLine();
				for (int j = 0; j < outer.size(); j++) {
					if (outer[j] >= 0) {
						draw->SetPixel(j, outer[j], Gdiplus::Color(255, 0, 0));
						draw->SetPixel(j, outer[j] + 1, Gdiplus::Color(255, 0, 0));
					}
				}

				auto inner2 = wide.chamberAnteriorLine();
				for (int j = 0; j < inner2.size(); j++) {
					if (inner2[j] >= 0) {
						draw2->SetPixel(j, inner2[j], Gdiplus::Color(0, 255, 0));
						draw2->SetPixel(j, inner2[j] + 1, Gdiplus::Color(0, 255, 0));
					}
				}

				for (int i = 0; i < 2; i++) {
					auto line = wide.chamberHingeLine(i);
					for (int j = 0; j < line.size(); j++) {
						if (line[j] >= 0) {
							draw2->SetPixel(j, line[j], Gdiplus::Color(0, 0, 255));
							draw2->SetPixel(j, line[j] + 1, Gdiplus::Color(0, 0, 255));
						}
					}
				}
			}
		}
	}
	return;
}


void CHctReportView::createBitmapOfAnteriorLens(void)
{
	CHctReportDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	for (int i = 0; i < pDoc->_anteriorLens.size(); i++) {
		auto& lens = pDoc->_anteriorLens[i];
		auto image = lens.imageCornea();
		if (image) {
			auto draw = image->getBitmap();
			if (draw) {
				_imageObjects.push_back(image);
				_imageBitmaps.push_back(draw);
				_imageTitles.push_back(lens.corneaImageName());

				auto inner = lens.corneaAnteriorLine();
				for (int j = 0; j < inner.size(); j++) {
					if (inner[j] >= 0) {
						draw->SetPixel(j, inner[j], Gdiplus::Color(0, 255, 0));
						draw->SetPixel(j, inner[j] + 1, Gdiplus::Color(0, 255, 0));
					}
				}
			}
		}
	}
	return;
}


void CHctReportView::createBitmapOfLensThickness(void)
{
	CHctReportDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	for (int i = 0; i < pDoc->_anteriorLens.size(); i++) {
		auto& lens = pDoc->_anteriorLens[i];
		auto image = lens.imageCornea();
		if (image) {
			auto draw = image->getBitmap();
			if (draw) {
				_imageObjects.push_back(image);
				_imageBitmaps.push_back(draw);
				_imageTitles.push_back(lens.corneaImageName());

				auto inner = lens.lensPosteriorLine();
				for (int j = 0; j < inner.size(); j++) {
					if (inner[j] >= 0) {
						draw->SetPixel(j, inner[j], Gdiplus::Color(0, 255, 0));
						draw->SetPixel(j, inner[j] + 1, Gdiplus::Color(0, 255, 0));
					}
				}
			}
		}
	}
	return;
}


void CHctReportView::updateLayoutOfImages(void)
{
	_imageRowCols.clear();
	_imageStartXYs.clear();
	_imageEndXYs.clear();

	const int SMALL_IMAGE_WIDTH = 300;
	const int SMALL_IMAGE_COLS_MAX = 5;
	const int NORMAL_IMAGE_WIDTH = 600;
	const int NORMAL_IMAGE_COLS_MAX = 4;
	const int HORZ_OFFSET = 50;
	const int VERT_OFFSET = 100;
	const int TOP_MARGIN = 100;
	const int LEFT_MARGIN = 50;
	const int CHART_WIDTH = 400;

	int row = 0, col = 0;
	int isx = LEFT_MARGIN, isy = TOP_MARGIN;
	int iex = 0, iey = 0, gex = 0, gey = 0;
	int wScreen = 0, hScreen = 0;
	int endX = 0, endY = 0;

	for (int i = 0; i < _imageObjects.size(); i++) {
		auto p = _imageObjects[i];
		if (p->getWidth() < SMALL_IMAGE_WIDTH) {
			if (++col > SMALL_IMAGE_COLS_MAX) {
				col = 1; ++row;
				isx = LEFT_MARGIN;
				isy = hScreen;
			}
		}
		else if (p->getWidth() < NORMAL_IMAGE_WIDTH) {
			if (++col > NORMAL_IMAGE_COLS_MAX) {
				col = 1; ++row;
				isx = LEFT_MARGIN;
				isy = hScreen;
			}
		}
		else {
			col = 1; ++row;
			isx = LEFT_MARGIN;
			isy = hScreen;
		}

		iex = (isx + p->getWidth());
		iey = (isy + p->getHeight());
		endX = iex + HORZ_OFFSET;
		endY = iey + VERT_OFFSET;

		_imageStartXYs.push_back(pair<int, int>(isx, isy));
		_imageEndXYs.push_back(pair<int, int>(iex, iey));
		_imageRowCols.push_back(pair<int, int>(row, col));

		wScreen = (wScreen < endX ? endX : wScreen);
		hScreen = (hScreen < endY ? endY : hScreen);
		isx = endX;
	}

	_screenWidth = wScreen + CHART_WIDTH;
	_screenHeight = (int)(hScreen * 1.2);
	_screenHeight += 1024;
	return;
}


void CHctReportView::updateScreenBitmap(void)
{
	CPaintDC dc(this);

	Bitmap screen(_screenWidth, _screenHeight);
	Graphics G(dc);
	Graphics memG(&screen);

	memG.FillRectangle(&SolidBrush(Color(255, 255, 255)), 0, 0, _screenWidth, _screenHeight);
	drawImageObjects(memG);

	if (_indexOfImageSelected >= 0) {
		drawIntensityGraph(memG, _indexOfImageSelected);
	}

	drawPatternReport(memG);

	// Why cached bitmap should be redrawn overall and recreated for every update of screen display?
	_pScreenBitmap = make_unique<CachedBitmap>(&screen, &G);
	return;
}


void CHctReportView::drawImageObjects(Gdiplus::Graphics & G)
{
	Gdiplus::Font font(L"Arial", 12, FontStyleRegular, UnitPixel);
	Gdiplus::SolidBrush brush(Color(0, 0, 0));

	for (int i = 0; i < _imageObjects.size(); i++) {
		// auto p = (const SegmScan::ScanImage*)m_imageObjects[i
		// auto bscan = _bscanObjects[i];
		auto p = _imageObjects[i];
		auto xy = _imageStartXYs[i];

		Bitmap* pBitmap = (Bitmap*)_imageBitmaps[i];// .get();
		G.DrawImage(pBitmap, xy.first, xy.second);
		
		wstring text;
		wostringstream oss;
		oss << _imageTitles[i];
		oss << L", " << p->getWidth() << L" x " << p->getHeight();

		text = oss.str();
		G.DrawString(text.data(), text.length(), &font, PointF(xy.first, xy.second + p->getHeight() + 5), &brush);
		
		oss.str(L"");
		oss.precision(4);
		oss << "stddev: " << p->getStddev() << " mean: " << p->getMean() << " ratio: " << (float)(p->getStddev() / p->getMean());
		text = oss.str();
		G.DrawString(text.data(), text.length(), &font, PointF(xy.first, xy.second + p->getHeight() + 20), &brush);

		/*
		p = bscan->getBsegmGradients();

		oss.str(L"");
		oss.precision(4);
		oss << "stddev: " << p->getStddev() << " mean: " << p->getMean() << " ratio: " << (float)(p->getStddev() / p->getMean());
		text = oss.str();
		G.DrawString(text.data(), text.length(), &font, PointF(xy.first, xy.second + p->getHeight() + 35), &brush);
		*/

	}
	return;
}


void CHctReportView::drawIntensityGraph(Gdiplus::Graphics & G, int index)
{
	if (index < 0 || index >= _imageObjects.size()) {
		return;
	}

	auto p = _imageObjects[index];
	// auto p = _bscanObjects[index]->getBsegmGradients();
	if (p == nullptr) {
		return;
	}

	int gsx = 0, gsy = 0;
	getGraphStartXY(index, gsx, gsy);

	const int GRAPH_X_SIZE = 255;
	const int GRAPH_Y_SIZE = p->getHeight();
	int gex = gsx + GRAPH_X_SIZE;
	int gey = gsy + GRAPH_Y_SIZE;

	Pen pen(Color(0, 0, 0));
	G.DrawLine(&pen, gsx, gsy, gex, gsy);
	G.DrawLine(&pen, gsx, gsy, gsx, gey);

	pen.SetDashStyle(DashStyleDot);
	G.DrawLine(&pen, gsx + 50, gsy, gsx + 50, gey);
	G.DrawLine(&pen, gsx + 100, gsy, gsx + 100, gey);
	G.DrawLine(&pen, gsx + 150, gsy, gsx + 150, gey);
	G.DrawLine(&pen, gsx + 200, gsy, gsx + 200, gey);

	if (_columnAtPointer >= 0 && _columnAtPointer < p->getWidth() &&
		_rowAtPointer >= 0 && _rowAtPointer < p->getHeight()) {
		const unsigned char* pBits = p->getBitsData(); // p->getBitsData();
		unsigned char gray;
		int xpre, ypre, xcur, ycur;

		// Gray change along the selected column. 
		Pen pen2(Color(0x00, 0xFF, 0xFF));
		for (int row = 0; row < p->getHeight(); row++) {
			gray = pBits[row*p->getWidth() + _columnAtPointer];
			if (row == 0) {
				xpre = gsx + gray; ypre = gsy;
			}
			else {
				xcur = gsx + gray; ycur = gsy + row;
				G.DrawLine(&pen2, xpre, ypre, xcur, ycur);
				xpre = xcur; ypre = ycur;
			}
		}

		// Gray value at position selected. 
		gray = pBits[_rowAtPointer*p->getWidth() + _columnAtPointer];
		int xpos = gsx + gray;
		int ypos = gsy + _rowAtPointer;

		Pen pen3(Color(0xFF, 0x00, 0x00));
		pen3.SetWidth(2);
		G.DrawEllipse(&pen3, xpos, ypos, 5, 5);

		wstring text;
		wostringstream oss(text);
		oss << gray << " at (" << _rowAtPointer << ", " << _columnAtPointer << ")";

		Gdiplus::Font font(L"Arial", 12, FontStyleRegular, UnitPixel);
		Gdiplus::SolidBrush brush(Color(0, 0, 0));

		text = oss.str();
		G.DrawString(text.data(), text.length(), &font, PointF(xpos - 30, ypos + 10), &brush);
	}
	return;
}


void CHctReportView::drawPatternReport(Gdiplus::Graphics & G)
{
	auto data = Analysis::getProtocolData();
	if (data == nullptr) {
		return;
	}

	// return;
	if (data->getDescript().isMacularScan())
	{
		auto report = Analysis::obtainMacularReport();
		auto enface = report->getMacularEshot(OcularLayerType::ILM, OcularLayerType::BRM);

		if (enface != nullptr) {
			auto image = enface->makeImage(300, 300, false);
			G.DrawImage(image.getBitmap(), 100, _screenHeight - 1024);
		}

		auto tmap = report->getMacularEplot(OcularLayerType::ILM, OcularLayerType::IPL);

		if (tmap != nullptr) {
			int img_w, img_h, img_sx, img_sy;
			float center_x, center_y;
			float range_hw, range_hh;
			int cx_pos, cy_pos;

			img_w = 300;
			img_h = 300;
			img_sx = 500;
			img_sy = _screenHeight - 1024;

			auto image = tmap->makeImage(img_w, img_h);
			G.DrawImage(image.getBitmap(), img_sx, img_sy);

			center_x = report->getFoveaCenterX();
			center_y = report->getFoveaCenterY();
			range_hw = report->getDescript()->getScanRangeX() * 0.5f;
			range_hh = report->getDescript()->getScanRangeY() * 0.5f;
			cx_pos = (int)((1.0f + (center_x / range_hw)) * img_w * 0.5f);
			cy_pos = (int)((1.0f + (center_y / range_hh)) * img_h * 0.5f);

			Pen pen(Color(0xFF, 0x00, 0x00));
			pen.SetWidth(2);
			G.DrawEllipse(&pen, img_sx + cx_pos, img_sy + cy_pos, 5, 5);
		}

		auto chart = report->makeETDRSChart(OcularLayerType::ILM, OcularLayerType::NFL);
		LogD() << "Center thick.: " << chart.centerThickness();
		LogD() << "Inner thick.: " << chart.innerThickness(0) << "," << chart.innerThickness(1) << "," << chart.innerThickness(2) << "," << chart.innerThickness(3);
		LogD() << "Outer thick.: " << chart.outerThickness(0) << "," << chart.outerThickness(1) << "," << chart.outerThickness(2) << "," << chart.outerThickness(3);
		LogD() << "Average thick.: " << chart.averageThickness();
		LogD() << "Superior thick.: " << chart.superiorThickness();
		LogD() << "Inferior thick.: " << chart.inferiorThickness();

		/*
		for (int i = 0; i < 6; i++) {
		LogD() << "Hexgonal thick-" << i << ": " << chart.hexagonalThickness(i) << ", " << chart.hexagonalThicknessPercentile(i);
		}
		*/

		float centerX = report->getFoveaCenterX();
		float centerY = report->getFoveaCenterY();
		float centerT = report->getFoveaCenterThickness();

		LogD() << "Fovea center: " << centerX << ", " << centerY << ", thick.: " << centerT;

		auto chart2 = report->makeETDRSChart(OcularLayerType::ILM, OcularLayerType::RPE, centerX, centerY);

		LogD() << "Center thick.: " << chart2.centerThickness();
		LogD() << "Inner thick.: " << chart2.innerThickness(0) << "," << chart2.innerThickness(1) << "," << chart2.innerThickness(2) << "," << chart2.innerThickness(3);
		LogD() << "Outer thick.: " << chart2.outerThickness(0) << "," << chart2.outerThickness(1) << "," << chart2.outerThickness(2) << "," << chart2.outerThickness(3);
		LogD() << "Average thick.: " << chart2.averageThickness();
		LogD() << "Superior thick.: " << chart2.superiorThickness();
		LogD() << "Inferior thick.: " << chart2.inferiorThickness();

		auto chart3 = report->makeGCCThicknessChart();
		for (int i = 0; i < 6; i++) {
			LogD() << "Sectors thick-" << i << ": " << chart3.sectionThickness(i);
		}
		LogD() << "Average thick.: " << chart3.averageThickness();
		LogD() << "Superior thick.: " << chart3.superiorThickness();
		LogD() << "Inferior thick.: " << chart3.inferiorThickness();
	}
	else if (data->getDescript().isDiscScan())
	{
		auto report = Analysis::obtainDiscReport();
		auto enface = report->getDiscEshot(OcularLayerType::ILM, OcularLayerType::RPE);

		if (enface != nullptr) {
			auto image = enface->makeImage(300, 300, true);
			image.equalizeHistogram(2.0f);
			G.DrawImage(image.getBitmap(), 100, _screenHeight - 1024);
		}

		auto tmap = report->getDiscEplot(OcularLayerType::ILM, OcularLayerType::RPE);

		if (tmap != nullptr)
		{
			int img_w, img_h, img_sx, img_sy;
			float center_x, center_y;
			float range_hw, range_hh;
			int cx_pos, cy_pos;

			img_w = 300;
			img_h = 300;
			img_sx = 500;
			img_sy = _screenHeight - 1024;

			auto image = tmap->makeImage(img_w, img_h);
			G.DrawImage(image.getBitmap(), img_sx, img_sy);

			center_x = report->getNerveHeadCenterX();
			center_y = report->getNerveHeadCenterY();
			range_hw = report->getDescript()->getScanRangeX() * 0.5f;
			range_hh = report->getDescript()->getScanRangeY() * 0.5f;
			cx_pos = (int)((1.0f + (center_x / range_hw)) * img_w * 0.5f);
			cy_pos = (int)((1.0f + (center_y / range_hh)) * img_h * 0.5f);

			Pen pen(Color(0xFF, 0x00, 0x00));
			pen.SetWidth(2);
			G.DrawEllipse(&pen, img_sx + cx_pos, img_sy + cy_pos, 5, 5);
		}

		LogD() << "Optic cup, area: " << report->getCupArea() << ", volume: " << report->getCupVolume();
		LogD() << "Optic disc, area: " << report->getDiscArea() << ", volume: " << report->getDiscVolume();
		LogD() << "Optic rim, area: " << report->getRimArea();
		LogD() << "C/D horz. ratio: " << report->getCupDiscHorzRatio();
		LogD() << "C/D vert. ratio: " << report->getCupDiscVertRatio();
		LogD() << "C/D area ratio: " << report->getCupDiscAreaRatio();

		/*
		auto chart = report->makeDiscChart(OcularLayerType::ILM, OcularLayerType::NFL);
		for (int i = 0; i < 12; i++) {
		LogD() << "Clock thick-" << i << ": " << chart.clockThickness(i);
		}

		for (int i = 0; i < 4; i++) {
		LogD() << "Quadrant thick-" << i << ": " << chart.quadrantThickness(i);
		}


		std::vector<float> list = chart.getTSNITGraph(360, 15);
		std::vector<float> norm = chart.getNormalLine(256);
		std::vector<float> bord = chart.getBorderLine(256);
		std::vector<float> outs = chart.getOutsideLine(256);

		for (int i = 0; i < list.size(); i++) {
		LogD() << i << ", " << list[i];
		}
		*/

		float centerX = report->getNerveHeadCenterX();
		float centerY = report->getNerveHeadCenterY();
		LogD() << "Nerve Head center: " << report->getNerveHeadCenterX() << ", " << report->getNerveHeadCenterY();

		auto chart2 = report->makeDiscChart(OcularLayerType::ILM, OcularLayerType::NFL, centerX, centerY, ONH_DISC_RNFL_CIRCLE_SIZE);
		for (int i = 0; i < 12; i++) {
			LogD() << "Clock thick-" << i << ": " << chart2.clockThickness(i);
		}

		for (int i = 0; i < 4; i++) {
			LogD() << "Quadrant thick-" << i << ": " << chart2.quadrantThickness(i);
		}
	}
	else if (data->getDescript().isCorneaScan())
	{
		auto report = Analysis::obtainCorneaReport();

		auto radius = report->getCurvatureRadiusMap();

		if (radius != nullptr) {
			auto image = radius->makeImage(300, 300);
			G.DrawImage(image.getBitmap(), 100, _screenHeight - 1024);
			LogD() << "Curvature radius (H): " << radius->getCurvatureRadiusOnCenterHorz();
			LogD() << "Curvature radius (V): " << radius->getCurvatureRadiusOnCenterVert();
		}

		auto tmap = report->getCorneaEplot(OcularLayerType::EPI, OcularLayerType::BOW);

		if (tmap != nullptr) {
			auto image = tmap->makeImage(300, 300);
			G.DrawImage(image.getBitmap(), 500, _screenHeight - 1024);
			LogD() << "Average Center: " << tmap->getAverageThicknessOnCenter();
		}

		auto chart = report->makeCorneaChart(OcularLayerType::EPI, OcularLayerType::BOW);
		LogD() << "Center thick.: " << chart.centerThickness();
		LogD() << "Inner thick.: " << chart.innerThickness(0) << "," << chart.innerThickness(1) << "," << chart.innerThickness(2) << "," << chart.innerThickness(3);
		LogD() << "Inner thick.: " << chart.innerThickness(4) << "," << chart.innerThickness(5) << "," << chart.innerThickness(6) << "," << chart.innerThickness(7);

		LogD() << "Outer thick.: " << chart.outerThickness(0) << "," << chart.outerThickness(1) << "," << chart.outerThickness(2) << "," << chart.outerThickness(3);
		LogD() << "Outer thick.: " << chart.outerThickness(4) << "," << chart.outerThickness(5) << "," << chart.outerThickness(6) << "," << chart.outerThickness(7);
		LogD() << "Average thick.: " << chart.averageThickness();
		LogD() << "Superior thick.: " << chart.superiorThickness();
		LogD() << "Inferior thick.: " << chart.inferiorThickness();
	}
	return;
}


bool CHctReportView::getGraphStartXY(int index, int & x, int & y)
{
	if (index < 0 || index >= _imageRowCols.size()) {
		return false;
	}

	auto grid = _imageRowCols[index];
	int last = index;

	for (int i = index + 1; i < _imageRowCols.size(); i++) {
		auto next = _imageRowCols[i];
		if (grid.first != next.first) {
			break;
		}
		last = i;
	}

	const int HORZ_OFFSET = 50;
	x = _imageEndXYs[last].first + HORZ_OFFSET;
	y = _imageStartXYs[last].second;
	return true;
}


bool CHctReportView::getIndexOfImageAtPosition(int x, int y)
{
	for (int i = 0; i < _imageObjects.size(); i++) {
		// auto p = (const SegmScan::ScanImage*)m_imageObjects[i];
		auto p = (const OcularImage*)_imageObjects[i];
		auto xy = _imageStartXYs[i];

		if (x >= xy.first && x <= (xy.first + p->getWidth()) &&
			y >= xy.second && y <= (xy.second + p->getHeight())) {
			_indexOfImageSelected = i;
			_columnAtPointer = x - xy.first;
			_rowAtPointer = y - xy.second;
			return true;
		}
	}
	return false;
}


void CHctReportView::OnDraw(CDC *pDC)
{
	CHctReportDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
	if (_pScreenBitmap) {
		Graphics G(pDC->GetSafeHdc());
		G.DrawCachedBitmap(_pScreenBitmap.get(), 0, 0);
	}
}
 

void CHctReportView::OnInitialUpdate()
{	
	CScrollView::OnInitialUpdate();

	CHctReportDoc* pDoc = GetDocument();
	if (!pDoc)
		return;

	if (pDoc->isScanSegment()) {
		createBitmapOfPattern();
	}
	else if (pDoc->isWideAnterior()) {
		createBitmapOfWideAnterior();
	}
	else if (pDoc->isAnteriorLens()) {
		createBitmapOfAnteriorLens();
	}
	else if (pDoc->isLensThickness()) {
		createBitmapOfLensThickness();
	}

	updateLayoutOfImages();
	updateScreenBitmap();

	CSize sizeTotal;
	// TODO: calculate the total size of this view
	sizeTotal.cx = _screenWidth;
	sizeTotal.cy = _screenHeight;
	CSize page(256, 512);
	CSize line(64, 128);
	SetScrollSizes(MM_TEXT, sizeTotal, page, line);
}

void CHctReportView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_PRIOR: // Page Up
		SendMessage(WM_VSCROLL, SB_PAGEUP, 0L);
		break;

	case VK_NEXT:  // Page Down
		SendMessage(WM_VSCROLL, SB_PAGEDOWN, 0L);
		break;

	case VK_UP:    // Arrow Up
		SendMessage(WM_VSCROLL, SB_LINEUP, 0L);
		break;

	case VK_DOWN:  // Arrow Down
		SendMessage(WM_VSCROLL, SB_LINEDOWN, 0L);
		break;

	case VK_HOME:  // Home Key
		SendMessage(WM_VSCROLL, SB_TOP, 0L);
		break;

	case VK_END:   // End Key
		SendMessage(WM_VSCROLL, SB_BOTTOM, 0L);
		break;

	default:
		CScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

// CHctReportView printing

BOOL CHctReportView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CHctReportView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CHctReportView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CHctReportView diagnostics

#ifdef _DEBUG
void CHctReportView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CHctReportView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CHctReportDoc* CHctReportView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHctReportDoc)));
	return (CHctReportDoc*)m_pDocument;
}
#endif //_DEBUG


// CHctReportView message handlers


void CHctReportView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	CPoint scrolled = GetDeviceScrollPosition();
	if (getIndexOfImageAtPosition(scrolled.x + point.x, scrolled.y + point.y)) {
		updateScreenBitmap();
		Invalidate(FALSE); // FALSE to not make display flickering. 
	}

	CScrollView::OnLButtonDown(nFlags, point);
}


void CHctReportView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CScrollView::OnMouseMove(nFlags, point);
}
