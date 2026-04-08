#include "stdafx.h"
#include "AngioDecorr.h"
#include "AngioLayers.h"
#include "AngioLayout.h"
#include "AngioData.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>
#include <vector>

#include <boost\range\combine.hpp>
#include <boost\range\irange.hpp>


struct AngioDecorr::AngioDecorrImpl
{
	vector<CvImage> decorrelations;
	vector<CvImage> differentials;
	vector<CvImage> intensities;
	vector<CvImage> differWeights;

	vector<float> decorrProjection;
	vector<float> decorrProjectionMax;
	vector<float> differProjection;
	vector<float> intensProjection;

	vector<float> decorrAngiogram;
	vector<float> differAngiogram;
	vector<float> outputAngiogram;
	vector<float> offsetProjection;

	vector<float> decorrProjectionMask;
	vector<float> differProjectionMask;

	vector<vector<float>> bscanDecorrLists;
	vector<vector<float>> bscanIntensLists;
	vector<float> bscanDecorrMeans;
	vector<float> bscanDecorrStdevs;
	vector<float> bscanDecorrMaxVals;
	vector<float> bscanIntensMeans;
	vector<float> bscanIntensStdevs;
	vector<float> bscanIntensMaxVals;

	vector<float> bscanMotionTransX;
	vector<float> bscanMotionTransY;
	vector<float> bscanMotionEccVal;

	float decorrMean = 0.0f;
	float decorrStdev = 0.0f;
	float decorrMaxVal = 0.0f;
	float intensMean = 0.0f;
	float intensStdev = 0.0f;
	float intensMaxVal = 0.0f;
	float differMean = 0.0f;
	float differStdev = 0.0f;
	float differMaxVal = 0.0f;

	float lowerThreshold = 0.0f;
	float upperThreshold = 2500.0f;
	float backgroundThreshold = 0.0f;
	float backgroundMean = 0.0f;
	float backgroundStdev = 0.0f;

	vector<float> bgThresholds;
	vector<float> bgMeans;
	vector<float> bgStdDevs;

	vector<float> amplitMeans;
	vector<float> amplitStdevs;
	vector<float> amplitThresholds;
	vector<float> layerThreshold1;
	vector<float> layerThreshold2;
	vector<float> layerThreshold3;
	int projectWidth = 0;
	int projectHeight = 0;

	AngioDecorrImpl() {

	};
};

AngioDecorr::AngioDecorr()
	: d_ptr(make_unique<AngioDecorrImpl>())
{
}


AngioDecorr::~AngioDecorr()
{
	// Destructor should be defined for unique_ptr to delete AngioDecorrImpl as an incomplete type.
}


OctAngio::AngioDecorr::AngioDecorr(AngioDecorr && rhs) = default;
AngioDecorr & OctAngio::AngioDecorr::operator=(AngioDecorr && rhs) = default;



auto OctAngio::AngioDecorr::estimateThreshold(const AngioLayout & layout, const AngioData & data, const AngioLayers & layers) -> bool
{
	auto lines = layout.numberOfLines();
	LayerArrays ilms = layers.getUpperLayers(OcularLayerType::ILM);
	LayerArrays nfls = layers.getUpperLayers(OcularLayerType::NFL);
	LayerArrays ioss = layers.getUpperLayers(OcularLayerType::IOS);
	LayerArrays brms = layers.getLowerLayers(OcularLayerType::BRM);

	getImpl().amplitMeans = vector<float>(lines, 0.0f);
	getImpl().amplitStdevs = vector<float>(lines, 0.0f);
	getImpl().amplitThresholds = vector<float>(lines, 0.0f);
	getImpl().layerThreshold1 = vector<float>(lines, 0.0f);
	getImpl().layerThreshold2 = vector<float>(lines, 0.0f);
	getImpl().layerThreshold3 = vector<float>(lines, 0.0f);

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, &layout, &data, &ilms, &nfls, &ioss, &brms, this]()
			{
				auto width = data.getAmplitudes()[0][0].getWidth();
				auto height = data.getAmplitudes()[0][0].getHeight();

				for (int line_idx : tasks[k]) {
					auto& amplit = data.getAmplitudes()[line_idx][0];
					auto p_buff = (const float*)amplit.getBitsData();

					auto vec1 = vector<float>();
					auto vec2 = vector<float>();
					auto vec3 = vector<float>();
					for (int x = 0; x < width; x++) {
						auto ilm = min(max(ilms[line_idx][x], 0), height - 1);
						auto nfl = min(max(nfls[line_idx][x], 0), height - 1);
						auto ios = min(max(ioss[line_idx][x], 0), height - 1);
						auto brm = min(max(brms[line_idx][x], 0), height - 1);
						auto chr = min(brm + ANGIO_BRM_TO_CHOROID_HEIGHT, height - 1);

						for (int y = 0; y < height; y++) {
							auto val = p_buff[y*width + x]; 
							if (y >= ilm && y <= ios) {	// Inner retina
								vec1.push_back(val);
							}
							if (y >= ios && y <= chr) {	// Outer retina
								vec2.push_back(val);
							}
							if (y >= ilm && y <= chr) { // Total retina
								vec3.push_back(val);
							}
						}
					}

					float mean1 = accumulate(vec1.begin(), vec1.end(), 0) / (vec1.size() + 1);
					float mean2 = accumulate(vec2.begin(), vec2.end(), 0) / (vec2.size() + 1);
					float mean3 = accumulate(vec3.begin(), vec3.end(), 0) / (vec3.size() + 1);
					
					float diff1 = 0.0f, diff2 = 0.0f, diff3 = 0.0f;
					for (auto elem : vec1) {
						diff1 += (elem - mean1) * (elem - mean1);
					}
					for (auto elem : vec2) {
						diff2 += (elem - mean2) * (elem - mean2);
					}
					for (auto elem : vec3) {
						diff3 += (elem - mean3) * (elem - mean3);
					}
					float stde1 = sqrt(diff1 / vec1.size());
					float stde2 = sqrt(diff2 / vec2.size());
					float stde3 = sqrt(diff3 / vec3.size());

					float thresh1 = mean1 + stde1 * ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_INNER;
					float thresh2 = mean2 + stde2 * ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_OUTER;
					float thresh3 = mean3 + stde3 * ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_TOTAL;

					getImpl().layerThreshold1[line_idx] = thresh1;
					getImpl().layerThreshold2[line_idx] = thresh2;
					getImpl().layerThreshold3[line_idx] = thresh3;

					float mean, sdev, thresh;
					amplit.getMeanStddev(&mean, &sdev);
					thresh = mean + sdev * ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO;
					thresh = max(thresh, ANGIO_BSCAN_THRESH_MIN);

					getImpl().amplitMeans[line_idx] = (float)mean;
					getImpl().amplitStdevs[line_idx] = (float)sdev;
					getImpl().amplitThresholds[line_idx] = thresh;

					// LogD() << "Inner retina, line: " << line_idx << ", mean: " << mean1 << " (" << stde1 << "), regions: " << vec1.size() << ", thresh: " << thresh1;
					// LogD() << "Outer retina, line: " << line_idx << ", mean: " << mean2 << " (" << stde2 << "), regions: " << vec2.size() << ", thresh: " << thresh2;
					// LogD() << "Total retina, line: " << line_idx << ", mean: " << mean3 << " (" << stde3 << "), regions: " << vec3.size() << ", thresh: " << thresh3;
					// LogD() << "Angio b-scan, line: " << line_idx << ", mean: " << mean << " (" << sdev << "), regions: " << amplit.getWidth() * amplit.getHeight() << ", thresh: " << thresh;
				}
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});
	return true;
}


