#include "stdafx.h"
#include "GlobalAnalysisDef.h"

using namespace OctGlobal;
using namespace std;


std::string OCTGLOBAL_DLL_API OctGlobal::LayerTypeToString(OcularLayerType type)
{
	if (type == OcularLayerType::ILM) {
		return "ILM";
	}
	if (type == OcularLayerType::NFL) {
		return "NFL";
	}
	if (type == OcularLayerType::IPL) {
		return "IPL";
	}
	if (type == OcularLayerType::OPL) {
		return "OPL";
	}
	if (type == OcularLayerType::IOS) {
		return "IOS";
	}
	if (type == OcularLayerType::RPE) {
		return "RPE";
	}
	if (type == OcularLayerType::OPR) {
		return "OPR";
	}
	if (type == OcularLayerType::BRM) {
		return "BRM";
	}
	if (type == OcularLayerType::BRM) {
		return "BRM";
	}
	if (type == OcularLayerType::EPI) {
		return "EPI";
	}
	if (type == OcularLayerType::BOW) {
		return "BOW";
	}
	if (type == OcularLayerType::END) {
		return "END";
	}
	return "Unknown";
};
