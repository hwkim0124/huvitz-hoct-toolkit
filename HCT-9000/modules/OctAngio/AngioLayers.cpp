#include "stdafx.h"
#include "AngioLayers.h"
#include "CppUtil2.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/format.hpp>
#include <numeric>
#include <json\json.hpp>
#include <fstream>
#include <iostream>
#include <map>

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;


struct AngioLayers::AngioLayersImpl
{
	OcularLayerType upperType = OcularLayerType::ILM;
	OcularLayerType lowerType = OcularLayerType::IPL;
	OcularLayerType upperType2 = OcularLayerType::NFL;
	OcularLayerType lowerType2 = OcularLayerType::IPL;

	vector<vector<int>> uppers;
	vector<vector<int>> lowers;
	vector<vector<int>> uppers2;
	vector<vector<int>> lowers2;

	OcularLayerType upperLayer = OcularLayerType::ILM;
	OcularLayerType lowerLayer = OcularLayerType::NFL;
	float upperOffset = 0.0f;
	float lowerOffset = 0.0f;

	int lines = 0;
	int repeats = 0;
	int points = 0;

	map<OcularLayerType, LayerArrays> layers;

	AngioLayersImpl() {

	};
};



AngioLayers::AngioLayers()
	: d_ptr(make_unique<AngioLayersImpl>())
{
}


AngioLayers::~AngioLayers()
{
	// Destructor should be defined for unique_ptr to delete AngioLayersImpl as an incomplete type.
}


OctAngio::AngioLayers::AngioLayers(AngioLayers && rhs) = default;
AngioLayers & OctAngio::AngioLayers::operator=(AngioLayers && rhs) = default;


void OctAngio::AngioLayers::setupAngioLayers(int lines, int points, int repeats)
{
	getImpl().lines = lines;
	getImpl().repeats = repeats;
	getImpl().points = points;

	for (int j = 0; j < NUMBER_OF_RETINA_LAYERS; j++) {
		auto k = static_cast<int>(OcularLayerType::ILM) + j;
		auto type = static_cast<OcularLayerType>(k);
		getImpl().layers[type] = LayerArrays(lines);
	}
	return;
}


void OctAngio::AngioLayers::setupSlabRange(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	getImpl().upperLayer = upper;
	getImpl().lowerLayer = lower;
	getImpl().upperOffset = upperOffset;
	getImpl().lowerOffset = lowerOffset;
	return;
}


bool OctAngio::AngioLayers::loadLayerSegmentFiles(const std::string dirPath)
{
	int lines = getImpl().lines;
	int repeats = getImpl().repeats;
	int points = getImpl().points;

	if (dirPath.empty()) {
		LogD() << "Angio layer dir path is empty!";
		return false;
	}

	LogD() << "Import layer segment files, path: " << dirPath << ", lines: " << lines << ", repeats: " << repeats << ", points: " << points;

	for (int i = 0; i < lines; i++) {
		auto line = vector<nlohmann::json>();
		std::string path;
		path = (boost::format("%s//%03d.json") % dirPath % i).str();

		try {
			std::ifstream file(path, std::ios::in);
			nlohmann::json json;
			file >> json;
			file.close();

			for (int j = 0; j < NUMBER_OF_RETINA_LAYERS; j++) {
				auto k = static_cast<int>(OcularLayerType::ILM) + j;
				auto type = static_cast<OcularLayerType>(k);
				/*
				if (type == OcularLayerType::OPR) {
					continue;
				}
				*/

				auto name = LayerTypeToString(type);
				auto layer = std::vector<int>(points, -1);

				try {
					layer = json["layers"][name].get<std::vector<int>>();
				}
				catch (...) {
					if (type == OcularLayerType::OPR) {
						auto name2 = LayerTypeToString(OcularLayerType::BRM);
						layer = json["layers"][name2].get<std::vector<int>>();
					}
				}

				if (layer.size() == points) {
					auto layers = getImpl().layers[type];
					getImpl().layers[type][i] = layer;
				}
				else {
					LogD() << "Angio layer segment size not matched, at line: " << i << ", size: " << layer.size();
					break;
				}
			}
		}
		catch (...) {
			LogD() << "Angio segment file reading error, path: " << path;
			// return false;
		}
	}

	testLayerSegments();
	return true;
}