auto OctAngio::AngioDecorr::calculateSignals(const AngioLayout& layout, const AngioData& data, const AngioLayers& layers, 
	int overlaps, int avgOffsetInn, int avgOffsetOut, float threshRatioInn, float threshRatioOut) -> bool
{
	CppUtil::ClockTimer::start();
	LogD() << "Calculating angio signals, lines: " << layout.numberOfLines() << ", repeats: " << layout.numberOfRepeats() << ", points: " << layout.numberOfPoints();

	auto lines = layout.numberOfLines();
	getImpl().decorrelations = vector<CvImage>(lines);
	getImpl().differentials = vector<CvImage>(lines);
	getImpl().intensities = vector<CvImage>(lines);

	auto decorr_max = vector<float>(lines);
	auto differ_max = vector<float>(lines);
	auto decorr_min = vector<float>(lines);
	auto differ_min = vector<float>(lines);

	// Retian region in where signals calculated. 
	LayerArrays layers1 = layers.getUpperLayers(DECORR_UPPER_LAYER_TYPE, DECORR_UPPER_LAYER_OFFSET);
	LayerArrays layers2 = layers.getLowerLayers(DECORR_LOWER_LAYER_TYPE, DECORR_LOWER_LAYER_OFFSET);

	LayerArrays ilms = layers.getUpperLayers(OcularLayerType::ILM);
	LayerArrays nfls = layers.getUpperLayers(OcularLayerType::NFL);
	LayerArrays opls = layers.getUpperLayers(OcularLayerType::OPL);
	LayerArrays ioss = layers.getUpperLayers(OcularLayerType::IOS);
	LayerArrays brms = layers.getLowerLayers(OcularLayerType::BRM);

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, &layout, &data, &layers1, &layers2, &ilms, &nfls, &opls, &ioss, &brms,
				avgOffsetInn, avgOffsetOut, overlaps, threshRatioInn, threshRatioOut,
				&decorr_max, &decorr_min, &differ_max, &differ_min, this]()
				{
					auto width = data.getAmplitudes()[0][0].getWidth();
					auto height = data.getAmplitudes()[0][0].getHeight();
					auto repeats = layout.numberOfRepeats();

					for (int line_idx : tasks[k]) {
						vector<float> decorrs = vector<float>(width * height, 0.0f);
						vector<float> differs = vector<float>(width * height, 0.0f);
						vector<float> intenss = vector<float>(width * height, 0.0f);

						const auto& images = data.getAmplitudes()[line_idx];
						const auto& uppers = layers1[line_idx];
						const auto& lowers = layers2[line_idx];

						auto bg_mean = getImpl().amplitMeans[line_idx];
						auto bg_stdv = getImpl().amplitStdevs[line_idx];
						auto athresh = getImpl().amplitThresholds[line_idx];

						const auto thresh1 = getImpl().layerThreshold1[line_idx];
						const auto thresh2 = getImpl().layerThreshold2[line_idx];
						const auto thresh3 = getImpl().layerThreshold3[line_idx];

						// thresh1 = max(max(thresh1, (int)bg_mean), (int)bg_stdv);
						LogD() << "Angio line: " << line_idx << ", mean: " << bg_mean << ", stdev: " << bg_stdv << ", thresh: " << athresh << ", regional : " << thresh1 << ", " << thresh2 << ", " << thresh3;

						vector<pair<int, int>> list;
						for (auto t = 1; t < images.size(); t++) {
							for (auto u = 0; u < (images.size() - t); u++) {
								list.emplace_back(u, u + t);
							}
						}

						auto lsize = list.size();
						for (auto pair : list) {
							auto img1 = pair.first;
							auto img2 = pair.second;
							auto p1 = (float*)images[img1].getBitsData();
							auto p2 = (float*)images[img2].getBitsData();

							if (img1 == 0 && img2 == 1) {
								// LogD() << "Line index: " << line_idx << ", img1: " << img1 << ", img2: " << img2;
							}
							else {
								// continue;
							}

							for (auto c = 0; c < width; c++) {
								auto r1 = uppers[c];
								auto r2 = lowers[c];

								const auto ilm = ilms[line_idx][c];
								const auto nfl = nfls[line_idx][c];
								const auto opl = opls[line_idx][c];
								const auto ios = ioss[line_idx][c];
								const auto brm = brms[line_idx][c];

								for (auto r = r1; r <= r2; r++) {
									auto idx1 = r * width + c;
									auto y1 = r + (r < ios ? avgOffsetInn * -1 : avgOffsetOut * -1);
									auto y2 = r + (r < ios ? avgOffsetInn * +1 : avgOffsetOut * +1);

									y1 = max(y1, 0);
									y2 = min(y2, height - 1);

									float a1 = 0.0f;
									float a2 = 0.0f;
									int count = 0; 
									for (int y = y1; y <= y2; y++) {
										auto idx2 = y * width + c;
										a1 += p1[idx2];
										a2 += p2[idx2];
										count += 1;
									}
									a1 /= count;
									a2 /= count;

									auto diff = 0.0f;
									auto deco = 0.0f;
									auto ints = (a1 + a2) / 2;
									auto thresh = athresh * (r < ios ? threshRatioInn : threshRatioOut);

									if ((a1 >= thresh || a2 >= thresh)) {
										deco = (std::pow((a1 - a2), 2.0f) / (std::pow(a1, 2.0f) + std::pow(a2, 2.0f)));
									}
									if ((a1 >= thresh || a2 >= thresh)) {
										diff = fabs(a1 - a2);
									}
									
									const auto AVASCULAR_HEIGHT = 7; 
									if (r <= min(nfl + AVASCULAR_HEIGHT, opl)) {
										auto dmax = (float)((nfl + AVASCULAR_HEIGHT) - ilm);
										auto dist = (float)(r - ilm);
										auto supp = (dmax > 0 ? (1.0f - (dist / dmax)) : 0.0f);
										diff = deco * diff * supp;
									}
									
									differs[idx1] = max(differs[idx1], diff);
									decorrs[idx1] = max(decorrs[idx1], deco);
									intenss[idx1] = max(intenss[idx1], ints);
								}
							}
						}

						decorr_max[line_idx] = *std::max_element(decorrs.begin(), decorrs.end());
						differ_max[line_idx] = *std::max_element(differs.begin(), differs.end());
						decorr_min[line_idx] = *std::min_element(decorrs.begin(), decorrs.end());
						differ_min[line_idx] = *std::min_element(differs.begin(), differs.end());

						getImpl().decorrelations[line_idx].fromFloat32((const unsigned char*)&decorrs[0], width, height);
						getImpl().differentials[line_idx].fromFloat32((const unsigned char*)&differs[0], width, height);
						getImpl().intensities[line_idx].fromFloat32((const unsigned char*)&intenss[0], width, height);
					};
				}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	float mean1 = 0.0f, stdev1 = 0.0f;
	float mean2 = 0.0f, stdev2 = 0.0f;
	double minval1 = 0.0, maxval1 = 0.0;
	double minval2 = 0.0, maxval2 = 0.0;
	for (int i = 0; i < lines; i++) {
		getImpl().decorrelations[i].getMeanStddev(&mean1, &stdev1);
		getImpl().differentials[i].getMeanStddev(&mean2, &stdev2);
		minval1 = decorr_min[i]; maxval1 = decorr_max[i];
		minval2 = differ_min[i]; maxval2 = differ_max[i];
		// LogD() << "Line index: " << i << ", decorr. mean: " << mean1 << ", stdev: " << stdev1 << ", min: " << minval1 << ", max: " << maxval1;
		// LogD() << "Line index: " << i << ", differ. mean: " << mean2 << ", stdev: " << stdev2 << ", min: " << minval2 << ", max: " << maxval2;
	}

	auto msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angio signal calculated, elapsed: " << msec;
	return true;
}


