#pragma once
#include "ReportCtrlDef.h"
#include <array>
#include <tuple>

using namespace std;
using namespace ReportCtrl;

class REPORTCTRL_DLL_API DrawMeasureTool
{
	typedef struct 
	{
		std::vector <Gdiplus::PointF> points;

		void clear() {
			points.clear();
		}

		void addPoints(Gdiplus::PointF point) {
			if (isFull()) clear();
			points.push_back(point);
		}

		bool isFull() {
			return (points.size() == 2);
		}

	} DrawLength;

	typedef struct
	{
		std::vector <Gdiplus::PointF> points;

		void clear() {
			points.clear();
		}

		void addPoints(Gdiplus::PointF point) {
			if (isFull()) clear();
			points.push_back(point);
		}

		bool isFull() {
			return (points.size() == 3);
		}
		size_t size() {
			return points.size();
		}
	} DrawAngle;

	typedef struct
	{
		std::vector <Gdiplus::PointF> points;

		void clear() 
		{
			points.clear();
		}

		void addPoints(Gdiplus::PointF point) 
		{
			if (points.size() > 0)
			{
				int distX = abs(points.back().X - point.X);
				int distY = abs(points.back().Y - point.Y);

				if (distX < 10 && distY < 10)
				{
					return;
				}
			}
			
			points.push_back(point);
		}

		size_t size()
		{
			return points.size();
		}

		Gdiplus::PointF front()
		{
			return points.front();
		}

		Gdiplus::PointF back()
		{
			return points.back();
		}

	} DrawArea;

	typedef struct
	{
		bool hasPoint;
		std::vector <Gdiplus::PointF> pointCup;
		std::vector <Gdiplus::PointF> pointDisc;
		
		void clear(void)
		{
			hasPoint = false;
			pointCup.clear();
			pointDisc.clear();
		}

		void addPointCup(Gdiplus::PointF point)
		{
			pointCup.push_back(point);
		}

		void addPointDisc(Gdiplus::PointF point)
		{
			pointDisc.push_back(point);
		}


	} DrawCupDisc;

	typedef struct _DrawTISA
	{
		enum class PointType {
			ScleralSpur = 0,
			PointOnCornea,
			PointOnIrisForRange,
			PointOnIrisForScleralSpur,
			PointTypeNum
		};

		array<tuple<PointType, bool/*fixed*/, Gdiplus::PointF>, (int)PointType::PointTypeNum> pointItems;
		double rangeUM;

		_DrawTISA(double range)
		{
			pointItems[0] = make_tuple(PointType::ScleralSpur, false, Gdiplus::PointF(-1, -1));
			pointItems[1] = make_tuple(PointType::PointOnCornea, false, Gdiplus::PointF(-1, -1));
			pointItems[2] = make_tuple(PointType::PointOnIrisForRange, false, Gdiplus::PointF(-1, -1));
			pointItems[3] = make_tuple(PointType::PointOnIrisForScleralSpur, false, Gdiplus::PointF(-1, -1));

			rangeUM = range;
		}

		auto getItem(PointType type)->tuple<PointType, bool, Gdiplus::PointF>&
		{
			return *std::find_if(pointItems.begin(), pointItems.end(), [type](auto x) { return get<0>(x) == type; });
		}

		void clear()
		{
			for (auto& item : pointItems) {
				get<1>(item) = false;
				get<2>(item) = Gdiplus::PointF(-1, -1);
			}
		}

		void addPoints(Gdiplus::PointF point)
		{
			for (auto& item : pointItems) {
				if (!get<1>(item)) {
					get<2>(item) = point;
					break;
				}
			}
		}

		size_t size()
		{
			for (auto item : pointItems) {
				auto pt = get<2>(item);
				if (pt.X < 0 && pt.Y < 0) {
					return (size_t)get<0>(item);
				}
			}
			return pointItems.size();
		}
	} DrawTISA;

public:
	DrawMeasureTool();
	virtual ~DrawMeasureTool();
	void setUnit(CString unitLength, CString unitArea);
	void setImageSize(int sx, int sy);
	void setWindowSize(int sx, int sy);
	void setPixelSize(double sx, double sy);	// size of 1 image pixel

	void showDrawing(bool show);
	void setDrawingTool(DrawingTool tool);
	auto getDrawingTool()->DrawingTool;

	void clearMeasurePoints(void);
	void recalcMeasurePoints(double rateX, double rateY);
	void insertMeasurePoint(CPoint point);
	void insertMeasurePoint(int x, int y);
	void removeMeasurePoint(void);
	void removeMeasureAngle(void);
	void clearCupDisc(void);

