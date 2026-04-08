#include "stdafx.h"
#include "MacularReport2.h"

#include "CppUtil2.h"
#include "OctResult2.h"
#include "OctData2.h"
#include "SegmScan2.h"

using namespace OctReport;
using namespace OctResult;
using namespace OctData;
using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct MacularReport2::MacularReport2Impl
{
	bool isFoveaCenter;
	int foveaCenterLine;
	int foveaCenterXpos;
	float foveaCenterX;
	float foveaCenterY;
	float foveaCenterThickness;

	MacularReport2Impl() : isFoveaCenter(false)
	{
	}
};


OctReport::MacularReport2::MacularReport2() :
	d_ptr(make_unique<MacularReport2Impl>())
{
}


OctReport::MacularReport2::~MacularReport2() = default;
OctReport::MacularReport2::MacularReport2(MacularReport2 && rhs) = default;
MacularReport2 & OctReport::MacularReport2::operator=(MacularReport2 && rhs) = default;


bool OctReport::MacularReport2::isFoveaCenter(void) const
{
	return d_ptr->isFoveaCenter;
}


float OctReport::MacularReport2::getFoveaCenterX(void) const
{
	return d_ptr->foveaCenterX;
}


float OctReport::MacularReport2::getFoveaCenterY(void) const
{
	return d_ptr->foveaCenterY;
}


float OctReport::MacularReport2::getFoveaCenterThickness(void) const
{
	return d_ptr->foveaCenterThickness;
}


int OctReport::MacularReport2::getFoveaCenterLineIndex(void) const
{
	return d_ptr->foveaCenterLine;
}


int OctReport::MacularReport2::getFoveaCenterLateralPos(void) const
{
	return d_ptr->foveaCenterXpos;
}


bool OctReport::MacularReport2::updateContents(void)
{
	clearContents();

	if (isEmpty()) {
		return false;
	}

	if (getDescript()->isEnfaceScan() ||
		getDescript()->isRadialScan() ||
		getDescript()->isRasterScan()) {
	}
	else {
		return false;
	}

	locateFoveaCenter();
	return true;
}


void OctReport::MacularReport2::clearContents(void)
{
	d_ptr->isFoveaCenter = false;
	d_ptr->foveaCenterX = 0.0f;
	d_ptr->foveaCenterY = 0.0f;
	d_ptr->foveaCenterLine = 0;
	d_ptr->foveaCenterXpos = 0;
	d_ptr->foveaCenterThickness = 0.0f;
	return;
}


SegmScan::MacularEnfaceImage * OctReport::MacularReport2::getEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto shot = prepareEnfaceImage(upper, lower, upperOffset, lowerOffset);
	return static_cast<MacularEnfaceImage*>(shot);
}


SegmScan::MacularThicknessMap * OctReport::MacularReport2::getThicknessMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto tmap = prepareEnfaceMap(upper, lower, upperOffset, lowerOffset);
	return static_cast<MacularThicknessMap*>(tmap);
}


SegmScan::MacularEshot * OctReport::MacularReport2::getMacularEshot(OcularLayerType upper, OcularLayerType lower)
{
	auto shot = getEnfaceImage(upper, lower);
	return static_cast<MacularEshot*>(shot);
}


SegmScan::MacularEplot * OctReport::MacularReport2::getMacularEplot(OcularLayerType upper, OcularLayerType lower)
{
	auto tmap = prepareEnfaceMap(upper, lower);
	return static_cast<MacularEplot*>(tmap);
}


SegmScan::MacularChart OctReport::MacularReport2::makeMacularChart(OcularLayerType upper, OcularLayerType lower, float centerX, float centerY)
{
	MacularChart chart;

	auto tmap = getThicknessMap(upper, lower);
	if (tmap) {
		chart.setEnfaceData(tmap->getThicknessMap());
		chart.setLocation(centerX, centerY);
		chart.setEyeSide(getEyeSide());
		chart.updateMetrics();
	}

	return chart;
}