auto OctAngio::AngioDecorr::updateProjectionMask(const AngioLayout & layout, const LayerArrays & uppers, const LayerArrays & lowers, bool outerFlow, int avgSize) -> bool
{
	auto lines = layout.numberOfLines();
	auto points = layout.numberOfPoints();
	auto projSize = lines * points;

	auto vertical = layout.isVerticalScan();

	getImpl().differProjection = vector<float>(projSize, 0.0f);
	getImpl().decorrProjection = vector<float>(projSize, 0.0f);

	LogD() << "Updating projection masks, lines: " << lines << ", points: " << points << ", vertical: " << vertical << ", outerFlow: " << outerFlow << ", avgSize: " << avgSize;

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, vertical, outerFlow, &layout, &uppers, &lowers, &avgSize, this]()
			{
				auto lines = layout.numberOfLines();
				auto points = layout.numberOfPoints();
				auto height = DECORR_IMAGE_HEIGHT;

				for (auto line_idx : tasks[k]) {
					const auto &cImage = getImpl().decorrelations[line_idx];
					const auto &dImage = getImpl().differentials[line_idx];

					const float* decorr = (const float*)cImage.getBitsData();
					const float* differ = (const float*)dImage.getBitsData();

					if (uppers.size() < line_idx && lowers.size() < line_idx) {
						continue;
					}

					const auto& layer1 = uppers[line_idx];
					const auto& layer2 = lowers[line_idx];
					if (layer1.size() != points || layer2.size() != points) {
						continue;
					}

					for (auto c = 0; c < points; c++) {
						auto index = (vertical ? (c * lines + line_idx) : (line_idx * points + c));

						auto dcMax = 0.0f;
						auto dfMax = 0.0f;
						auto dcSum = 0.0f;
						auto dfSum = 0.0f;

						auto y1 = layer1[c];
						auto y2 = layer2[c];
						y1 = min(max(y1, 0), height - 1);
						y2 = min(max(y2, 0), height - 1);

						for (auto r = y1; r <= y2; r++) {
							auto w1 = r - avgSize / 2;
							auto w2 = r + avgSize / 2;
							w1 = max(w1, 0);
							w2 = min(w2, height - 1);

							auto dcsum = 0.0f;
							auto dfsum = 0.0f;
							auto dcval = 0.0f;
							auto dfval = 0.0f;
							auto count = 0;
							for (auto t = w1; t <= w2; t++) {
								auto idx2 = t * points + c;
								auto dc = decorr[idx2];
								auto df = differ[idx2];
								dcsum += dc;
								dfsum += df;
								count += 1;
							}
							dcval = (count == avgSize ? dcsum / count : 0.0f);
							dfval = (count == avgSize ? dfsum / count : 0.0f);
							dcMax = max(dcval, dcMax);
							dfMax = max(dfval, dfMax);
						}
						getImpl().decorrProjection[index] = dcMax;
						getImpl().differProjection[index] = dfMax;
					}
				}
			}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	if (!GlobalSettings::isUserModeSettings()) 
	{
		auto fpos = getImpl().decorrProjection.begin();
		auto epos = getImpl().decorrProjection.end();
		auto max_val = *max_element(fpos, epos);
		auto min_val = *min_element(fpos, epos);

		float mean, stdev;
		CvImage image;
		image.fromFloat32((const unsigned char*)&getImpl().decorrProjection[0], points, lines);
		image.getMeanStddev(&mean, &stdev);
		LogD() << "Vessel mask decorr. mean: " << mean << ", stdev: " << stdev << ", min: " << min_val << ", max: " << max_val;
	}

	if (!GlobalSettings::isUserModeSettings())
	{
		auto fpos = getImpl().differProjection.begin();
		auto epos = getImpl().differProjection.end();
		auto max_val = *max_element(fpos, epos);
		auto min_val = *min_element(fpos, epos);

		float mean, stdev;
		CvImage image;
		image.fromFloat32((const unsigned char*)&getImpl().differProjection[0], points, lines);
		image.getMeanStddev(&mean, &stdev);
		LogD() << "Vessel mask differ. mean: " << mean << ", stdev: " << stdev << ", min: " << min_val << ", max: " << max_val;
	}

	getImpl().projectWidth = layout.getWidth();
	getImpl().projectHeight = layout.getHeight();
	getImpl().differAngiogram = getImpl().differProjection;
	getImpl().decorrAngiogram = getImpl().decorrProjection;
	return true;
}