void OctAngio::AngioLayers::testLayerSegments(void)
{
	auto& layer1 = getImpl().layers[OcularLayerType::ILM];
	auto& layer2 = getImpl().layers[OcularLayerType::RPE];

	if (!layer1.empty() && layer1[0].size() > 5) {
		auto& v = layer1[0];
		LogD() << "ILM: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
	}
	else {
		LogD() << "ILM: empty";
	}
	if (!layer2.empty() && layer2[0].size() > 5) {
		auto& v = layer2[0];
		LogD() << "RPE: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
	}
	else {
		LogD() << "RPE: empty";
	}
	return;
}


bool OctAngio::AngioLayers::importLayersFromSegmentFiles(OcularLayerType upper, OcularLayerType lower, 
								float upperOffset, float lowerOffset, 
								std::string dirPath, bool vascular)
{
	int lines = getImpl().lines;
	int repeats = getImpl().repeats;
	int points = getImpl().points;

	LogD() << "Angio layer segment files importing, path: " << dirPath << ", lines: " << lines << ", repeats: " << repeats << ", points: " << points;
	LogD() << "Upper layer: " << LayerTypeToString(upper) << ", offset: " << upperOffset;
	LogD() << "Lower layer: " << LayerTypeToString(lower) << ", offset: " << lowerOffset;

	if (dirPath.empty()) {
		LogD() << "Angio layer dir path is empty!";
		return false;
	}

	getImpl().uppers = vector<vector<int>>(lines);
	getImpl().lowers = vector<vector<int>>(lines);
	// getImpl().uppers2 = vector<vector<int>>(lines);
	// getImpl().lowers2 = vector<vector<int>>(lines);

	getImpl().upperType = upper;
	getImpl().lowerType = lower;

	int height = 768; // getImpl().imageHeight;

	for (int i = 0; i < lines; i++) {
		auto line = vector<nlohmann::json>();
		std::string path;
		path = (boost::format("%s//%03d.json") % dirPath % i).str();

		try {
			std::ifstream file(path, std::ios::in);
			nlohmann::json json;
			file >> json;
			file.close();

			// line.push_back(std::move(json));
			auto name1 = LayerTypeToString(upper);
			auto name2 = LayerTypeToString(lower);

			auto layer1 = json["layers"][name1].get<std::vector<int>>();
			auto layer2 = json["layers"][name2].get<std::vector<int>>();

			if (vascular) {
				if (name1 == "ILM" && name2 == "IPL") {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 0; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 5; });

					auto layerE = json["layers"]["IOS"].get<std::vector<int>>();
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (name1 == "NFL" && name2 == "IPL") {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 1; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 5; });

					auto layerE = json["layers"]["IOS"].get<std::vector<int>>();
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (name1 == "IPL" && name2 == "IPL") {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 5; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 23; });

					auto layerE = json["layers"]["IOS"].get<std::vector<int>>();
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (name1 == "IPL" && name2 == "OPL") {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 5; });
					transform(layer1.begin(), layer1.end(), layer2.begin(), [&](int x) { return x + 23; });

					auto layerE = json["layers"]["IOS"].get<std::vector<int>>();
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (name1 == "IPL" && name2 == "BRM") {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 23; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 0; });
				}
				else if (name1 == "BRM" && name2 == "BRM") {
					transform(layer1.begin(), layer1.end(), layer2.begin(), [&](int x) { return min(x + 5, height - 1); });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return min(x + 15, height - 1); });
				}
			}
			else {
				if (upperOffset != 0.0f) {
					int offset = (int)(upperOffset / GlobalSettings::getRetinaScanAxialResolution());
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return min(max(x + offset, 0), height - 1); });
				}
				if (lowerOffset != 0.0f) {
					int offset = (int)(lowerOffset / GlobalSettings::getRetinaScanAxialResolution());
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return min(max(x + offset, 0), height - 1); });
				}
				if (upperOffset != 0.0f || lowerOffset != 0.0f) {
					transform(begin(layer2), end(layer2), begin(layer1), begin(layer2), [](int e1, int e2) { return max(e1, e2); });
				}
			}

			int index = i; // i * repeats + j;
			if (layer1.size() == points && layer2.size() == points) {
				getImpl().uppers[index] = layer1;
				getImpl().lowers[index] = layer2;

				/*
				{
					auto layer1 = json["layers"]["NFL"].get<std::vector<int>>();
					auto layer2 = json["layers"]["IPL"].get<std::vector<int>>();

					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 0; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 5; });

					auto layerE = json["layers"]["OPL"].get<std::vector<int>>();
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });

					getImpl().uppers2[i] = layer1;
					getImpl().lowers2[i] = layer2;
				}
				*/
			}
			else {
				// getImpl().uppers[index] = std::vector<int>(points, 0);
				// getImpl().lowers[index] = std::vector<int>(points, height - 1);
				LogD() << "Angio layer segment size not matched, at line: " << i << ", size: " << layer1.size();
			}
		}
		catch (...) {
			LogD() << "Angio segment file reading error, path: " << path;
			// return false;
		}
	}

	if (getImpl().uppers[0].size() > 5) {
		auto& v = getImpl().uppers[0];
		LogD() << "Uppers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
	}
	else {
		LogD() << "Uppers: empty";
	}
	if (getImpl().lowers[0].size() > 5) {
		auto& v = getImpl().lowers[0];
		LogD() << "Lowers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
	}
	else {
		LogD() << "Lowers: empty";
	}
	return true;
}