SegmScan::MacularETDRSChart OctReport::MacularReport2::makeETDRSChart(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, float centerX, float centerY)
{
	MacularETDRSChart chart;

	auto tmap = getThicknessMap(upper, lower, upperOffset, lowerOffset);
	if (tmap) {
		chart.setEnfaceData(tmap->getThicknessMap());
		chart.setLocation(centerX, centerY);
		chart.setEyeSide(getEyeSide());
		chart.updateMetrics();
	}

	return chart;
}


SegmScan::GCCThicknessChart OctReport::MacularReport2::makeGCCThicknessChart(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, float centerX, float centerY)
{
	SegmScan::GCCThicknessChart chart;

	auto tmap = getThicknessMap(upper, lower, upperOffset, lowerOffset);
	if (tmap) {
		chart.setEnfaceData(tmap->getThicknessMap());
		chart.setLocation(centerX, centerY);
		chart.setEyeSide(getEyeSide());
		chart.updateMetrics();
	}

	return chart;
}


void OctReport::MacularReport2::locateFoveaCenter(void)
{
	float rangeX = getDescript()->getScanRangeX();
	float rangeY = getDescript()->getScanRangeY();
	int numLines = getDescript()->getNumberOfScanLines();
	int numPoints = getDescript()->getNumberOfScanPoints();

	int numBscans = getProtocolData()->countBscanData();
	if (numBscans != numLines || numBscans <= 0) {
		return;
	}

	/*
	if (rangeX < FOVEA_OPTIC_DISC_RANGE_X_WIDE || getDescript()->isRadialScan()) {
		detectFoveaCenterInMacularScan(rangeX, rangeY, numLines, numPoints);
	}
	else {
		detectFoveaCenterInWideScan(rangeX, rangeY, numLines, numPoints);
	}
	*/
	determineFoveaCenterInMacularScan(rangeX, rangeY, numLines, numPoints, getEyeSide());

	return;
}