auto OctAngio::AngioDecorr::updateProjectionProfiles(const AngioLayout & layout, const AngioLayers & layers,
													bool calcStats, bool applyPAR, bool modelEye,
													const std::vector<float>& maskDecorr, const std::vector<float>& maskDiffer,
													int avgOffsetInn, int avgOffsetOut, float maskWeight) -> bool
{
	auto lines = layout.numberOfLines();
	auto points = layout.numberOfPoints();
	auto projSize = lines * points;
	auto maskMean = 0.0f;

	auto vertical = layout.isVerticalScan();
	auto outerFlow = layers.isOuterRetinaFlows();

	// Retian region in where signals calculated. 
	LayerArrays slabs1 = layers.getUpperLayersOfSlab();
	LayerArrays slabs2 = layers.getLowerLayersOfSlab();

	// OPL ~ (RPE + 25)
	LayerArrays nones1 = layers.getUpperLayersOfNonVascular();
	LayerArrays nones2 = layers.getLowerLayersOfNonVascular();

	// OPL ~ (BRM + 9)
	LayerArrays shade1 = layers.getUpperLayersOfShadowed2();
	LayerArrays shade2 = layers.getLowerLayersOfShadowed2();

	// NFL ~ (IPL + 15)
	LayerArrays vascs1 = layers.getUpperLayersOfVasculature();
	LayerArrays vascs2 = layers.getLowerLayersOfVasculature();

	LayerArrays nfls = layers.getUpperLayers(OcularLayerType::NFL);
	LayerArrays opls = layers.getUpperLayers(OcularLayerType::OPL);

	getImpl().differProjection = vector<float>(projSize, 0.0f);
	getImpl().decorrProjection = vector<float>(projSize, 0.0f);
	getImpl().decorrProjectionMax = vector<float>(projSize, 0.0f);
	getImpl().intensProjection = vector<float>(projSize, 0.0f);
	getImpl().offsetProjection = vector<float>(projSize, 0.0f);

	if (calcStats) {
		getImpl().bscanDecorrLists = vector<vector<float>>(lines);
		getImpl().bscanDecorrMeans = vector<float>(lines, 0.0f);
		getImpl().bscanDecorrStdevs = vector<float>(lines, 0.0f);
		getImpl().bscanDecorrMaxVals = vector<float>(lines, 0.0f);

		getImpl().bscanIntensLists = vector<vector<float>>(lines);
		getImpl().bscanIntensMeans = vector<float>(lines, 0.0f);
		getImpl().bscanIntensStdevs = vector<float>(lines, 0.0f);
		getImpl().bscanIntensMaxVals = vector<float>(lines, 0.0f);
	}

	bool projMask = applyPAR;
	if (projMask) {
		if (layout.scanRangeX() <= DECORR_PROJECT_REMOVE_RANGE_MAX &&
			maskDecorr.size() == projSize && maskDiffer.size() == projSize) {
			maskMean = std::accumulate(maskDecorr.begin(), maskDecorr.end(), 0.0f) / projSize;
			maskMean = maskMean * maskWeight;
		}
		else {
			projMask = false;
		}
	}

	bool isFovea = false;
	int fovea_cx = 0, fovea_cy = 0;
	if (!modelEye) {
		if (layout.isFoveaCenter()) {
			if (layout.getFoveaCenterInPixel(fovea_cx, fovea_cy)) {
				isFovea = true;
			}
		}
	}

	auto avgSize = (outerFlow ? avgOffsetOut : avgOffsetInn);
	LogD() << "Updating projection profiles, lines: " << lines << ", points: " << points << ", vertical: " << vertical << ", outFlow: " << outerFlow << ", projMask: " << projMask << ", avgSize: " << avgSize;

	auto n_workers = std::thread::hardware_concurrency();
	std::vector<std::vector<int>> tasks(n_workers);

	for (int i = 0; i < lines; i++) {
		int k = i % n_workers;
		tasks[k].push_back(i);
	}

	std::vector<std::thread> workers;
	for (unsigned int k = 0; k < n_workers; k++) {
		if (tasks[k].size() > 0) {
			workers.push_back(std::thread([tasks, k, calcStats, vertical, outerFlow, projMask, &layout, &layers,
				&slabs1, &slabs2, &nones1, &nones2, &shade1, &shade2, &vascs1, &vascs2,
				isFovea, fovea_cx, fovea_cy, &nfls, &opls,
				maskDecorr, maskDiffer, avgSize, maskWeight, modelEye, maskMean, this]()
				{
					auto lines = layout.numberOfLines();
					auto points = layout.numberOfPoints();
					auto height = DECORR_IMAGE_HEIGHT;
					auto angio_w = layout.getWidth();
					auto angio_h = layout.getHeight();

					for (auto line_idx : tasks[k]) {
						const auto& cImage = getImpl().decorrelations[line_idx];
						const auto& dImage = getImpl().differentials[line_idx];

						const float* decorr = (const float*)cImage.getBitsData();
						const float* differ = (const float*)dImage.getBitsData();
						const float* intens = nullptr;

						if (calcStats) {
							const auto& tImage = getImpl().intensities[line_idx];
							intens = (const float*)tImage.getBitsData();
						}

						const auto& uppers = slabs1[line_idx];
						const auto& lowers = slabs2[line_idx];

						bool useTailMask = false;
						if (vascs1.size() > line_idx && vascs1[line_idx].size() == points &&
							vascs2.size() > line_idx && vascs2[line_idx].size() == points) {
							useTailMask = true;
						}

						bool fromBase = false;
						if (shade1.size() > line_idx && shade1[line_idx].size() == points &&
							shade2.size() > line_idx && shade2[line_idx].size() == points) {
							fromBase = true;
						}

						for (auto c = 0; c < points; c++) {
							auto index = (vertical ? (c * lines + line_idx) : (line_idx * points + c));
							auto x = index % angio_w;
							auto y = index / angio_w;

							auto dcMax = 0.0f, dcSum = 0.0f, dcAvg = 0.0f;
							auto dfMax = 0.0f, dfSum = 0.0f, dfAvg = 0.0f;
							auto itMax = 0.0f;
							auto depth = 0;
							auto offset = 0.0f;
							auto halfSize = avgSize / 2;

							auto y1 = uppers[c];
							auto y2 = lowers[c];
							y1 = min(max(y1, 0), height - 1);
							y2 = min(max(y2, 0), height - 1);

							for (auto r = y1; r <= y2; r++) {
								auto w1 = r - avgSize / 2;
								auto w2 = r + avgSize / 2;
								w1 = max(w1, y1); // 0);
								w2 = min(w2, y2); // height - 1);

								auto dcsum = 0.0f;
								auto dfsum = 0.0f;
								auto dcval = 0.0f;
								auto dfval = 0.0f;
								auto count = 0;
								for (auto t = w1; t <= w2; t++) {
									auto idx2 = t * points + c;
									dcsum += decorr[idx2];
									dfsum += differ[idx2];
									count += 1;
								}
								dcval = dcsum / count;
								dfval = dfsum / count;

								auto opl = opls[line_idx][c];
								auto nfl = nfls[line_idx][c];

								/*
								if (count != avgSize) {
									dcval = 0.0f;
									dfval = 0.0f;
								}
								*/

								if (useTailMask) {
									if (projMask && fromBase) {
										auto nones_y1 = nones1[line_idx][c];
										auto nones_y2 = nones2[line_idx][c];
										auto shade_y1 = shade1[line_idx][c];
										auto shade_y2 = shade2[line_idx][c];

										if (r >= shade_y1) { // }&& r <= shade_y2) {
											auto coeff = max(maskDecorr[index], maskMean);
											if (coeff > 0.001f) { // ANGIO_PROJ_MASK_VALUE_MIN) {
												auto ratio = (1.0f - log10(10.0f * coeff + 0.001f)); // *maskWeight); // *0.25f;
												ratio = min(max(ratio, 0.0f), 1.0f);
												dcval = dcval * ratio;
												dfval = dfval * ratio;
											}
										}
									}
								}
								else {
									// Without PAR.
								}

								/*
								if (modelEye) {
									if (r <= nfl) {
										auto decay = 0.65f;
										auto depth = (nfl - r);
										dcval = dcval * exp(-decay * depth);
										dfval = dfval * exp(-decay * depth);
									}
								}
								*/
								if (isFovea) {
									const auto AVASCULAR_HEIGHT = 7;
									if (r <= min(nfl + AVASCULAR_HEIGHT, opl)) {
										auto pixelSizeX = layout.getPixelSizeX();
										auto pixelSizeY = layout.getPixelSizeY();
										auto x_dist = abs(c - fovea_cx) * pixelSizeX;
										auto y_dist = abs(line_idx - fovea_cy) * pixelSizeY;
										auto dist = sqrt((x_dist * x_dist) + (y_dist * y_dist));
										const auto FOVEA_CENTER_RADIUS = 0.45f;
										if (dist < FOVEA_CENTER_RADIUS) {
											auto ratio = dist * dist * 5.0f;
											dcval = dcval * ratio;
											dfval = dfval * ratio;
										}
									}
								}
								
								if (dcMax <= dcval) {
									if (fromBase) {
										offset = max((float)(shade2[line_idx][c] - r), 0.0f);
									}
									else {
										offset = max((float)(lowers[c] - r), 0.0f);
									}
								}
								dcMax = max(dcval, dcMax);
								dfMax = max(dfval, dfMax);
								dcSum += dcval; // dcsum;
								dfSum += dfval; // dfsum;
							
								if (calcStats) {
									auto idx2 = r * points + c;
									auto it = intens[idx2];
									if (itMax < it) {
										itMax = it;
									}
									getImpl().bscanDecorrLists[line_idx].push_back(dcval);
									getImpl().bscanIntensLists[line_idx].push_back(it);
								}
								depth += 1;
							}

							dcAvg = (depth > 0.0f ? (dcSum / depth) : 0.0f);
							dfAvg = (depth > 0.0f ? (dfSum / depth) : 0.0f);

							getImpl().decorrProjection[index] = dcMax; // dcSum;
							getImpl().differProjection[index] = dfMax; // dfSum;
							getImpl().decorrProjectionMax[index] = dcMax;
							getImpl().intensProjection[index] = itMax;
							getImpl().offsetProjection[index] = offset;
						}
					}
				}));
		}
	}

	std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
		t.join();
	});

	{
		auto fpos = getImpl().decorrProjection.begin();
		auto epos = getImpl().decorrProjection.end();
		auto max_val = *max_element(fpos, epos);
		auto min_val = *min_element(fpos, epos);

		float mean, stdev;
		CvImage image;
		image.fromFloat32((const unsigned char*)&getImpl().decorrProjection[0], points, lines);
		image.getMeanStddev(&mean, &stdev);
		LogD() << "Projection decorr. mean: " << mean << ", stdev: " << stdev << ", min: " << min_val << ", max: " << max_val;
	}

	{
		auto fpos = getImpl().differProjection.begin();
		auto epos = getImpl().differProjection.end();
		auto max_val = *max_element(fpos, epos);
		auto min_val = *min_element(fpos, epos);

		float mean, stdev;
		CvImage image;
		image.fromFloat32((const unsigned char*)&getImpl().differProjection[0], points, lines);
		image.getMeanStddev(&mean, &stdev);
		LogD() << "Projection differ. mean: " << mean << ", stdev: " << stdev << ", min: " << min_val << ", max: " << max_val;
	}

	getImpl().projectWidth = layout.getWidth();
	getImpl().projectHeight = layout.getHeight();
	getImpl().differAngiogram = getImpl().differProjection;
	getImpl().decorrAngiogram = getImpl().decorrProjection;
	return true;
}