	void setTextColor(COLORREF textColor);
	void setLastMoustPos(Gdiplus::PointF mousePt);

protected:
	bool hasCupDiscPoints(void);
	bool getCupDiscPoint(CPoint point, bool &isDisc, int &index);
	void moveCupDiscPoint(CPoint point, bool isDisc, int index);
	void createCupDiscPoints(CPoint point);
	void insertCupPoint(CPoint point);
	void insertDiscPoint(CPoint point);

	Gdiplus::PointF getWindowPointFromImagePoint(Gdiplus::PointF point);
	Gdiplus::PointF getImagePointFromWindowPoint(Gdiplus::PointF point);

protected:
	CString m_unitLength;
	CString m_unitArea;
	int m_imageSizeX;
	int m_imageSizeY;
	int m_windowSizeX;
	int m_windowSizeY;
	double m_pixelSizeX;
	double m_pixelSizeY;
	COLORREF m_textColor;

	bool m_showMeasure;
	DrawingTool m_drawingTool;

	std::vector <DrawLength> m_vecLength;
	DrawLength m_drawLength;

	std::vector <DrawAngle> m_vecAngle;
	DrawAngle m_drawAngle;

	std::vector <DrawArea> m_vecArea;
	DrawArea m_drawArea;

	DrawCupDisc m_drawCupDisc;
	std::vector <pair<DrawingTool, DrawTISA>> m_drawTISAItemList;

	//
	Gdiplus::PointF m_lastMousePt;

protected:
	void drawMeasure(CDC *pDC, CRect rect, CRect rcRoi = CRect(0, 0, 0, 0));
	bool drawLines(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec, CRect rcRoi);
	bool drawPolygon(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec, CRect rcRoi, bool fill = false);
	bool drawCloseCurve(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec, Gdiplus::Color color, CRect rcRoi);
	void drawLength(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec);
	void drawAngle(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec);
	void drawArea(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec);
	void drawCupDiscRatio(Gdiplus::Graphics *G, CRect rect, DrawCupDisc cupDisc, CRect rcRoi);
	bool drawPoints(Gdiplus::Graphics *G, CRect rect, vector <Gdiplus::PointF> vec, CRect rcRoi);
	bool drawPoint(Gdiplus::Graphics *G, CRect rect, Gdiplus::PointF point, CRect rcRoi);
	bool drawCircle(Gdiplus::Graphics *G, CRect rect, Gdiplus::PointF center, int radius, CRect rcRoi);
	void drawMeasureTISA(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);
	void drawMeasureTISA_1(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);
	void drawMeasureTISA_2(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);
	void drawMeasureTISA_3(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);
	void drawMeasureTISA_4(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);
	void drawMeasureTISA_result(DrawTISA& target, Gdiplus::Graphics *G, CRect rect, CRect rcRoi);

	double getPixelsBetweenPoint(Gdiplus::PointF point1, Gdiplus::PointF point2);
	double getLengthOfLine(Gdiplus::PointF point1, Gdiplus::PointF point2);
	double getAngleOfPoints(Gdiplus::PointF point1, Gdiplus::PointF point2, Gdiplus::PointF point3);
	double getAreaOfPolygon(vector <Gdiplus::PointF> points);
	Gdiplus::RectF getLabelRectForLength(Gdiplus::Graphics *G, pair <Gdiplus::PointF,
		Gdiplus::PointF> points, CString text, Gdiplus::Font* pFont, float offsetRatio);
	Gdiplus::RectF getLabelRectForPolygon(Gdiplus::Graphics *G, vector<Gdiplus::PointF> points,
		CString text, Gdiplus::Font* pFont);
	auto getPtOnCircleTISA(CRect rect, Gdiplus::PointF center, double angle, double radius)->pair<Gdiplus::PointF, double>;
	auto getPerpendicularLineTISA(CRect rect, Gdiplus::PointF ptOnCircle, double angleProj)->tuple<vector<Gdiplus::PointF>, double, double>;
	auto getPtOnIrisTISA(Gdiplus::PointF clickedPos, Gdiplus::PointF ptOnCornea, double slopeOfPerpLine, double interceptOfPerpLine)->Gdiplus::PointF;
	auto getCurRangePxTISA(DrawTISA& target)->double;
	auto getTisaAod(DrawTISA& target)->pair<double, double>;
};