bool OctAngio::AngioLayers::isSuperficialFlows(void) const
{
	// return (getImpl().upperLayer == OcularLayerType::ILM && getImpl().lowerLayer == OcularLayerType::IPL
	return (getImpl().lowerLayer == OcularLayerType::IPL);
}

bool OctAngio::AngioLayers::isDeepFlows(void) const
{
	// return (getImpl().upperLayer == OcularLayerType::IPL && getImpl().lowerLayer == OcularLayerType::OPL);
	return (getImpl().lowerLayer == OcularLayerType::OPL);
}

bool OctAngio::AngioLayers::isOuterRetinaFlows(void) const
{
	// return (getImpl().upperType == OcularLayerType::OPL && getImpl().lowerType == OcularLayerType::IOS);

	/*
	bool flag1 = false;
	bool flag2 = false;
	auto type = getImpl().upperLayer;
	if (type == OcularLayerType::IOS || type == OcularLayerType::RPE ||
		type == OcularLayerType::OPR || type == OcularLayerType::BRM || 
		type == OcularLayerType::OPL || type == OcularLayerType::IPL) {
		flag1 = true;
	}
	
	type = getImpl().lowerLayer;
	if (type == OcularLayerType::RPE || type == OcularLayerType::OPR || type == OcularLayerType::BRM) {
		flag2 = true;
	}
	if (type == OcularLayerType::IOS) {
		flag2 = true;
	}

	if (flag1 && flag2) {
		return true;
	}
	*/

	bool is_upper = false;
	bool is_lower = false;

	auto type1 = getImpl().upperLayer;
	if (type1 == OcularLayerType::IOS || type1 == OcularLayerType::RPE || type1 == OcularLayerType::OPL ||
		type1 == OcularLayerType::OPR || type1 == OcularLayerType::BRM) {
		is_upper = true;
	}

	auto type2 = getImpl().lowerLayer;
	if (type2 == OcularLayerType::RPE || type2 == OcularLayerType::IOS || 
		type2 == OcularLayerType::OPR || type2 == OcularLayerType::BRM) {
		is_lower = true;
	}

	bool flag = (is_upper && is_lower);
	return flag;
}