bool OctAngio::AngioDecorr::checkIfDecorrelationsLoaded(int lines, int points, int repeats)
{
	if (getImpl().decorrelations.size() != lines || getImpl().differentials.size() != lines) {
		return false; 
	}

	if (getImpl().decorrelations.empty() || getImpl().differentials.empty()) {
		return false;
	}

	if (getImpl().decorrelations[0].getWidth() != points ||
		getImpl().differentials[0].getWidth() != points) {
		return false;
	}
	return true;
}


bool OctAngio::AngioDecorr::calculateBscanImageStats(const AngioLayout & layout)
{
	int lines = layout.numberOfLines();
	if (getImpl().bscanDecorrLists.size() != lines ||
		getImpl().bscanIntensLists.size() != lines) {
		return false;
	}


	for (int i = 0; i < lines; i++) {
		const auto& decors = getImpl().bscanDecorrLists[i];
		const auto& intens = getImpl().bscanIntensLists[i];

		if (decors.size() > 100) {
			auto sorted = decors;
			sort(sorted.begin(), sorted.end(), greater<float>());
			auto quater = (int)(sorted.size() * 0.05f);
			auto value = sorted[quater];

			auto dvect = vector<float>(quater);
			auto ivect = vector<float>(quater);
			int count = 0;

			for (int i = 0; i < decors.size(); i++) {
				if (decors[i] >= value) {
					dvect[count] = decors[i];
					ivect[count] = intens[i];
					if (++count >= quater) {
						break;
					}
				}
			}

			if (dvect.size() > 0)
			{
				auto first = dvect.begin();
				auto last = dvect.end();
				auto sum = accumulate(first, last, 0.0f);
				auto mean = sum / quater;
				auto vmax = *max_element(dvect.begin(), dvect.end());

				vector<float> diff(quater);
				std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
				auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
				auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

				getImpl().bscanDecorrMeans[i] = mean;
				getImpl().bscanDecorrStdevs[i] = stdev;
				getImpl().bscanDecorrMaxVals[i] = vmax;
			}
			else {
				getImpl().bscanDecorrMeans[i] = 0.0f;
				getImpl().bscanDecorrStdevs[i] = 0.0f;
				getImpl().bscanDecorrMaxVals[i] = 0.0f;
			}

			if (ivect.size() > 0)
			{
				auto first = ivect.begin();
				auto last = ivect.end();
				auto sum = accumulate(first, last, 0.0f);
				auto mean = sum / quater;
				auto vmax = *max_element(ivect.begin(), ivect.end());

				vector<float> diff(quater);
				std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
				auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
				auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

				getImpl().bscanIntensMeans[i] = mean;
				getImpl().bscanIntensStdevs[i] = stdev;
				getImpl().bscanIntensMaxVals[i] = vmax;
			}
			else {
				getImpl().bscanIntensMeans[i] = 0.0f;
				getImpl().bscanIntensStdevs[i] = 0.0f;
				getImpl().bscanIntensMaxVals[i] = 0.0f;
			}
		}
		else {
			getImpl().bscanDecorrMeans[i] = 0.0f;
			getImpl().bscanDecorrStdevs[i] = 0.0f;
			getImpl().bscanDecorrMaxVals[i] = 0.0f;
			getImpl().bscanIntensMeans[i] = 0.0f;
			getImpl().bscanIntensStdevs[i] = 0.0f;
			getImpl().bscanIntensMaxVals[i] = 0.0f;
		}
	}
	return true;
}