void OctReport::MacularReport2::determineFoveaCenterInMacularScan(float rangeX, float rangeY, int numLines, int numPoints, EyeSide side)
{
	const float HORZ_MARGIN_IN_MM = 1.8f;
	const float VERT_MARGIN_IN_MM = 1.8f;
	const float OUTER_RING_RADIUS = 1.2f;
	const float INNER_RING_RADIUS = 0.4f;
	const float DIFF_RATIO_MAX = 0.99f;
	const float DIFF_RATIO_MIN = 0.0f;

	int max_yidx = -1;
	int max_xidx = -1;
	float max_diff = -1.0f;
	float max_rate = 0.0f;
	float fov_cent = 0.0f;

	if (getDescript()->isHorizontal())
	{
		auto pixs_per_y_mm = (numLines / rangeY);
		auto pixs_per_x_mm = (numPoints / rangeX);

		int line_sidx = (int)((VERT_MARGIN_IN_MM) * pixs_per_y_mm);
		int line_eidx = numLines - line_sidx - 1;
		int point_sidx = (int)((HORZ_MARGIN_IN_MM) * pixs_per_x_mm);
		int point_eidx = numPoints - point_sidx - 1;

		if (rangeX >= FOVEA_OPTIC_DISC_RANGE_X_WIDE) {
			if (isEyeOD()) {
				point_eidx = (int)(numPoints * 0.50f);
			}
 			else {
				point_sidx = (int)(numPoints * 0.50f);
			}
		}

		auto tmap = getThicknessMap(OcularLayerType::ILM, OcularLayerType::RPE);
		auto timg = tmap->makeImage(numPoints, numPoints);
		timg.applyGaussianBlur(1.0);
		timg.resize(numPoints, numLines);

		auto data = timg.copyDataInFloats();
		for (int i = line_sidx; i <= line_eidx; i++) {
			for (int j = point_sidx; j <= point_eidx; j++) {
				auto cent_thick = data[i * numPoints + j];
				auto oy1 = (int)(i - (OUTER_RING_RADIUS * pixs_per_y_mm));
				auto oy2 = (int)(i + (OUTER_RING_RADIUS * pixs_per_y_mm));
				auto ox1 = (int)(j - (OUTER_RING_RADIUS * pixs_per_x_mm));
				auto ox2 = (int)(j + (OUTER_RING_RADIUS * pixs_per_x_mm));

				auto tsum = 0.0f;
				auto tcnt = 0;
				// auto vect = std::vector<float>();
				for (int y = oy1; y <= oy2; y++) {
					for (int x = ox1; x <= ox2; x++) {
						// vect.push_back(data[y * numPoints + x]);
						auto dx = (x - j) / pixs_per_x_mm;
						auto dy = (y - i) / pixs_per_y_mm;
						auto dist = sqrtf(dx * dx + dy * dy);
						if (dist >= INNER_RING_RADIUS && dist <= OUTER_RING_RADIUS) {
							tsum += data[y * numPoints + x];
							tcnt += 1;
						}
					}
				}
				// sort(vect.begin(), vect.end());
				// float outer_thick = vect[(int)(vect.size() * 0.9f)];
				float outer_thick = tsum / tcnt;
				auto diff_thick = outer_thick - cent_thick;
				auto rate_thick = (cent_thick / outer_thick);

				if (diff_thick > max_diff) {
					max_diff = diff_thick;
					max_rate = rate_thick;
					max_yidx = i;
					max_xidx = j;
					fov_cent = cent_thick;
				}
			}
		}
		if (max_yidx >= 0 && max_xidx >= 0) {
			auto ox1 = (int)(max_xidx - (INNER_RING_RADIUS * pixs_per_x_mm));
			auto ox2 = (int)(max_xidx + (INNER_RING_RADIUS * pixs_per_x_mm));
			for (int x = ox1; x <= ox2; x++) {
				auto val = data[max_yidx * numPoints + x];
				if (fov_cent > val) {
					fov_cent = val;
					max_xidx = x;
				}
			}
		}
	}
	else {
		auto pixs_per_y_mm = (numPoints / rangeY);
		auto pixs_per_x_mm = (numLines / rangeX);

		int line_sidx = (int)((HORZ_MARGIN_IN_MM) * pixs_per_x_mm);
		int line_eidx = numLines - line_sidx - 1;
		int point_sidx = (int)((VERT_MARGIN_IN_MM) * pixs_per_y_mm);
		int point_eidx = numPoints - point_sidx - 1;

		if (rangeX >= FOVEA_OPTIC_DISC_RANGE_X_WIDE) {
			if (isEyeOD()) {
				line_eidx = (int)(numLines * 0.50f);
			}
			else {
				line_sidx = (int)(numLines * 0.50f);
			}
		}

		auto tmap = getThicknessMap(OcularLayerType::ILM, OcularLayerType::RPE);
		auto timg = tmap->makeImage(numPoints, numPoints);
		timg.applyGaussianBlur(1.0);
		timg.resize(numLines, numPoints);

		auto data = timg.copyDataInFloats();
		for (int i = line_sidx; i <= line_eidx; i++) {
			for (int j = point_sidx; j <= point_eidx; j++) {
				auto cent_thick = data[j * numLines + i];
				auto oy1 = (int)(j - (OUTER_RING_RADIUS * pixs_per_y_mm));
				auto oy2 = (int)(j + (OUTER_RING_RADIUS * pixs_per_y_mm));
				auto ox1 = (int)(i - (OUTER_RING_RADIUS * pixs_per_x_mm));
				auto ox2 = (int)(i + (OUTER_RING_RADIUS * pixs_per_x_mm));

				auto tsum = 0.0f;
				auto tcnt = 0;
				// auto vect = std::vector<float>();
				for (int y = oy1; y <= oy2; y++) {
					for (int x = ox1; x <= ox2; x++) {
						// vect.push_back(data[y * numPoints + x]);
						auto dx = (x - i) / pixs_per_x_mm;
						auto dy = (y - j) / pixs_per_y_mm;
						auto dist = sqrtf(dx * dx + dy * dy);
						if (dist >= INNER_RING_RADIUS && dist <= OUTER_RING_RADIUS) {
							tsum += data[y * numLines + x];
							tcnt += 1;
						}
					}
				}
				// sort(vect.begin(), vect.end());
				// float outer_thick = vect[(int)(vect.size() * 0.9f)];
				float outer_thick = tsum / tcnt;
				auto diff_thick = outer_thick - cent_thick;
				auto rate_thick = (cent_thick / outer_thick);

				if (diff_thick > max_diff) {
					max_diff = diff_thick;
					max_rate = rate_thick;
					max_yidx = j;
					max_xidx = i;
					fov_cent = cent_thick;
				}
			}
		}
		if (max_yidx >= 0 && max_xidx >= 0) {
			auto oy1 = (int)(max_yidx - (INNER_RING_RADIUS * pixs_per_y_mm));
			auto oy2 = (int)(max_yidx + (INNER_RING_RADIUS * pixs_per_y_mm));
			for (int y = oy1; y <= oy2; y++) {
				auto val = data[y * numLines + max_xidx];
				if (fov_cent > val) {
					fov_cent = val;
					max_yidx = y;
				}
			}
		}
	}

	if (max_rate > DIFF_RATIO_MIN && max_rate < DIFF_RATIO_MAX) {
		float xPosMM, yPosMM;
		if (getDescript()->isHorizontal()) {
			d_ptr->foveaCenterLine = max_yidx;
			d_ptr->foveaCenterXpos = max_xidx;
			xPosMM = (((float)max_xidx / (float)numPoints) - 0.5f) * rangeX;
			yPosMM = (((float)max_yidx / (float)numLines) - 0.5f) * rangeY;
		}
		else {
			d_ptr->foveaCenterLine = max_xidx;
			d_ptr->foveaCenterXpos = max_yidx;
			xPosMM = (((float)max_xidx / (float)numLines) - 0.5f) * rangeX;
			yPosMM = (((float)max_yidx / (float)numPoints) - 0.5f) * rangeY;
		}
		d_ptr->foveaCenterX = xPosMM;
		d_ptr->foveaCenterY = yPosMM;
		d_ptr->foveaCenterThickness = fov_cent;
		d_ptr->isFoveaCenter = true;
		LogD() << "Fovea center located at line " << d_ptr->foveaCenterLine << ", pos " << d_ptr->foveaCenterXpos <<
			" (" << xPosMM << " mm, " << yPosMM << " mm), thick.: " << fov_cent << " um. " << ", diff.: " << max_diff << ", rate: " << max_rate;
	}
	return;
}