bool OctAngio::AngioLayers::isChoroidalFlows(void) const
{
	return (getImpl().upperLayer == OcularLayerType::BRM && getImpl().lowerLayer == OcularLayerType::BRM);
}


void OctAngio::AngioLayers::setLayerPoints(int line, OcularLayerType type, std::vector<int> points)
{
	auto it = getImpl().layers.find(type);
	if (it != end(getImpl().layers)) {
		if (it->second.size() > line) {
			it->second[line] = points;
		}
	}
	return;
}


LayerArrays OctAngio::AngioLayers::getLayerArrays(OcularLayerType type, float offset) const
{
	auto arrays = LayerArrays(getImpl().lines);
	int noffset = (int)(offset / GlobalSettings::getRetinaScanAxialResolution());
	int height = 768;

	auto it = getImpl().layers.find(type);
	if (it != end(getImpl().layers)) {
		int line = 0;
		for (auto e : it->second) {
			auto v = std::vector<int>(getImpl().points);
			transform(cbegin(e), cend(e), begin(v), [=](int x) { return min(max(x + noffset, 0), height-1); });
			arrays[line++] = v;
		}
	}
	return arrays;
}


LayerArrays OctAngio::AngioLayers::getUpperLayers(OcularLayerType type, float offset) const
{
	if (getImpl().lines <= 0 || getImpl().points <= 0) {
		return LayerArrays();
	}

	auto it = getImpl().layers.find(type);
	if (it != end(getImpl().layers) && it->second.size() == getImpl().lines) {
		return getLayerArrays(type, offset);
	}
	else {
		auto arrays = LayerArrays(getImpl().lines);
		for (auto& e : arrays) {
			e = std::vector<int>(getImpl().points, 0);
		}
		return arrays;
	}
}