bool OctAngio::AngioDecorr::calculateProjectionStats(bool modelEye)
{
	auto intens = getImpl().intensProjection;
	auto decors = getImpl().decorrProjectionMax;
	auto differs = getImpl().differProjection;

	for (int t = 0; t <= 3; t++)
	{
		auto percent = (t * 30 + 10) / 100.0f;
		auto sorted = decors;
		sort(sorted.begin(), sorted.end(), greater<float>());
		auto quater = (int)(sorted.size() * percent) - 1;
		auto value = sorted[quater];

		auto dvect = vector<float>(quater);
		auto ivect = vector<float>(quater);
		auto fvect = vector<float>(quater);
		int count = 0;

		for (int i = 0; i < decors.size(); i++) {
			if (decors[i] >= value) {
				dvect[count] = decors[i];
				ivect[count] = intens[i];
				fvect[count] = differs[i];
				if (++count >= quater) {
					break;
				}
			}
		}

		{
			auto first = dvect.begin();
			auto last = dvect.end();
			auto sum = accumulate(first, last, 0.0f);
			auto mean = sum / quater;
			auto vmax = *max_element(dvect.begin(), dvect.end());

			vector<float> diff(quater);
			std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
			auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
			auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

			getImpl().decorrMean = mean;
			getImpl().decorrStdev = stdev;
			getImpl().decorrMaxVal = vmax;
			LogD() << "Projection decorrs, p-" << percent << ", avg: " << mean << ", stdev: " << stdev << ", max: " << vmax;
		}

		{
			auto first = fvect.begin();
			auto last = fvect.end();
			auto sum = accumulate(first, last, 0.0f);
			auto mean = sum / quater;
			auto vmax = *max_element(fvect.begin(), fvect.end());

			vector<float> diff(quater);
			std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
			auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
			auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

			getImpl().differMean = mean;
			getImpl().differStdev = stdev;
			getImpl().differMaxVal = vmax;

			LogD() << "Projection differs, p-" << percent << ", avg: " << mean << ", stdev: " << stdev << ", max: " << vmax;
		}

		{
			auto first = ivect.begin();
			auto last = ivect.end();
			auto sum = accumulate(first, last, 0.0f);
			auto mean = sum / quater;
			auto vmax = *max_element(ivect.begin(), ivect.end());

			vector<float> diff(quater);
			std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
			auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
			auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

			getImpl().intensMean = mean;
			getImpl().intensStdev = stdev;
			getImpl().intensMaxVal = vmax;

			LogD() << "Projection intenss, p-" << percent << ", avg: " << mean << ", stdev: " << stdev << ", max: " << vmax;
		}
	}

	if (modelEye) 
	{
		auto percent = 0.25f;
		auto sorted = decors;
		sort(sorted.begin(), sorted.end(), greater<float>());
		auto quater = (int)(sorted.size() * percent) - 1;
		auto value = sorted[quater];

		auto dvect = vector<float>(quater);
		auto ivect = vector<float>(quater);
		auto fvect = vector<float>(quater);
		int count = 0;

		for (int i = 0; i < decors.size(); i++) {
			if (decors[i] < value) {
				dvect[count] = decors[i];
				ivect[count] = intens[i];
				fvect[count] = differs[i];
				if (++count >= quater) {
					break;
				}
			}
		}

		{
			auto first = dvect.begin();
			auto last = dvect.end();
			auto sum = accumulate(first, last, 0.0f);
			auto mean = sum / quater;
			auto vmax = *max_element(dvect.begin(), dvect.end());

			vector<float> diff(quater);
			std::transform(first, last, diff.begin(), [mean](float x) { return x - mean; });
			auto sq_sum = inner_product(diff.begin(), diff.end(), diff.begin(), 0.0f);
			auto stdev = (float)sqrt(sq_sum / (quater - 1.0f));

			getImpl().decorrMean = mean;
			getImpl().decorrStdev = stdev;
			getImpl().decorrMaxVal = vmax;
			LogD() << "Background decorrs, p-" << percent << ", avg: " << mean << ", stdev: " << stdev << ", max: " << vmax;
		}
	}

	return true;
}


bool OctAngio::AngioDecorr::normalizeProjectionProfiles(const AngioLayout& layout, const AngioLayers& layers, float decorrMin, float decorrMax, float differMin, float differMax, float dropoff)
{
	normalizeDifferAngiogram(layout, layers, differMin, differMax, dropoff, false);
	normalizeDecorrAngiogram(layout, layers, decorrMin, decorrMax, dropoff, false);
	return true;
}


bool OctAngio::AngioDecorr::normalizeDifferAngiogram(const AngioLayout & layout, const AngioLayers & layers, float rangeMin, float rangeMax, float sizeMin, bool modelEye)
{
	int w = layout.getWidth();
	int h = layout.getHeight();

	auto& projs = getImpl().differAngiogram;
	if (projs.size() <= 0) {
		return false;
	}

	auto fpos = projs.begin();
	auto epos = projs.end();
	auto max_val = *max_element(fpos, epos);
	auto min_val = *min_element(fpos, epos);

	float mean, stdev;
	CvImage image;
	image.fromFloat32((const unsigned char*)&projs[0], w, h);
	image.getMeanStddev(&mean, &stdev);

	float rangeDiff = (mean + stdev) * 0.5f;
	rangeDiff = max(rangeDiff, sizeMin);
	
	auto data = projs;
	std::sort(data.begin(), data.end());

	auto min_idx = (int)((rangeMin / 100.0) * (data.size() - 1));
	auto max_idx = (int)((rangeMax / 100.0) * (data.size() - 1));
	min_idx = min(max(min_idx, 0), (int)data.size() - 1);
	max_idx = min(max(max_idx, 0), (int)data.size() - 1);

	auto min_norm = data[min_idx];
	auto max_norm = data[max_idx];

	if (modelEye) {
		min_norm = min(min_val, 0.0f);
		max_norm = max(max_val, 2500.0f);
	}
	else {
		if ((max_norm - min_norm) < rangeDiff) {
			max_norm = min_norm + rangeDiff;
		}
	}

	std::for_each(projs.begin(), projs.end(), [&](float &elem) { elem = max(elem, min_norm); });
	std::for_each(projs.begin(), projs.end(), [&](float &elem) { elem = min(elem, max_norm); });

	LogD() << "Angio normalization (Differ.), min: " << min_val << ", max: " << max_val << ", mean: " << mean << ", stdev: " << stdev << ", meye: " << modelEye;
	LogD() << "Differ. Normalization range: " << rangeMin << " ~ " << rangeMax << ", value: " << min_norm << " ~ " << max_norm;

	if (max_norm > 0.0f) {
		if (max_norm > min_norm) {
			std::transform(fpos, epos, fpos, bind2nd(minus<float>(), min_norm));
			std::transform(fpos, epos, fpos, bind2nd(divides<float>(), (max_norm - min_norm)));
		}
	}
	return true;
}