void OctReport::MacularReport2::detectFoveaCenterInMacularScan(float rangeX, float rangeY, int numLines, int numPoints)
{
	int disc_line = (int)(numLines * (FOVEA_CENTER_DISC_RADIUS_IN_MM / rangeY));
	int line_sidx = disc_line + 1;
	int line_eidx = numLines - line_sidx - 1;
	int disc_side = (int)(numPoints * (FOVEA_CENTER_DISC_RADIUS_IN_MM / rangeX));
	int spot_sidx = disc_side + 1;
	int spot_eidx = numPoints - spot_sidx - 1;

	float axialRes = (float)GlobalSettings::getRetinaScanAxialResolution();
	float slopeDepth = 0.0f, foveaDepth = 0.0f;
	int centerX = 0, centerY = 0;

	for (int i = line_sidx; i <= line_eidx; i++) {
		auto p = getProtocolData()->getBscanData(i);

		auto inner = p->getLayerPoints(OcularLayerType::ILM);
		auto outer = p->getLayerPoints(OcularLayerType::RPE);
		auto nerve = p->getLayerPoints(OcularLayerType::NFL);

		auto thicks = p->getLayerDistance(OcularLayerType::ILM, OcularLayerType::RPE);

		if (all_of(inner.cbegin(), inner.cend(), [](int e) { return e <= 0; }) ||
			all_of(outer.cbegin(), outer.cend(), [](int e) { return e <= 0; }) ||
			all_of(nerve.cbegin(), nerve.cend(), [](int e) { return e <= 0; }) ||
			inner.size() != numPoints ||
			outer.size() != numPoints || 
			nerve.size() != numPoints) {
			continue;
		}

		for (int j = spot_sidx; j <= spot_eidx; j++) {
			if (inner[j] <= 0 || outer[j] <= 0 || nerve[j] <= 0) {
				continue;
			}

			int cent_h = (outer[j] - inner[j]);
			float cent_d = cent_h * axialRes;
			float nerv_d = (nerve[j] - inner[j]) * axialRes;

			if (nerv_d > FOVEA_CENTER_DISC_NERVE_LIMIT) {
				continue;
			}

			int sidx = max((j - disc_side), 0);
			int eidx = min((j + disc_side), numPoints-1);

			nerv_d = (nerve[sidx] - inner[sidx]) * axialRes;
			if (nerv_d > FOVEA_CENTER_DISC_NERVE_LIMIT) {
				continue;
			}

			nerv_d = (nerve[eidx] - inner[eidx]) * axialRes;
			if (nerv_d > FOVEA_CENTER_DISC_NERVE_LIMIT) {
				continue;
			}

			float disc_bh = abs(outer[eidx] - outer[sidx]) * axialRes;
			if (disc_bh > FOVEA_DISC_BOTTOM_DIFF_MAX) {
				continue;
			}

			float disc_bt = min(outer[sidx] - outer[j], outer[eidx] - outer[j]) * axialRes;
			if (disc_bt > FOVEA_DISC_BOTTOM_OFFSET) {
				continue;
			}

			int disc_h1 = 0, disc_h2 = 0;
			int disc_p1 = 0, disc_p2 = 0;

			for (int k = j; k >= sidx; k--) {
				if (outer[k] >= 0 && inner[k] >= 0 && nerve[k] >= 0) {
					if (thicks[k] < cent_h || (inner[k] > (inner[j] + 2))) {
						disc_h1 = disc_p1 = 0;
						break;
					}
					disc_h1 = max(disc_h1, (outer[k] - inner[k]));
					disc_p1 = k;
				}
			}
			for (int k = j; k <= eidx; k++) {
				if (outer[k] >= 0 && inner[k] >= 0 && nerve[k] >= 0) {
					if (thicks[k] < cent_h || (inner[k] > (inner[j] + 2))) {
						disc_h2 = disc_p2 = 0;
						break;
					}
					disc_h2 = max(disc_h2, (outer[k] - inner[k]));
					disc_p2 = k;
				}
			}

			if (disc_h1 > 0 && disc_h2 > 0) {
				float disc_d1 = disc_h1 * axialRes;
				float disc_d2 = disc_h2 * axialRes;
				float disc_d = min(disc_d1, disc_d2) - cent_d;
				
				float disc_a1 = 0, disc_a2 = 0;
				for (int k = disc_p1; k <= j; k++) {
					disc_a1 += thicks[k];
				}

				for (int k = disc_p2; k >= j; k--) {
					disc_a2 += thicks[k];
				}

				float ratio_d = max(disc_d1, disc_d2) / min(disc_d1, disc_d2);
				float ratio_a = max(disc_a1, disc_a2) / min(disc_a1, disc_a2);

				if (ratio_d < FOVEA_DISC_DEPTH_DIFF_MAX && ratio_a < FOVEA_DISC_AREA_DIFF_MAX) {
					if (slopeDepth < disc_d) {
						slopeDepth = disc_d;
						foveaDepth = cent_d;
						centerX = j;
						centerY = i;
						/*
						LogD() << "h1: " << disc_h1 << ", h2: " << disc_h2 << ", disc_d: " << disc_d;
						LogD() << "p1: " << disc_p1 << ", p2: " << disc_p2 << ", disc_a1: " << disc_a1 << ", disc_a2: " << disc_a2;
						LogD() << "ratio_d: " << ratio_d << ", ratio_a: " << ratio_a;
						LogD() << slopeDepth << ", " << foveaDepth << ", " << centerX << ", " << centerY;
						*/
					}
				}
			}
		}
	}

	if (slopeDepth > 0.0f)
	{
		float ratio = (float)slopeDepth / foveaDepth;
		if (ratio >= FOVEA_CENTER_DISC_DIFF_RATIO_MIN) {
			d_ptr->isFoveaCenter = true;
			d_ptr->foveaCenterLine = centerY;
			d_ptr->foveaCenterXpos = centerX;
			d_ptr->foveaCenterThickness = foveaDepth;

			float xPosMM, yPosMM;
			if (getDescript()->isRadialScan()) {
				float offset = (((float)centerX / (float)numPoints) - 0.5f) * rangeX;
				float degree = (180.0f / numLines) * centerY;

				xPosMM = (float)(offset * cos(NumericFunc::degreeToRadian(degree)));
				yPosMM = (float)(offset * sin(NumericFunc::degreeToRadian(degree)));
			}
			else {
				if (getDescript()->isHorizontal()) {
					xPosMM = (((float)centerX / (float)numPoints) - 0.5f) * rangeX;
					yPosMM = (((float)centerY / (float)numLines) - 0.5f) * rangeY;
				}
				else {
					yPosMM = (((float)centerX / (float)numPoints) - 0.5f) * rangeX;
					xPosMM = (((float)centerY / (float)numLines) - 0.5f) * rangeY;
				}
			}

			d_ptr->foveaCenterX = xPosMM;
			d_ptr->foveaCenterY = yPosMM;
			// LogD() << "ratio: " << ratio;
		}
	}
	return;
}


