#pragma once

#ifdef __REPORTCTRL_DLL
#define REPORTCTRL_DLL_API		__declspec(dllexport)
#else
#define REPORTCTRL_DLL_API		__declspec(dllimport)
#endif

namespace ReportCtrl
{
	enum class DrawingTool
	{
		disable = 0,
		length,
		angle,
		areaPolygon,
		areaCupDisc,
		annotation,
		tisa_500,
		tisa_750,
		aiAnalysis,
	};

//	enum class ThicknessLayer
//	{
//		None = -1,
//		ILMGCL,
//		ILMNFL,
//		ILMRPE,
//		EPIBOW,
//		EPIEND,
//	};

	enum class ThickChartType
	{
		none = -1,
		etdrs,
		gcc,
		rnfl,
		etdrs_rnfl,
	};

	enum class BScanColor
	{
		positive = 0,
		color,
		negative,
		origin,
	};

	enum class FundusColor
	{
		red = 0,
		green,
		blue,
		redFree,		// green + blue
		color
	};

	typedef struct _SegmentLayer
	{
		bool show;
		OctGlobal::OcularLayerType type;
		std::vector <int> data;
		COLORREF rgb;
	} SegmentLayer;
	
	typedef struct _ColorLevel		// y = alpha * (x + beta)
	{
		float alpha = 1.0f;
		float beta = 0.0f;

		void set(int min, int max)
		{
			alpha = (255.0f / (max - min));
			beta = (-1.0f * min);
		}
	} ColorLevel;

	const Gdiplus::Color kColorLine1 = Gdiplus::Color(0xff, 0xfc, 0x00);
	const Gdiplus::Color kColorLine2 = Gdiplus::Color(0x00, 0xff, 0xff);

	const float kSummaryValueInvalid = -9999.0f;

//	const float kDefalutBR = 74.57f;
//	const float kDefaultCT = 4.03f;
//	const float kDefalutUB = 1.00f;
//	const float kDefalutVR = 1.25f;
//
//	static void getYUV(float *y, float *u, float *v, float r, float g, float b)
//	{
//		// Coefficient refered from Wikipedia : YUV BT.709
//		static const float yr = 0.2126f, yg = 0.7152f, yb = 0.0722f;
//		static const float ur = -0.09991f, ug = -0.33609f, ub = 0.436f;
//		static const float vr = 0.615f, vg = -0.55861f, vb = -0.05639f;
//		if (y != nullptr)
//			*y = yr*r + yg*g + yb*b;
//		if (u != nullptr)
//			*u = ur*r + ug*g + ub*b;
//		if (v != nullptr)
//			*v = vr*r + vg*g + vb*b;
//	}
//
//	static void getRGB(float *r, float *g, float *b, float y, float u, float v)
//	{
//		// Coefficient refered from Wikipedia : YUV BT.709
//		static const float ry = 1.0f, ru = 0.0f, rv = 1.28003f;
//		static const float gy = 1.0f, gu = -0.21482f, gv = -0.38059f;
//		static const float by = 1.0f, bu = 2.12798f, bv = 0.0f;
//		if (r != nullptr)
//			*r = ry*y + ru*u + rv*v;
//		if (g != nullptr)
//			*g = gy*y + gu*u + gv*v;
//		if (b != nullptr)
//			*b = by*y + bu*u + bv*v;
//	}
//
//	static std::pair<float, float> getMeanStdDev(unsigned char *buf, int width, int xoff, int yoff, int side)
//	{
//		int pidx; // pixel index
//
//		float sumMean = 0.0;
//		for (int y = yoff; y < (yoff + side); y++) {
//			for (int x = xoff; x < (xoff + side); x++) {
//				pidx = y*width + x;
//				float b = float(buf[3 * pidx + 0]);
//				float g = float(buf[3 * pidx + 1]);
//				float r = float(buf[3 * pidx + 2]);
//				float lumi;
//				getYUV(&lumi, nullptr, nullptr, r, g, b);
//				sumMean += lumi;
//			}
//		}
//
//		float mean = sumMean / (side * side);
//
//		float sumDev = 0.0;
//		for (int y = yoff; y < (yoff + side); y++) {
//			for (int x = xoff; x < (xoff + side); x++) {
//				pidx = y*width + x;
//				float b = float(buf[3 * pidx + 0]);
//				float g = float(buf[3 * pidx + 1]);
//				float r = float(buf[3 * pidx + 2]);
//				float lumi;
//				getYUV(&lumi, nullptr, nullptr, r, g, b);
//				sumDev += (lumi - mean) * (lumi - mean);
//			}
//		}
//
//		float stddev = std::sqrtf(sumDev / (side*side));
//
//		return std::pair<float, float>(mean, stddev);
//	}
//
//	static std::pair<float, float> getMedian(unsigned char *buf, int width)
//	{
//		std::vector<int> xoffs, yoffs;
//		for (int x = 575; x <= 2075; x += 300) {
//			xoffs.push_back(x);
//			yoffs.push_back(x);
//		}
//		int side = 50;
//
//		std::vector<std::pair<float, float>> mdevs;
//
//		for (auto yoff : yoffs) {
//			for (auto xoff : xoffs) {
//				auto mdev = getMeanStdDev(buf, width, xoff, yoff, side);
//				mdevs.push_back(mdev);
//			}
//		}
//
//		std::sort(mdevs.begin(), mdevs.end(), [](std::pair<float, float> a, std::pair<float, float> b) {
//			return b.first < a.first;
//		});
//
//		return mdevs[mdevs.size() / 2];
//	}
//
//	static void correct(unsigned char *buf, int width, float mtgt, float dtgt, float ub, float vr)
//	{
//		auto med = getMedian(buf, width);
//		float msrc = med.first;
//		float dsrc = med.second;
//
//		for (int i = 0; i < (width * width); i++) {
//			float b = float(buf[3 * i + 0]);
//			float g = float(buf[3 * i + 1]);
//			float r = float(buf[3 * i + 2]);
//			float y, u, v;
//			getYUV(&y, &u, &v, r, g, b);
//			if (dsrc != 0.0) {
//				y = (y - msrc) * (dtgt / dsrc) + mtgt;
//			}
//			else {
//				y = (y - msrc) + mtgt;
//			}
//
//			u *= ub;
//			v *= vr;
//			getRGB(&r, &g, &b, y, u, v);
//			if (r > 255.0) r = 255.0;
//			if (r < 0.0) r = 0.0;
//			if (g > 255.0) g = 255.0;
//			if (g < 0.0) g = 0.0;
//			if (b > 255.0) b = 255.0;
//			if (b < 0.0) b = 0.0;
//
//			buf[3 * i + 0] = (unsigned char)b;
//			buf[3 * i + 1] = (unsigned char)g;
//			buf[3 * i + 2] = (unsigned char)r;
//		}
//	}
}