LayerArrays OctAngio::AngioLayers::getLowerLayers(OcularLayerType type, float offset) const
{
	if (getImpl().lines <= 0 || getImpl().points <= 0) {
		return LayerArrays();
	}

	auto it = getImpl().layers.find(type);
	if (it != end(getImpl().layers) && it->second.size() == getImpl().lines) {
		return getLayerArrays(type, offset);
	}
	else {
		auto arrays = LayerArrays(getImpl().lines);
		int height = 768;
		for (auto& e : arrays) {
			e = std::vector<int>(getImpl().points, height-1);
		}
		return arrays;
	}
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfSlab(void) const
{
	return getUpperLayers(getImpl().upperLayer, getImpl().upperOffset);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfSlab(void) const
{
	return getLowerLayers(getImpl().lowerLayer, getImpl().lowerOffset);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfProjectMask(void) const
{
	return getUpperLayers(SHADOWED_UPPER_LAYER_TYPE, SHADOWED_UPPER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfProjectMask(void) const
{
	return getLowerLayers(SHADOWED_LOWER_LAYER_TYPE, SHADOWED_LOWER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfShadowed(void) const
{
	return getUpperLayers(SHADOWED_UPPER_LAYER_TYPE, SHADOWED_UPPER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfShadowed(void) const
{
	return getLowerLayers(SHADOWED_LOWER_LAYER_TYPE, SHADOWED_LOWER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfShadowed2(void) const
{
	return getUpperLayers(SHADOWED_UPPER_LAYER_TYPE2, SHADOWED_UPPER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfShadowed2(void) const
{
	return getLowerLayers(SHADOWED_LOWER_LAYER_TYPE2, SHADOWED_LOWER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfVasculature(void) const
{
	return getUpperLayers(VASCULAR_UPPER_LAYER_TYPE, VASCULAR_UPPER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfVasculature(void) const
{
	return getLowerLayers(VASCULAR_LOWER_LAYER_TYPE, VASCULAR_LOWER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfVasculature2(void) const
{
	return getUpperLayers(VASCULAR_UPPER_LAYER_TYPE2, VASCULAR_UPPER_LAYER_OFFSET2);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfVasculature2(void) const
{
	return getLowerLayers(VASCULAR_LOWER_LAYER_TYPE2, VASCULAR_LOWER_LAYER_OFFSET2);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfSuperficial(void) const
{
	return getUpperLayers(SUPERFICIAL_UPPER_LAYER_TYPE, SUPERFICIAL_UPPER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfSuperficial(void) const
{
	return getLowerLayers(SUPERFICIAL_LOWER_LAYER_TYPE, SUPERFICIAL_LOWER_LAYER_OFFSET);
}

LayerArrays OctAngio::AngioLayers::getUpperLayersOfNonVascular(void) const
{
	auto inners = getUpperLayers(OcularLayerType::OPL, 0.0f);
	auto outers = getUpperLayers(NONVASCULAR_UPPER_LAYER_TYPE, NONVASCULAR_UPPER_LAYER_OFFSET);

	for (auto i = 0; i < inners.size(); i++) {
		for (auto j = 0; j < inners[i].size(); j++) {
			outers[i][j] = max(inners[i][j], outers[i][j]);
		}
	}

	return outers;
}

LayerArrays OctAngio::AngioLayers::getLowerLayersOfNonVascular(void) const
{
	auto inners = getLowerLayers(NONVASCULAR_LOWER_LAYER_TYPE, NONVASCULAR_LOWER_LAYER_OFFSET);
	auto outers = getLowerLayers(OcularLayerType::BRM, 0.0f);

	for (auto i = 0; i < inners.size(); i++) {
		for (auto j = 0; j < inners[i].size(); j++) {
			inners[i][j] = min(inners[i][j], outers[i][j]);
		}
	}
	return inners;
}


void OctAngio::AngioLayers::setUpperLayers(const std::vector<std::vector<int>>& layers)
{
	getImpl().uppers = layers;
}

void OctAngio::AngioLayers::setLowerLayers(const std::vector<std::vector<int>>& layers)
{
	getImpl().lowers = layers;
}

const std::vector<std::vector<int>>& OctAngio::AngioLayers::upperLayers(void) const
{
	return getImpl().uppers;
}

const std::vector<std::vector<int>>& OctAngio::AngioLayers::lowerLayers(void) const
{
	return getImpl().lowers;
}

LayerArrays OctAngio::AngioLayers::getUpperLayers(void) const
{
	int lines = getImpl().lines;
	int points = getImpl().points;

	auto layers = getImpl().uppers;
	if (layers.size() != lines) {
		layers = LayerArrays(lines);
	}

	int height = 768;
	for (auto& l : layers) {
		if (l.size() != points || any_of(l.begin(), l.end(), [=](int e) { return e < 0 || e > height; } )) {
			l = LayerPoints(points, 0);
		}
	}
	return layers;
}

LayerArrays OctAngio::AngioLayers::getLowerLayers(void) const
{
	int lines = getImpl().lines;
	int points = getImpl().points;

	auto layers = getImpl().lowers;
	if (layers.size() != lines) {
		layers = LayerArrays(lines);
	}

	int height = 768;
	for (auto& l : layers) {
		if (l.size() != points || any_of(l.begin(), l.end(), [=](int e) { return e < 0 || e > height; })) {
			l = LayerPoints(points, height-1);
		}
	}
	return layers;
}

OcularLayerType & OctAngio::AngioLayers::upperLayerType(void)
{
	return getImpl().upperLayer;
}

OcularLayerType & OctAngio::AngioLayers::lowerLayerType(void)
{
	return getImpl().lowerLayer;
}


AngioLayers::AngioLayersImpl & OctAngio::AngioLayers::getImpl(void) const
{
	return *d_ptr;
}