bool OctAngio::AngioDecorr::normalizeDecorrAngiogram(const AngioLayout& layout, const AngioLayers& layers, float rangeMin, float rangeMax, float sizeMin, bool modelEye)
{
	int w = layout.getWidth();
	int h = layout.getHeight();

	auto& projs = getImpl().decorrAngiogram;
	if (projs.size() <= 0) {
		return false;
	}

	CvImage image;
	float mean, stdev;

	if (layers.isOuterRetinaFlows())
	{
		image.fromFloat32((const unsigned char*)&projs[0], w, h);
		image.applyGuidedFilter();
		image.getMeanStddev(&mean, &stdev);
		auto vect = image.copyDataInFloats();

		auto min_proj = *std::min_element(projs.begin(), projs.end());
		auto max_proj = *std::max_element(projs.begin(), projs.end());
		auto min_blur = *std::min_element(vect.begin(), vect.end());
		auto max_blur = *std::max_element(vect.begin(), vect.end());
		LogD() << "Decorr. projection, min: " << min_proj << ", max: " << max_proj << ", blurred min: " << min_blur << ", max: " << max_blur;

		auto vals = vect;
		std::sort(vals.begin(), vals.end());

		auto range_w_min = 0.0f;// rangeMin;
		auto range_w_max = 100.0f;// rangeMax;
		auto w_idx1 = (int)((range_w_min / 100.0) * (vals.size() - 1));
		auto w_idx2 = (int)((range_w_max / 100.0) * (vals.size() - 1));
		w_idx1 = min(max(w_idx1, 0), (int)vals.size() - 1);
		w_idx2 = min(max(w_idx2, 0), (int)vals.size() - 1);
		auto min_wdat = vals[w_idx1];
		auto max_wdat = vals[w_idx2];
		auto wdat_size = (max_wdat - min_wdat + 0.00001f);

		auto y1 = (h * 0.0f);
		auto y2 = (h * 1.0f);
		auto x1 = (w * 0.15f);
		auto x2 = (w * 0.85f);
		auto wsum = 0.0f;
		auto xsum = 0.0f;
		auto ysum = 0.0f;

		for (int y = y1; y < y2; y++) {
			for (int x = x1; x < x2; x++) {
				auto idx = y * w + x;
				auto val = vect[idx];
				val = (val - min_wdat) / wdat_size;
				val = max(val, 0.0f);
				val = pow(val, 2.0);
				wsum += val;
				xsum += (x * val);
				ysum += (y * val);
			}
		}
		auto xcen = 0.0f;
		auto ycen = 0.0f;
		xcen = xsum / wsum;
		ycen = ysum / wsum;

		LogD() << "Decorr. masks norm. range: " << range_w_min << " ~ " << range_w_max << ", min: " << min_wdat << ", max: " << max_wdat << ", xcen: " << xcen << ", ycen: " << ycen;

		auto alpha = 0.5f;
		auto pi = (float)NumericFunc::PI();
		auto tsize = 256.0f;
		auto wghts = vector<float>(w * h);
		auto supps = vector<float>(w * h);

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				auto idx = y * w + x;
				auto r = sqrt(pow((x - xcen), 2.0f) + pow((y - ycen), 2.0f));
				auto t = min(r / tsize, 1.0f);
				auto A = 1.0f - alpha * (1.0f - std::cos(pi * t)) * 0.5f;
				auto W = (vect[idx] * A * 10.0f);
				wghts[idx] = W;
			}
		}

		std::transform(projs.begin(), projs.end(), wghts.begin(), supps.begin(), [](float e1, float e2) { return e1 * e2; });
		{
			auto min_wght = *std::min_element(wghts.begin(), wghts.end());
			auto max_wght = *std::max_element(wghts.begin(), wghts.end());
			auto min_supp = *std::min_element(supps.begin(), supps.end());
			auto max_supp = *std::max_element(supps.begin(), supps.end());
			LogD() << " Decorr. weight, min: " << min_wght << ", max: " << max_wght << ", suppressed min: " << min_supp << ", max: " << max_supp;
		}

		wsum = xsum = ysum = 0.0f;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				auto idx = y * w + x;
				auto val = supps[idx];
				val = max(val, 0.0f);
				val = pow(val, 2.0);
				wsum += val;
				xsum += (x * val);
				ysum += (y * val);
			}
		}
		xcen = xsum / wsum;
		ycen = ysum / wsum;

		auto min_supp = *std::min_element(supps.begin(), supps.end());
		auto max_supp = *std::max_element(supps.begin(), supps.end());
		alpha = max_supp / (1.0f + fabs(max_supp));
		alpha = min(max(alpha, 0.1f), 0.5f);

		LogD() << "Decorr. recenter, xcen: " << xcen << ", ycen: " << ycen << ", alpha: " << alpha;

		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				auto idx = y * w + x;
				auto r = sqrt(pow((x - xcen), 2.0f) + pow((y - ycen), 2.0f));
				auto t = min(r / tsize, 1.0f);
				auto A = 1.0f - alpha * (1.0f - std::cos(pi * t)) * 0.5f;
				auto W = (vect[idx] * A * 10.0f);
				wghts[idx] = W;
			}
		}
		std::transform(projs.begin(), projs.end(), wghts.begin(), supps.begin(), [](float e1, float e2) { return e1 * e2; });
		std::copy(supps.begin(), supps.end(), projs.begin());
		{
			auto min_wght = *std::min_element(wghts.begin(), wghts.end());
			auto max_wght = *std::max_element(wghts.begin(), wghts.end());
			auto min_supp = *std::min_element(supps.begin(), supps.end());
			auto max_supp = *std::max_element(supps.begin(), supps.end());
			LogD() << " Decorr. weight, min: " << min_wght << ", max: " << max_wght << ", suppressed min: " << min_supp << ", max: " << max_supp;
		}
		/*
		std::sort(supps.begin(), supps.end());
		auto s_idx1 = (int)((99.9 / 100.0) * (vals.size() - 1));
		auto s_idx2 = (int)((99.5 / 100.0) * (vals.size() - 1));
		auto s_idx3 = (int)((99 / 100.0) * (vals.size() - 1));
		auto s_idx4 = (int)((97.5 / 100.0) * (vals.size() - 1));
		auto s_idx5 = (int)((95 / 100.0) * (vals.size() - 1));
		LogD() << "Suppressed, " << supps[s_idx1] << ", " << supps[s_idx2] << ", " << supps[s_idx3] << ", " << supps[s_idx4] << ", " << supps[s_idx5];
		*/
	}

	auto fpos = projs.begin();
	auto epos = projs.end();
	auto max_val = *max_element(fpos, epos);
	auto min_val = *min_element(fpos, epos);

	image.fromFloat32((const unsigned char*)&projs[0], w, h);
	image.getMeanStddev(&mean, &stdev);
	float rangeDiff = (mean + stdev) * 0.5f;
	rangeDiff = max(rangeDiff, sizeMin);

	auto data = projs;
	std::sort(data.begin(), data.end());

	auto min_idx = (int)((rangeMin / 100.0) * (data.size() - 1));
	auto max_idx = (int)((rangeMax / 100.0) * (data.size() - 1));
	min_idx = min(max(min_idx, 0), (int)data.size() - 1);
	max_idx = min(max(max_idx, 0), (int)data.size() - 1);

	auto min_norm = data[min_idx];
	auto max_norm = data[max_idx];

	if (modelEye) {
		min_norm = min(min_val, 0.0f);
		max_norm = max(max_val, 1.0f);
	}
	else {
		/*
		if (layers.isOuterRetinaFlows()) {
			if (max_val < 1.0f) {
				max_norm = max_norm * (1.0f / max_val);
				max_norm = min(max_norm, 1.0f);
			}
		}
		*/
		if ((max_norm - min_norm) < rangeDiff) {
			max_norm = min_norm + rangeDiff;
		}
	}

	std::for_each(projs.begin(), projs.end(), [&](float &elem) { elem = max(elem, min_norm); });
	std::for_each(projs.begin(), projs.end(), [&](float &elem) { elem = min(elem, max_norm); });

	LogD() << "Angio normalization (Decorr.), min: " << min_val << ", max: " << max_val << ", mean: " << mean << ", stdev: " << stdev << ", meye: " << modelEye;
	LogD() << "Decorr. Normalization range: " << rangeMin << " ~ " << rangeMax << ", value: " << min_norm << " ~ " << max_norm;

	if (max_norm > 0.0f) {
		if (max_norm > min_norm) {
			std::transform(fpos, epos, fpos, bind2nd(minus<float>(), min_norm));
			std::transform(fpos, epos, fpos, bind2nd(divides<float>(), (max_norm - min_norm)));
		}
	}
	return true;
}