void OctReport::MacularReport2::detectFoveaCenterInWideScan(float rangeX, float rangeY, int numLines, int numPoints)
{
	float paraMaxArea = 0.0f;
	float centerThick = 0.0f;
	float foveaThick = 0.0f;
	int centerX = 0, centerY = 0;

	auto fov_thicks = vector<float>(numLines, 99999.0f);
	auto fov_idx_xs = vector<int>(numLines, 0);
	auto fov_idx_ys = vector<int>(numLines, 0);
	
	if (getDescript()->isHorizontal())
	{
		int search_y_sidx = (int)(numLines * ((FOVEA_PERI_MARGIN_Y_IN_MM) / rangeY));
		int search_y_eidx = numLines - search_y_sidx - 1;
		int search_x_sidx = (int)(numPoints * ((FOVEA_PARA_OUTER_RADIUS_IN_MM + FOVEA_PERI_MARGIN_X_IN_MM) / rangeX));
		int search_x_eidx = numPoints - search_x_sidx - 1;

		if (isEyeOD()) {
			search_x_eidx = (int)(numPoints * 0.50f);
		}
		else {
			search_x_sidx = (int)(numPoints * 0.50f);
		}

		for (int y = search_y_sidx; y <= search_y_eidx; y++) {
			auto bscan = getProtocolData()->getBscanData(y);
			auto ipl_thicks = bscan->getLayerThickness(OcularLayerType::ILM, OcularLayerType::IPL);
			auto ret_thicks = bscan->getLayerThickness(OcularLayerType::ILM, OcularLayerType::RPE);
			auto line_para_max = 0.0f;
			auto line_cent_x = 0;
			auto line_cent_y = 0;
			auto line_cent_t = 0.0f;

			if (all_of(ipl_thicks.cbegin(), ipl_thicks.cend(), [](int e) { return e <= 0; }) ||
				ipl_thicks.size() != numPoints || ret_thicks.size() != numPoints) {
				continue;
			}

			for (int x = search_x_sidx; x <= search_x_eidx; x++) {
				auto cent_t = ipl_thicks[x];
				auto para_out_x = x - (int)(numPoints * (FOVEA_PARA_OUTER_RADIUS_IN_MM / rangeX));
				auto para_inn_x = x - (int)(numPoints * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeX));

				auto para_area1 = 0.0f;
				auto para_tmax1 = 0.0f;
				auto para_mean1 = 0.0f;
				auto para_count = 0;

				for (int i = para_out_x; i <= para_inn_x; i++) {
					auto thick = (ipl_thicks[i] - cent_t);
					if (thick >= 0.0f) {
						para_area1 += thick;
						para_count += 1;
						para_tmax1 = max(para_tmax1, thick);
					}
				}
				if (para_count > 0) {
					para_mean1 = para_area1 / para_count;
				}

				para_out_x = x + (int)(numPoints * (FOVEA_PARA_OUTER_RADIUS_IN_MM / rangeX));
				para_inn_x = x + (int)(numPoints * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeX));

				auto para_area2 = 0.0f;
				auto para_tmax2 = 0.0f;
				auto para_mean2 = 0.0f;
				para_count = 0;

				for (int i = para_inn_x; i <= para_out_x; i++) {
					auto thick = (ipl_thicks[i] - cent_t);
					if (thick >= 0.0f) {
						para_area2 += thick;
						para_count += 1;
						para_tmax2 = max(para_tmax2, thick);
					}
				}
				if (para_count > 0) {
					para_mean2 = para_area2 / para_count;
				}

				if (para_mean1 > 0.0f && para_mean2 > 0.0f) {
					auto area_mean = (para_mean1 + para_mean2) / 2.0f;
					if (line_para_max < area_mean) {
						line_para_max = area_mean;
						line_cent_t = ret_thicks[x];
						line_cent_x = x;
						line_cent_y = y;
					}
				}
			}

			if (paraMaxArea < line_para_max) {
				paraMaxArea = line_para_max;
				centerThick = line_cent_t;
				centerX = line_cent_x;
				centerY = line_cent_y;
			}

			fov_thicks[y] = line_cent_t;
			fov_idx_xs[y] = line_cent_x;
			fov_idx_ys[y] = line_cent_y;
		}
	}
	else {
		int search_y_sidx = (int)(numPoints * ((FOVEA_PERI_MARGIN_Y_IN_MM) / rangeY));
		int search_y_eidx = numPoints - search_y_sidx - 1;
		int search_x_sidx = (int)(numLines * ((FOVEA_PARA_OUTER_RADIUS_IN_MM + FOVEA_PERI_MARGIN_X_IN_MM) / rangeX));
		int search_x_eidx = numLines - search_x_sidx - 1;

		if (isEyeOD()) {
			search_x_eidx = (int)(numLines * 0.50f);
		}
		else {
			search_x_sidx = (int)(numLines * 0.50f);
		}

		for (int x = search_x_sidx; x <= search_x_eidx; x++) {
			auto bscan = getProtocolData()->getBscanData(x);
			auto ipl_thicks = bscan->getLayerThickness(OcularLayerType::ILM, OcularLayerType::IPL);
			auto ret_thicks = bscan->getLayerThickness(OcularLayerType::ILM, OcularLayerType::RPE);

			auto line_para_max = 0.0f;
			auto line_cent_x = 0;
			auto line_cent_y = 0;
			auto line_cent_t = 0.0f;

			if (all_of(ipl_thicks.cbegin(), ipl_thicks.cend(), [](int e) { return e <= 0; }) ||
				ipl_thicks.size() != numPoints || ret_thicks.size() != numPoints) {
				continue;
			}

			for (int y = search_y_sidx; y <= search_y_eidx; y++) {
				auto cent_t = ipl_thicks[y];
				auto para_out_y = max(y - (int)(numPoints * (FOVEA_PARA_OUTER_RADIUS_IN_MM / rangeY)), 0);
				auto para_inn_y = max(y - (int)(numPoints * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeY)), 0);

				auto para_area1 = 0.0f;
				auto para_tmax1 = 0.0f;
				auto para_mean1 = 0.0f;
				auto para_count = 0;

				for (int i = para_out_y; i <= para_inn_y; i++) {
					auto thick = (ipl_thicks[i] - cent_t);
					if (thick >= 0.0f) {
						para_area1 += thick;
						para_count += 1;
						para_tmax1 = max(para_tmax1, thick);
					}
				}
				if (para_count > 0) {
					para_mean1 = para_area1 / para_count;
				}

				para_out_y = min(y + (int)(numPoints * (FOVEA_PARA_OUTER_RADIUS_IN_MM / rangeY)), numPoints - 1);
				para_inn_y = min(y + (int)(numPoints * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeY)), numPoints - 1);

				auto para_area2 = 0.0f;
				auto para_tmax2 = 0.0f;
				auto para_mean2 = 0.0f;
				para_count = 0;

				for (int i = para_inn_y; i <= para_out_y; i++) {
					auto thick = (ipl_thicks[i] - cent_t);
					if (thick >= 0.0f) {
						para_area2 += thick;
						para_count += 1;
						para_tmax2 = max(para_tmax2, thick);
					}
				}
				if (para_count > 0) {
					para_mean2 = para_area2 / para_count;
				}

				if (para_mean1 > 0.0f && para_mean2 > 0.0f) {
					auto area_mean = (para_mean1 + para_mean2) / 2.0f;
					if (line_para_max < area_mean) {
						line_para_max = area_mean;
						line_cent_t = ret_thicks[y];
						line_cent_x = x;
						line_cent_y = y;
					}
				}
			}

			if (paraMaxArea < line_para_max) {
				paraMaxArea = line_para_max;
				centerThick = line_cent_t;
				centerX = line_cent_x;
				centerY = line_cent_y;
			}

			fov_thicks[x] = line_cent_t;
			fov_idx_xs[x] = line_cent_x;
			fov_idx_ys[x] = line_cent_y;
		}
	}
	
	if (paraMaxArea > 16.0f) {
		/*
		if (getDescript()->isHorizontal()) {
			auto y1 = max(centerY - (int)(numLines * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeY)), 0);
			auto y2 = min(centerY + (int)(numLines * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeY)), numLines - 1);
			for (int y = y1; y <= y2; y++) {
				if (centerThick > fov_thicks[y]) {
					centerThick = fov_thicks[y];
					centerX = fov_idx_xs[y];
					centerY = fov_idx_ys[y];
				}
			}
		}
		else {
			auto x1 = max(centerX - (int)(numLines * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeX)), 0);
			auto x2 = min(centerX + (int)(numLines * (FOVEA_PARA_INNER_RADIUS_IN_MM / rangeX)), numLines - 1);
			for (int x = x1; x <= x2; x++) {
				if (centerThick > fov_thicks[x]) {
					centerThick = fov_thicks[x];
					centerX = fov_idx_xs[x];
					centerY = fov_idx_ys[x];
				}
			}
		}
		*/

		float xPosMM, yPosMM;
		if (getDescript()->isHorizontal()) {
			d_ptr->foveaCenterLine = centerY;
			d_ptr->foveaCenterXpos = centerX;
			xPosMM = (((float)centerX / (float)numPoints) - 0.5f) * rangeX;
			yPosMM = (((float)centerY / (float)numLines) - 0.5f) * rangeY;
		}
		else {
			d_ptr->foveaCenterLine = centerX;
			d_ptr->foveaCenterXpos = centerY;
			xPosMM = (((float)centerX / (float)numLines) - 0.5f) * rangeX;
			yPosMM = (((float)centerY / (float)numPoints) - 0.5f) * rangeY;
		}
		d_ptr->foveaCenterX = xPosMM;
		d_ptr->foveaCenterY = yPosMM;
		d_ptr->foveaCenterThickness = centerThick;
		d_ptr->isFoveaCenter = true;
	}
	else {
		d_ptr->isFoveaCenter = false;
	}
	return;
}


std::unique_ptr<SegmScan::OcularEnfaceImage> OctReport::MacularReport2::createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto enface = unique_ptr<MacularEnfaceImage>(new MacularEnfaceImage);

	const auto& desc = getProtocolData()->getDescript();
	const auto& bsegms = getProtocolData()->getBscanSegmList();

	bool result = enface->setupEnfaceImage(desc, upper, lower, upperOffset, lowerOffset, bsegms);

	if (result) {
		return std::move(enface);
	}
	return nullptr;
}


std::unique_ptr<SegmScan::OcularEnfaceMap> OctReport::MacularReport2::createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto plot = unique_ptr<MacularEnfaceMap>(new MacularEnfaceMap);

	const auto& desc = getProtocolData()->getDescript();
	const auto& bsegms = getProtocolData()->getBscanSegmList();

	bool result = plot->setupThicknessMap(desc, upper, lower, upperOffset, lowerOffset, bsegms);

	if (result) {
		return std::move(plot);
	}
	return nullptr;
}


MacularReport2::MacularReport2Impl & OctReport::MacularReport2::getImpl(void) const
{
	return *d_ptr;
}