bool OctAngio::AngioDecorr::denoiseProjectionProfiles(int lines, int points, float decorrMin, float decorrMax)
{
	{
		auto& projs = getImpl().decorrAngiogram;
		if (projs.size() <= 0) {
			return false;
		}

		auto fpos = projs.begin();
		auto epos = projs.end();
		auto max_v = *max_element(fpos, epos);
		auto min_v = *min_element(fpos, epos);

		float mean, stdev;
		CvImage image;
		image.fromFloat32((const unsigned char*)&projs[0], points, lines);
		image.getMeanStddev(&mean, &stdev);

		float valueMax = min(mean + stdev * 3.5f, max_v);
		float valueMin = mean * 0.5f;
		valueMax = min(max(valueMax, valueMin + 0.35f), 1.0f);

		if (decorrMin > 0.0f) {
			valueMin = decorrMin;
		}
		if (decorrMax > 0.0f) {
			valueMax = decorrMax;
		}

		LogD() << "Angio normalization (Decorr.), min: " << min_v << ", max: " << max_v << ", mean: " << mean << ", stdev: " << stdev;
		LogD() << "Decorr. Normalization range: " << valueMin << ", " << valueMax;

		std::for_each(fpos, epos, [&](float &elem) { elem = (elem < valueMin ? 0.0f : elem); });
		std::for_each(fpos, epos, [&](float &elem) { elem = (elem > valueMax ? 0.0f : elem); });
	}
	return true;
}

std::vector<CvImage>& OctAngio::AngioDecorr::decorrelations(void) const
{
	return getImpl().decorrelations;
}

std::vector<CvImage>& OctAngio::AngioDecorr::differentials(void) const
{
	return getImpl().differentials;
}


vector<float>& OctAngio::AngioDecorr::decorrAngiogram(void) const
{
	return getImpl().decorrAngiogram;
}

vector<float>& OctAngio::AngioDecorr::differAngiogram(void) const
{
	return getImpl().differAngiogram;
}

vector<float>& OctAngio::AngioDecorr::outputAngiogram(void) const
{
	return getImpl().outputAngiogram;
}

std::vector<float>& OctAngio::AngioDecorr::decorrProjectionMax(void) const
{
	return getImpl().decorrProjectionMax;
}

std::vector<float>& OctAngio::AngioDecorr::decorrProjectionMask(void) const
{
	return getImpl().decorrProjectionMask;
}

std::vector<float>& OctAngio::AngioDecorr::differProjectionMask(void) const
{
	return getImpl().differProjectionMask;
}

std::vector<float>& OctAngio::AngioDecorr::decorrAxialOffsets(void) const
{
	return getImpl().offsetProjection;
}


CppUtil::CvImage OctAngio::AngioDecorr::decorrAngiogramImage(bool axialMax) const
{
	int w = getImpl().projectWidth;
	int h = getImpl().projectHeight;

	auto& decors = (axialMax ? decorrProjectionMax() : decorrAngiogram());
	if (decors.empty() || decors.size() != w * h) {
		return CppUtil::CvImage();
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&decors[0], w, h);
	return image;
}


CppUtil::CvImage OctAngio::AngioDecorr::differAngiogramImage(void) const
{
	int w = getImpl().projectWidth;
	int h = getImpl().projectHeight;

	auto& differs = differAngiogram();
	if (differs.empty() || differs.size() != w * h) {
		return CppUtil::CvImage();
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&differs[0], w, h);
	return image;
}


float * OctAngio::AngioDecorr::getDecorrelationData(int index)
{
	if (getImpl().decorrelations.size() < index) {
		return nullptr;
	}

	auto& image = getImpl().decorrelations[index];
	if (image.getWidth() < 0 || image.getHeight() < 0) {
		return nullptr;
	}

	return (float*)image.getBitsData();
}


float * OctAngio::AngioDecorr::getDifferentialsData(int index)
{
	if (getImpl().differentials.size() < index) {
		return nullptr;
	}

	auto& image = getImpl().differentials[index];
	if (image.getWidth() < 0 || image.getHeight() < 0) {
		return nullptr;
	}

	return (float*)image.getBitsData();
}


CppUtil::CvImage OctAngio::AngioDecorr::getDecorrelationImage(int index)
{
	if (getImpl().decorrelations.size() < index) {
		return CppUtil::CvImage();
	}

	auto& image = getImpl().decorrelations[index];
	if (image.getWidth() < 0 || image.getHeight() < 0) {
		return CppUtil::CvImage();
	}

	return image;
}


CppUtil::CvImage OctAngio::AngioDecorr::getDifferentialsImage(int index)
{
	if (getImpl().differentials.size() < index) {
		return CppUtil::CvImage();
	}

	auto& image = getImpl().differentials[index];
	if (image.getWidth() < 0 || image.getHeight() < 0) {
		return CppUtil::CvImage();
	}

	return image;
}


float & OctAngio::AngioDecorr::backgroundThreshold(void)
{
	return getImpl().backgroundThreshold;
}


void OctAngio::AngioDecorr::getDecorrelationStat(float & mean, float & stdev, float & maxval)
{
	mean = getImpl().decorrMean;
	stdev = getImpl().decorrStdev;
	maxval = getImpl().decorrMaxVal;
	return;
}


void OctAngio::AngioDecorr::getIntensityStat(float & mean, float & stdev, float & maxval)
{
	mean = getImpl().intensMean;
	stdev = getImpl().intensStdev;
	maxval = getImpl().intensMaxVal;
	return;
}


void OctAngio::AngioDecorr::getDecorrelationStatOfBscan(int index, float & mean, float & stdev, float & maxval)
{
	if (index >= 0 && index < getImpl().bscanDecorrMeans.size()) {
		mean = getImpl().bscanDecorrMeans[index];
		stdev = getImpl().bscanDecorrStdevs[index];
		maxval = getImpl().bscanDecorrMaxVals[index];
	}
	else {
		mean = stdev = maxval = 0.0f;
	}
	return;
}


void OctAngio::AngioDecorr::getIntensityStatOfBscan(int index, float & mean, float & stdev, float & maxval)
{
	if (index >= 0 && index < getImpl().bscanIntensMeans.size()) {
		mean = getImpl().bscanIntensMeans[index];
		stdev = getImpl().bscanIntensStdevs[index];
		maxval = getImpl().bscanIntensMaxVals[index];
	}
	else {
		mean = stdev = maxval = 0.0f;
	}
	return;
}


AngioDecorr::AngioDecorrImpl & OctAngio::AngioDecorr::getImpl(void) const
{
	return *d_ptr;
}
