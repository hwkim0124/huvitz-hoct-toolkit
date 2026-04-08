#include "stdafx.h"
#include "ConfigFile.h"
#include "tinyxml2.h"

#include "SystemProfile.h"
#include "DeviceSettings.h"
#include "SignalSettings.h"
#include "CameraSettings.h"
#include "FundusSettings.h"
#include "CorneaSettings.h"
#include "RetinaSettings.h"
#include "FixationSettings.h"
#include "PatternSettings.h"
#include "SystemSettings.h"
#include "AngioSettings.h"

#include <vector>

using namespace OctConfig;
using namespace std;
using namespace tinyxml2;


struct ConfigFile::ConfigFileImpl
{
	tinyxml2::XMLDocument* pDoc;
	tinyxml2::XMLNode* pRoot;
	tinyxml2::XMLNode* pNode;

	ConfigFileImpl() {
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ConfigFile::ConfigFileImpl> ConfigFile::d_ptr(new ConfigFileImpl());


ConfigFile::ConfigFile()
{
}


ConfigFile::~ConfigFile()
{
}


bool OctConfig::ConfigFile::loadSystemProfile(const char * filename)
{
	tinyxml2::XMLDocument doc;
	if (!checkXMLResult(doc.LoadFile(filename))) {
		return false;
	}

	XMLNode* pRoot = doc.FirstChildElement("HCT");
	if (pRoot == nullptr) {
		return false;
	}

	getImpl().pDoc = &doc;
	getImpl().pRoot = pRoot;
	getImpl().pNode = pRoot;

	if (!loadDeviceSettings(SystemProfile::getDeviceSettings())) {
		//return false;
	}
	/*
	if (!loadSignalSettings(SystemProfile::getSignalSettings())) {
		return false;
	}
	*/
	if (!loadCameraSettings(SystemProfile::getCameraSettings())) {
		//return false;
	}
	if (!loadFundusSettings(SystemProfile::getFundusSettings())) {
		//return false;
	}
	if (!loadRetinaSettings(SystemProfile::getRetinaSettings())) {
		//return false;
	}
	if (!loadCorneaSettings(SystemProfile::getCorneaSettings())) {
		//return false;
	}
	if (!loadFixationSettings(SystemProfile::getFixationSettings())) {
		//return false;
	}
	if (!loadPatternSettings(SystemProfile::getPatternSettings())) {
		//return false;
	}
	if (!loadSystemSettings(SystemProfile::getSystemSettings())) {
		//return false;
	}
	if (!loadAngioSettings(SystemProfile::getAngioSettings())) {
		//return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveSystemProfile(const char* filename)
{
	tinyxml2::XMLDocument doc;

	XMLDeclaration* decl = doc.NewDeclaration();
	doc.LinkEndChild(decl);

	XMLElement* root = doc.NewElement("HCT");
	doc.LinkEndChild(root);

	XMLComment* comment = doc.NewComment("System Configuration for Huvitz OCT");
	root->LinkEndChild(comment);

	getImpl().pDoc = &doc;
	getImpl().pRoot = root;
	getImpl().pNode = root;

	saveDeviceSettings(SystemProfile::getDeviceSettings());
	/*
	saveSignalSettings(SystemProfile::getSignalSettings());
	*/
	saveCameraSettings(SystemProfile::getCameraSettings());
	saveFundusSettings(SystemProfile::getFundusSettings());
	saveRetinaSettings(SystemProfile::getRetinaSettings());
	saveCorneaSettings(SystemProfile::getCorneaSettings());
	saveFixationSettings(SystemProfile::getFixationSettings());
	savePatternSettings(SystemProfile::getPatternSettings());
	saveSystemSettings(SystemProfile::getSystemSettings());
	saveAngioSettings(SystemProfile::getAngioSettings());

	return checkXMLResult(doc.SaveFile(filename));
}


ConfigFile::ConfigFileImpl& OctConfig::ConfigFile::getImpl(void)
{
	return *d_ptr;
}


bool OctConfig::ConfigFile::checkXMLResult(int result)
{
	return (result == XML_SUCCESS);
}


bool OctConfig::ConfigFile::loadDeviceSettings(DeviceSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("DeviceSettings");
	if (group == nullptr) return false;

	if (!loadSectionOfLightLeds(pset, group)) {
		return false;
	}

	/*
	if (!loadSectionOfStepMotors(pset, group)) {
		return false;
	}
	*/
	return true;
}


bool OctConfig::ConfigFile::loadCameraSettings(CameraSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("CameraSettings");
	if (group == nullptr) return false;

	if (!loadSectionOfCameras(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadFundusSettings(FundusSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("FundusColor");
	if (group == nullptr) return false;

	if (!loadSectionOfFundus(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadCorneaSettings(CorneaSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("CorneaImage");
	if (group == nullptr) return false;

	if (!loadSectionOfCornea(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadRetinaSettings(RetinaSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("RetinaImage");
	if (group == nullptr) return false;

	if (!loadSectionOfRetina(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadFixationSettings(FixationSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("InternalFixation");
	if (group == nullptr) return false;

	if (!loadSectionOfFixation(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadSystemSettings(SystemSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("SystemSettings");
	if (group == nullptr) return false;

	if (!loadSectionOfSystemOptions(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadPatternSettings(PatternSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("PatternSettings");
	if (group == nullptr) return false;

	if (!loadSectionOfPatternRange(pset, group)) {
		return false;
	}
	return true;
}

bool OctConfig::ConfigFile::loadAngioSettings(AngioSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = root->FirstChildElement("AngioSettings");
	if (group == nullptr) return false;

	if (!loadSectionOfAngioParams(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::loadSignalSettings(SignalSettings * pset)
{
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLNode* child;
	XMLElement* elem;
	int index = 0;
	double value = 0.0;

	XMLElement* group = root->FirstChildElement("SignalSettings");
	if (group != nullptr) {

		XMLElement* items = group->FirstChildElement("WavelengthFunction");
		if (items != nullptr) {
			for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryDoubleAttribute("value", &value))) {
						pset->setWavelengthParameter(index++, value);
					}
				}
			}
		}

		items = group->FirstChildElement("DispersionCompensation");
		if (items != nullptr) {
			index = 0;
			float param = 0.0f;
			for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryFloatAttribute("value", &param))) {
						pset->setDispersionParameter(index++, param);
					}
				}
			}
		}
	}

	return true;
}


bool OctConfig::ConfigFile::loadSectionOfLightLeds(DeviceSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	int value = 0;

	// Light LEDs.
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* lights = group->FirstChildElement("LightLEDs");
	if (lights != nullptr)
	{
		for (child = lights->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr) {
					LightLedItem* item = pset->getLightLedItem(name);
					if (item != nullptr) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							item->setValue(value);
						}
					}
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfCameras(CameraSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	float fvalue = 0.0f;
	int ivalue = 0;

	// Ir Cameras
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* list = group->FirstChildElement("Cameras");
	if (list != nullptr)
	{
		for (child = list->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr) {
					CameraItem* item = pset->getCameraItem(name);
					if (item != nullptr) {
						if (checkXMLResult(elem->QueryFloatAttribute("again", &fvalue))) {
							item->setAnalogGain(fvalue);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("dgain", &fvalue))) {
							item->setDigitalGain(fvalue);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("again2", &fvalue))) {
							item->setAnalogGain(fvalue, 1);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("dgain2", &fvalue))) {
							item->setDigitalGain(fvalue, 1);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("again3", &fvalue))) {
							item->setAnalogGain(fvalue, 2);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("dgain3", &fvalue))) {
							item->setDigitalGain(fvalue, 2);
						}
						/*
						if (checkXMLResult(elem->QueryIntAttribute("shiftX", &ivalue))) {
							item->shiftX() = ivalue;
						}
						if (checkXMLResult(elem->QueryIntAttribute("shiftY", &ivalue))) {
							item->shiftY() = ivalue;
						}
						*/
					}
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfFundus(FundusSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	double value = 0.0;
	bool flag = false;
	int coord = 0;
	int level = 0;
	double factor = 0.0f;
	float fvalue = 0.0f;
	int ivalue = 0;

	vector<double> params;
	vector<int> coordXs;
	vector<int> coordYs;
	vector<int> coordXs_FILR;
	vector<int> coordYs_FILR;
	pair<int, int> center = std::pair<int, int>(0, 0);
	int radius = 0;

	XMLElement* items = group->FirstChildElement("WhiteBalance");
	if (items != nullptr) {
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child != items->LastChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryDoubleAttribute("value", &value))) {
						params.push_back(value);
					}
				}
			}
			else {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
						pset->useWhiteBalance(true, flag);
						pset->setWhiteBalanceParameters(params);
					}
				}
			}
		}
	}

	items = group->FirstChildElement("RadialCorrection");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child != items->LastChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
						coordXs.push_back(coord);
					}
					if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
						coordYs.push_back(coord);
					}
				}
			}
			else {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
						pset->useColorCorrection(true, flag);
						pset->setCorrectionCoordsX(coordXs);
						pset->setCorrectionCoordsY(coordYs);
					}
				}
			}
		}
	}

	items = group->FirstChildElement("RadialCorrection_FILR");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child != items->LastChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
						coordXs_FILR.push_back(coord);
					}
					if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
						coordYs_FILR.push_back(coord);
					}
				}
			}
			else {
				pset->setCorrectionCoordsX_FILR(coordXs_FILR);
				pset->setCorrectionCoordsY_FILR(coordYs_FILR);
			}
		}
	}

	items = group->FirstChildElement("LevelCorrection");
	if (items != nullptr)
	{
		child = items->FirstChild();
		if (child != nullptr && (elem = child->ToElement()) != nullptr) {
			if (checkXMLResult(elem->QueryFloatAttribute("value", &fvalue))) {
				pset->setRadialCorrectionRatio(fvalue);
			}

			child = child->NextSibling();
			if (child != nullptr && (elem = child->ToElement()) != nullptr) {
				if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
					pset->useRadialCorrection(true, flag);
				}
			}
		}
	}

	items = group->FirstChildElement("RemoveReflectionLight");
	if (items != nullptr)
	{
		child = items->FirstChild();
		if (child != nullptr && (elem = child->ToElement()) != nullptr) {
			if (checkXMLResult(elem->QueryIntAttribute("value", &ivalue))) {
				pset->setRemoveReflection1(ivalue);
			}

			child = child->NextSibling();
			if (child != nullptr && (elem = child->ToElement()) != nullptr) {
				if (checkXMLResult(elem->QueryIntAttribute("value", &ivalue))) {
					pset->setRemoveReflection2(ivalue);
				}

				child = child->NextSibling();
				if (child != nullptr && (elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("value", &ivalue))) {
						pset->setRemoveReflection3(ivalue);
					}

					child = child->NextSibling();
					if (child != nullptr && (elem = child->ToElement()) != nullptr) {
						if (checkXMLResult(elem->QueryBoolAttribute("value", &flag))) {
							pset->useRemoveReflectionLight(true, flag);
						}
					}
				}
			}
		}
	}

	items = group->FirstChildElement("AutoEnhancement");
	if (items != nullptr)
	{
		child = items->FirstChild();
		if (child != nullptr && (elem = child->ToElement()) != nullptr) {
			if (checkXMLResult(elem->QueryFloatAttribute("value", &fvalue))) {
				pset->setAutoBright(fvalue);
			}

			child = child->NextSibling();
			if (child != nullptr && (elem = child->ToElement()) != nullptr) {
				if (checkXMLResult(elem->QueryFloatAttribute("value", &fvalue))) {
					pset->setAutoContrast(fvalue);
				}

				child = child->NextSibling();
				if (child != nullptr && (elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryFloatAttribute("value", &fvalue))) {
						pset->setAutoUB(fvalue);
					}

					child = child->NextSibling();
					if (child != nullptr && (elem = child->ToElement()) != nullptr) {
						if (checkXMLResult(elem->QueryFloatAttribute("value", &fvalue))) {
							pset->setAutoVR(fvalue);
						}
					}
				}
			}
		}
	}

	items = group->FirstChildElement("EdgeEnhancement");
	if (items != nullptr)
	{
		child = items->FirstChild();
		if (child != nullptr && (elem = child->ToElement()) != nullptr) {
			if (checkXMLResult(elem->QueryIntAttribute("value", &level))) {
				pset->setEdgeKernelSize(level);
			}
		}
	}

	items = group->FirstChildElement("ColorROI");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child != items->LastChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
						center.first = coord;
					}
					if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
						center.second = coord;
					}
				}

				child = child->NextSibling();
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("r", &coord))) {
						radius = coord;
					}
				}
			}
			else {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
						pset->useROICoordinates(true, flag);
						pset->setCenterPointOfROI(center);
						pset->setRadiusOfROI(radius);
					}
				}
			}
		}
	}

	items = group->FirstChildElement("AutoFlash");
	if (items != nullptr)
	{
		child = items->FirstChild();
		if (child != nullptr && (elem = child->ToElement()) != nullptr) {
			if (checkXMLResult(elem->QueryIntAttribute("normal", &level))) {
				pset->setAutoFlashLevelNormal(level);
			}
			if (checkXMLResult(elem->QueryIntAttribute("min", &level))) {
				pset->setAutoFlashLevelMin(level);
			}
			if (checkXMLResult(elem->QueryIntAttribute("max", &level))) {
				pset->setAutoFlashLevelMax(level);
			}

			while (true) {
				child = child->NextSibling();
				if (child != nullptr && (elem = child->ToElement()) != nullptr) {
					const char* name = elem->Attribute("name");
					if (name != nullptr) {
						if (!strcmp(name, "sizeBase")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashSizeBase(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashSizeBase(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashSizeBase(ivalue, 2);
							}
						}
						else if (!strcmp(name, "sizeHigh")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashSizeHigh(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashSizeHigh(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashSizeHigh(ivalue, 2);
							}
						}
						else if (!strcmp(name, "focusInt")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashFocusInt(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashFocusInt(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashFocusInt(ivalue, 2);
							}
						}
						else if (!strcmp(name, "defLevel")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashDefLevel(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashDefLevel(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashDefLevel(ivalue, 2);
							}
						}
						else if (!strcmp(name, "splitFous")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashSplitFocus(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashSplitFocus(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashSplitFocus(ivalue, 2);
							}
						}
						else if (!strcmp(name, "retinaIr")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashRetinaIr(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashRetinaIr(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashRetinaIr(ivalue, 2);
							}
						}
						else if (!strcmp(name, "workDot1")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashWorkDot1(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashWorkDot1(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashWorkDot1(ivalue, 2);
							}
						}
						else if (!strcmp(name, "workDot2")) {
							if (checkXMLResult(elem->QueryIntAttribute("value1", &ivalue))) {
								pset->setAutoFlashWorkDot2(ivalue, 0);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value2", &ivalue))) {
								pset->setAutoFlashWorkDot2(ivalue, 1);
							}

							if (checkXMLResult(elem->QueryIntAttribute("value3", &ivalue))) {
								pset->setAutoFlashWorkDot2(ivalue, 2);
							}
						}
					}
					else {
						if (!strcmp(elem->Name(), "status")) {
							if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
								pset->useAutoFlashLevel(true, flag);
							}
						}
					}
				}
				else {
					break;
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfRetina(RetinaSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	bool flag = false;
	int coord = 0;

	std::pair<int, int> center = std::pair<int, int>(0, 0);
	std::pair<int, int> kernel = std::pair<int, int>(0, 0);
	int radius = 0;
	int value = 0;
	float fvalue = 0.0f;

	XMLElement* items = group->FirstChildElement("ImageEnhance");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr) {
					if (!strcmp(elem->Name(), "status")) {
						if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
							pset->useImageEnhance(true, flag);
						}
					}
					else if (!strcmp(elem->Name(), "contrast")) {
						if (checkXMLResult(elem->QueryFloatAttribute("clipLimit", &fvalue))) {
							pset->setEnhanceClipLimit(fvalue);
						}
						if (checkXMLResult(elem->QueryIntAttribute("clipScalar", &value))) {
							pset->setEnhanceClipScalar(value);
						}
					}
				}
			}
		}
	}

	items = group->FirstChildElement("SplitFocus");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child == items->FirstChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
						center.first = coord;
					}
					if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
						center.second = coord;
					}
				}
			}
			/*
			else {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("width", &coord))) {
						kernel.first = coord;
					}
					if (checkXMLResult(elem->QueryIntAttribute("height", &coord))) {
						kernel.second = coord;
					}
				}
			}
			*/
		}

		pset->setSplitCenter(center);
		// pset->setSplitKernel(kernel.first, kernel.second);
	}

    items = group->FirstChildElement("RetinaROI");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if (child != items->LastChild()) {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
						center.first = coord;
					}
					if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
						center.second = coord;
					}
				}

				child = child->NextSibling();
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("r", &coord))) {
						radius = coord;
					}
				}
			}
			else {
				if ((elem = child->ToElement()) != nullptr) {
					if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
						pset->useROI(true, flag);
					}
				}
			}
		}

		pset->setCenterPointOfROI(center);
		pset->setRadiusOfROI(radius);
	}

	items = group->FirstChildElement("CenterMask");
	if (items != nullptr)
	{
		child = items->FirstChild();

		do {
			if (child != nullptr && ((elem = child->ToElement()) != nullptr)) {
				const char* name = elem->Attribute("name");
				if (name != nullptr) {
					if (!strcmp(name, "radius")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setCenterMaskSize(value);
						}
					}
					else if (!strcmp(name, "center")) {
						if (checkXMLResult(elem->QueryIntAttribute("x", &coord))) {
							center.first = coord;
						}
						if (checkXMLResult(elem->QueryIntAttribute("y", &coord))) {
							center.second = coord;
						}
						pset->setCenterPointOfMask(center);
					}
				}
				else if (!strcmp(elem->Name(), "status")) {
					if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
						pset->useCenterMask(true, flag);
					}
				}
			}
			else {
				break;
			}
		} while ((child = child->NextSibling()) != nullptr);
	}
    return true;
}


bool OctConfig::ConfigFile::loadSectionOfCornea(CorneaSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	int pixels;
	float size;

	XMLElement* items = group->FirstChildElement("SmallPupil");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr)
				{
					if (!strcmp(name, "pixelsPerMM")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &pixels))) {
							pset->setPixelsPerMM(pixels);
						}
					}
					if (!strcmp(name, "smallPupilSize")) {
						if (checkXMLResult(elem->QueryFloatAttribute("value", &size))) {
							pset->setSmallPupilSize(size);
						}
					}
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfFixation(FixationSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	bool flag = false;
	int row, col;
	int bright;
	int period = 0, onTime = 0, fixType = 0;

	XMLElement* items = group->FirstChildElement("LcdFixation");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				if (checkXMLResult(elem->QueryIntAttribute("row", &row)) &&
					checkXMLResult(elem->QueryIntAttribute("col", &col))) {
					const char* name = elem->Attribute("name");
					auto pos = std::pair<int, int>(row, col);
					if (!strcmp(name, "centerOD")) {
						pset->setCenterOD(pos);
					}
					else if (!strcmp(name, "fundusOD")) {
						pset->setFundusOD(pos);
					}
					else if (!strcmp(name, "scanDiskOD")) {
						pset->setScanDiskOD(pos);
					}
					else if (!strcmp(name, "leftSideOD")) {
						pset->setLeftSideOD(0, pos);
					}
					else if (!strcmp(name, "leftUpOD")) {
						pset->setLeftSideOD(1, pos);
					}
					else if (!strcmp(name, "leftDownOD")) {
						pset->setLeftSideOD(2, pos);
					}
					else if (!strcmp(name, "rightSideOD")) {
						pset->setRightSideOD(0, pos);
					}
					else if (!strcmp(name, "rightUpOD")) {
						pset->setRightSideOD(1, pos);
					}
					else if (!strcmp(name, "rightDownOD")) {
						pset->setRightSideOD(2, pos);
					}
					else if (!strcmp(name, "centerOS")) {
						pset->setCenterOS(pos);
					}
					else if (!strcmp(name, "fundusOS")) {
						pset->setFundusOS(pos);
					}
					else if (!strcmp(name, "scanDiskOS")) {
						pset->setScanDiskOS(pos);
					}
					else if (!strcmp(name, "leftSideOS")) {
						pset->setLeftSideOS(0, pos);
					}
					else if (!strcmp(name, "leftUpOS")) {
						pset->setLeftSideOS(1, pos);
					}
					else if (!strcmp(name, "leftDownOS")) {
						pset->setLeftSideOS(2, pos);
					}
					else if (!strcmp(name, "rightSideOS")) {
						pset->setRightSideOS(0, pos);
					}
					else if (!strcmp(name, "rightUpOS")) {
						pset->setRightSideOS(1, pos);
					}
					else if (!strcmp(name, "rightDownOS")) {
						pset->setRightSideOS(2, pos);
					}
				}
				else
				{
					const char* name = elem->Attribute("name");
					if (name != nullptr)
					{
						if (!strcmp(name, "brightness")) {
							if (checkXMLResult(elem->QueryIntAttribute("value", &bright))) {
								pset->setBrightness(bright);
							}
						}
						else if (!strcmp(name, "blink")) {
							if (checkXMLResult(elem->QueryBoolAttribute("value", &flag))) {
								pset->useLcdBlinkOn(true, flag);
							}
							if (checkXMLResult(elem->QueryIntAttribute("period", &period))) {
								pset->setBlinkPeriod(period);
							}
							if (checkXMLResult(elem->QueryIntAttribute("onTime", &onTime))) {
								pset->setBlinkOnTime(onTime);
							}
							if (checkXMLResult(elem->QueryIntAttribute("type", &fixType))) {
								pset->setFixationType(fixType);
							}
						}
					}
					else {
						if (checkXMLResult(elem->QueryBoolAttribute("valid", &flag))) {
							pset->useLcdFixation(true, flag);
						}
					}
				}
			}
		}
	}

	return true;
}


bool OctConfig::ConfigFile::loadSectionOfSystemOptions(SystemSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	bool flag = false;

	XMLElement* items = group->FirstChildElement("SystemInfo");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				int value;
				if (name != nullptr)
				{
					if (!strcmp(name, "SerialNumber")) {
						pset->setSerialNumber(elem->Attribute("value"));
					}
					if (!strcmp(name, "ModelType")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setModelType(value);
						}
					}
					/*if (!strcmp(name, "OctSoloVersion")) {
						if (checkXMLResult(elem->QueryBoolAttribute("value", &flag))) {
							pset->useOctSoloVersion(true, flag);
						}
					}*/
					if (!strcmp(name, "OctGrabberType")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setOctGrabberType(value);
						}
					}
					if (!strcmp(name, "OctLineCameraMode")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setOctLineCameraMode(value);
						}
					}
					if (!strcmp(name, "SldFaultDetection")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setSldFaultDetection(value);
						}
					}
					if (!strcmp(name, "RetinaTrackingSpeed")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->setRetinaTrackingSpeed(value);
						}
					}
					if (!strcmp(name, "TriggerForePadding")) {
						if (checkXMLResult(elem->QueryIntAttribute("fore1", &value))) {
							pset->setTriggerForePadd(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("fore2", &value))) {
							pset->setTriggerForePadd(1, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("fore3", &value))) {
							pset->setTriggerForePadd(2, value);
						}
					}
					if (!strcmp(name, "TriggerPostPadding")) {
						if (checkXMLResult(elem->QueryIntAttribute("post1", &value))) {
							pset->setTriggerPostPadd(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("post2", &value))) {
							pset->setTriggerPostPadd(1, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("post3", &value))) {
							pset->setTriggerPostPadd(2, value);
						}
					}
				}
			}
		}
	}

	/*
	XMLElement* items = group->FirstChildElement("SystemOptions");
	if (items != nullptr)
	{
		int value;
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr)
				{
					if (!strcmp(name, "checkMainboardAtStartup")) {
						if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
							pset->checkMainboardAtStartup(true, value);
						}
					}
				}
			}
		}
	}
	*/
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfPatternRange(PatternSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;

	XMLElement* items = group->FirstChildElement("PatternRange");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr)
				{
					float scaleX = 1.0f, scaleY = 1.0f, offsetX = 0.0f, offsetY = 0.0f;

					if (!strcmp(name, "retinaNormal")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setRetinaPatternScale(std::pair<float, float>(scaleX, scaleY), 0);
						pset->setRetinaPatternOffset(std::pair<float, float>(offsetX, offsetY), 0);
					}
					else if (!strcmp(name, "retinaFast")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setRetinaPatternScale(std::pair<float, float>(scaleX, scaleY), 1);
						pset->setRetinaPatternOffset(std::pair<float, float>(offsetX, offsetY), 1);
					}
					else if (!strcmp(name, "retinaFaster")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setRetinaPatternScale(std::pair<float, float>(scaleX, scaleY), 2);
						pset->setRetinaPatternOffset(std::pair<float, float>(offsetX, offsetY), 2);
					}
					else if (!strcmp(name, "corneaNormal")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setCorneaPatternScale(std::pair<float, float>(scaleX, scaleY), 0);
						pset->setCorneaPatternOffset(std::pair<float, float>(offsetX, offsetY), 0);
					}
					else if (!strcmp(name, "corneaFast")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setCorneaPatternScale(std::pair<float, float>(scaleX, scaleY), 1);
						pset->setCorneaPatternOffset(std::pair<float, float>(offsetX, offsetY), 1);
					}
					else if (!strcmp(name, "corneaFaster")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setCorneaPatternScale(std::pair<float, float>(scaleX, scaleY), 2);
						pset->setCorneaPatternOffset(std::pair<float, float>(offsetX, offsetY), 2);
					}
					else if (!strcmp(name, "topographyNormal")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setTopographyPatternScale(std::pair<float, float>(scaleX, scaleY), 0);
						pset->setTopographyPatternOffset(std::pair<float, float>(offsetX, offsetY), 0);
					}
					else if (!strcmp(name, "topographyFast")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setTopographyPatternScale(std::pair<float, float>(scaleX, scaleY), 1);
						pset->setTopographyPatternOffset(std::pair<float, float>(offsetX, offsetY), 1);
					}
					else if (!strcmp(name, "topographyFaster")) {
						if (checkXMLResult(elem->QueryFloatAttribute("scaleX", &scaleX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("scaleY", &scaleY))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetX", &offsetX))) {
						}
						if (checkXMLResult(elem->QueryFloatAttribute("offsetY", &offsetY))) {
						}
						pset->setTopographyPatternScale(std::pair<float, float>(scaleX, scaleY), 2);
						pset->setTopographyPatternOffset(std::pair<float, float>(offsetX, offsetY), 2);
					}
				}
			}
		}
	}

	items = group->FirstChildElement("ReferenceRange");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr)
				{
					int value;
					if (!strcmp(name, "rangeExtent")) {
						if (checkXMLResult(elem->QueryIntAttribute("lower", &value))) {
							pset->setReferenceRangeLowerSize(value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("upper", &value))) {
							pset->setReferenceRangeUpperSize(value);
						}
					}
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfAngioParams(AngioSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	// int pixels;
	// float size;

	XMLElement* items = group->FirstChildElement("AngioParams");
	if (items != nullptr)
	{
		for (child = items->FirstChild(); child != nullptr; child = child->NextSibling()) {
			if ((elem = child->ToElement()) != nullptr) {
				const char* name = elem->Attribute("name");
				if (name != nullptr)
				{
					float decorThreshold = 0.0f, motionThreshold = 1.0f, motionOverPoints = 0.5f, motionDistRatio = 1.15f;
					int averageOffset = 2, filterOrients = 12;
					float filterSigma = 3.0f, filterDivider = 1.5f, filterWeight = 0.5f;
					float contrast = 1.0f, biasField = 15.0f;

					if (!strcmp(name, "decorrelation")) {
						if (checkXMLResult(elem->QueryFloatAttribute("threshold", &decorThreshold))) {
							pset->setDecorThreshold(decorThreshold);
						}
						if (checkXMLResult(elem->QueryIntAttribute("averageOffset", &averageOffset))) {
							pset->setAverageOffset(averageOffset);
						}
					}
					else if (!strcmp(name, "motionCorrect")) {
						if (checkXMLResult(elem->QueryFloatAttribute("threshold", &motionThreshold))) {
							pset->setMotionThreshold(motionThreshold);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("overPoints", &motionOverPoints))) {
							pset->setMotionOverPoints(motionOverPoints);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("distRatio", &motionDistRatio))) {
							pset->setMotionDistRatio(motionDistRatio);
						}
					}
					else if (!strcmp(name, "filterParams")) {
						if (checkXMLResult(elem->QueryIntAttribute("orients", &filterOrients))) {
							pset->setFilterOrients(filterOrients);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("sigma", &filterSigma))) {
							pset->setFilterSigma(filterSigma);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("divider", &filterDivider))) {
							pset->setFilterDivider(filterDivider);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("weight", &filterWeight))) {
							pset->setFilterWeight(filterWeight);
						}					
					}
					else if (!strcmp(name, "enhanceParams")) {
						if (checkXMLResult(elem->QueryFloatAttribute("contrast", &contrast))) {
							pset->setEnhanceParam(contrast);
						}
						if (checkXMLResult(elem->QueryFloatAttribute("biasField", &biasField))) {
							pset->setBiasFieldSigma(biasField);
						}
					}
				}
			}
		}
	}
	return true;
}


bool OctConfig::ConfigFile::loadSectionOfStepMotors(DeviceSettings * pset, tinyxml2::XMLElement * group)
{
	XMLNode* child;
	XMLElement* elem;
	int value = 0;

	// Step Motors.
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* motors = group->FirstChildElement("StepMotors");
	if (motors == nullptr) return false;

	for (child = motors->FirstChild(); child != nullptr; child = child->NextSibling()) {
		if ((elem = child->ToElement()) != nullptr) {
			const char* name = elem->Attribute("name");
			if (name != nullptr) {
				StepMotorItem* item = pset->getStepMotorItem(name);
				if (item != nullptr) {
					if (checkXMLResult(elem->QueryIntAttribute("value", &value))) {
						item->setValue(value);
					}

					if (item->isType(StepMotorType::OCT_FOCUS) ||
						item->isType(StepMotorType::FUNDUS_FOCUS) ||
						item->isType(StepMotorType::POLARIZATION) ||
						item->isType(StepMotorType::REFERENCE) ||
						item->isType(StepMotorType::SPLIT_FOCUS)) {
						if (checkXMLResult(elem->QueryIntAttribute("origin", &value))) {
							item->setSetupPosition(0, value);
						}
					}
					else if (item->isType(StepMotorType::OCT_SAMPLE)) {
						if (checkXMLResult(elem->QueryIntAttribute("mirrorIn", &value))) {
							item->setSetupPosition(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("mirrorOut", &value))) {
							item->setSetupPosition(1, value);
						}
					}
					else if (item->isType(StepMotorType::FUNDUS_DIOPTER)) {
						if (checkXMLResult(elem->QueryIntAttribute("minus", &value))) {
							item->setSetupPosition(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("zero", &value))) {
							item->setSetupPosition(1, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("plus", &value))) {
							item->setSetupPosition(2, value);
						}
					}
					else if (item->isType(StepMotorType::OCT_DIOPTER)) {
						if (checkXMLResult(elem->QueryIntAttribute("minus", &value))) {
							item->setSetupPosition(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("zero", &value))) {
							item->setSetupPosition(1, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("plus", &value))) {
							item->setSetupPosition(2, value);
						}
					}
					else if (item->isType(StepMotorType::PUPIL_MASK)) {
						if (checkXMLResult(elem->QueryIntAttribute("normal", &value))) {
							item->setSetupPosition(0, value);
						}
						if (checkXMLResult(elem->QueryIntAttribute("small", &value))) {
							item->setSetupPosition(1, value);
						}
					}
				}
			}
		}
	}

	return true;
}


bool OctConfig::ConfigFile::saveDeviceSettings(const DeviceSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("DeviceSettings");
	root->LinkEndChild(group);

	if (!saveSectionOfLightLeds(pset, group)) {
		return false;
	}
	/*
	if (!saveSectionOfStepMotors(pset, group)) {
		return false;
	}
	*/
	return true;
}


bool OctConfig::ConfigFile::saveSignalSettings(const SignalSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("SignalSettings");
	root->LinkEndChild(group);

	XMLElement* elem = pdoc->NewElement("WavelengthFunction");
	group->LinkEndChild(elem);

	XMLElement* item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter0");
	item->SetAttribute("value", pset->getWavelengthParameter(0));

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter1");
	item->SetAttribute("value", pset->getWavelengthParameter(1));

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter2");
	item->SetAttribute("value", pset->getWavelengthParameter(2));

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter3");
	item->SetAttribute("value", pset->getWavelengthParameter(3));

	elem = pdoc->NewElement("DispersionCompensation");
	group->LinkEndChild(elem);

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter0");
	item->SetAttribute("value", pset->getDispersionParameter(0));

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter1");
	item->SetAttribute("value", pset->getDispersionParameter(1));

	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter2");
	item->SetAttribute("value", pset->getDispersionParameter(2));
	
	/*
	item = pdoc->NewElement("attribute");
	elem->LinkEndChild(item);
	item->SetAttribute("name", "parameter3");
	item->SetAttribute("value", pset->getDispersionParameter(3));
	*/
	return true;
}


bool OctConfig::ConfigFile::saveCameraSettings(const CameraSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("CameraSettings");
	root->LinkEndChild(group);

	if (!saveSectionOfCameras(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveFundusSettings(const FundusSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("FundusColor");
	root->LinkEndChild(group);

	if (!saveSectionOfFundus(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveCorneaSettings(const CorneaSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("CorneaImage");
	root->LinkEndChild(group);

	if (!saveSectionOfCornea(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveRetinaSettings(const RetinaSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("RetinaImage");
	root->LinkEndChild(group);

	if (!saveSectionOfRetina(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveFixationSettings(const FixationSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("InternalFixation");
	root->LinkEndChild(group);

	if (!saveSectionOfFixation(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveSystemSettings(SystemSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("SystemSettings");
	root->LinkEndChild(group);

	if (!saveSectionOfSystemOptions(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::savePatternSettings(PatternSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("PatternSettings");
	root->LinkEndChild(group);

	if (!saveSectionOfPatternRange(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveAngioSettings(AngioSettings * pset)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	tinyxml2::XMLNode* root = getImpl().pRoot;

	XMLElement* group = pdoc->NewElement("AngioSettings");
	root->LinkEndChild(group);

	if (!saveSectionOfAngioParams(pset, group)) {
		return false;
	}
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfLightLeds(const DeviceSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;

	// Light LEDs.
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* lights = pdoc->NewElement("LightLEDs");
	group->LinkEndChild(lights);

	XMLElement* elem;
	for (int index = 0; index < pset->getLightLedItemsCount(); index++) {
		LightLedItem* item;
		if ((item = pset->getLightLedItem(index)) != nullptr) {
			elem = pdoc->NewElement("LED");
			elem->SetAttribute("name", item->getName());
			elem->SetAttribute("value", item->getValue());
			lights->LinkEndChild(elem);
		}
	}
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfStepMotors(const DeviceSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;

	// Step Motors.
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* motors = pdoc->NewElement("StepMotors");
	group->LinkEndChild(motors);

	XMLElement* elem;
	for (int index = 0; index < pset->getStepMotorItemsCount(); index++) {
		StepMotorItem* item;
		if ((item = pset->getStepMotorItem(index)) != nullptr) {
			elem = pdoc->NewElement("MOTOR");
			elem->SetAttribute("name", item->getName());
			elem->SetAttribute("value", item->getValue());

			if (item->isType(StepMotorType::OCT_FOCUS) ||
				item->isType(StepMotorType::FUNDUS_FOCUS) ||
				item->isType(StepMotorType::POLARIZATION) ||
				item->isType(StepMotorType::REFERENCE) ||
				item->isType(StepMotorType::SPLIT_FOCUS)) {
				elem->SetAttribute("origin", item->getSetupPosition(0));
			}
			else if (item->isType(StepMotorType::OCT_SAMPLE)) {
				elem->SetAttribute("mirrorIn", item->getSetupPosition(0));
				elem->SetAttribute("mirrorOut", item->getSetupPosition(1));
			}
			else if (item->isType(StepMotorType::FUNDUS_DIOPTER) ||
				item->isType(StepMotorType::OCT_DIOPTER)) {
				elem->SetAttribute("minus", item->getSetupPosition(0));
				elem->SetAttribute("zero", item->getSetupPosition(1));
				elem->SetAttribute("plus", item->getSetupPosition(2));
			}
			else if (item->isType(StepMotorType::PUPIL_MASK)) {
				elem->SetAttribute("normal", item->getSetupPosition(0));
				elem->SetAttribute("small", item->getSetupPosition(1));
			}
			motors->LinkEndChild(elem);
		}
	}
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfCameras(const CameraSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;

	// Light LEDs.
	///////////////////////////////////////////////////////////////////////////////
	XMLElement* list = pdoc->NewElement("Cameras");
	group->LinkEndChild(list);

	XMLElement* elem;
	for (int index = 0; index < pset->getCameraListCount(); index++) {
		CameraItem* item;
		if ((item = pset->getCameraItem(index)) != nullptr) {
			elem = pdoc->NewElement("CAM");
			elem->SetAttribute("name", item->getName());
			elem->SetAttribute("again", item->analogGain());
			elem->SetAttribute("dgain", item->digitalGain());

			if (item->getType() == CameraType::RETINA) {
				elem->SetAttribute("again2", item->analogGain(1));
				elem->SetAttribute("dgain2", item->digitalGain(1));
				elem->SetAttribute("again3", item->analogGain(2));
				elem->SetAttribute("dgain3", item->digitalGain(2));
			}
			/*
			elem->SetAttribute("shiftX", item->shiftX());
			elem->SetAttribute("shiftY", item->shiftY());
			*/
			list->LinkEndChild(elem);
		}
	}
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfFundus(const FundusSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("WhiteBalance");
	group->LinkEndChild(list);

	vector<double> params = pset->getWhiteBalanceParameters();

	XMLElement* item;
	for (int index = 0; index < params.size(); index++) {
		item = pdoc->NewElement("attribute");
		list->LinkEndChild(item);

		name = "parameter" + to_string(index);
		item->SetAttribute("name", name.c_str());
		item->SetAttribute("value", params[index]);
	}

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useWhiteBalance());


	list = pdoc->NewElement("RadialCorrection");
	group->LinkEndChild(list);

	vector<int> coordXs = pset->getCorrectionCoordsX();
	vector<int> coordYs = pset->getCorrectionCoordsY();

	for (int index = 0; index < coordXs.size(); index++) {
		item = pdoc->NewElement("attribute");
		list->LinkEndChild(item);

		name = "coordinate" + to_string(index);
		item->SetAttribute("name", name.c_str());
		item->SetAttribute("x", coordXs[index]);
		item->SetAttribute("y", coordYs[index]);
	}

	list = pdoc->NewElement("RadialCorrection_FILR");
	group->LinkEndChild(list);

	vector<int> coordXs_FILR = pset->getCorrectionCoordsX_FILR();
	vector<int> coordYs_FILR = pset->getCorrectionCoordsY_FILR();

	for (int index = 0; index < coordXs_FILR.size(); index++) {
		item = pdoc->NewElement("attribute");
		list->LinkEndChild(item);

		name = "coordinate" + to_string(index);
		item->SetAttribute("name", name.c_str());
		item->SetAttribute("x", coordXs_FILR[index]);
		item->SetAttribute("y", coordYs_FILR[index]);
	}


	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useColorCorrection());


	list = pdoc->NewElement("LevelCorrection");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "ratio");
	item->SetAttribute("value", pset->radialCorrectionRatio());
	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useRadialCorrection());

	list = pdoc->NewElement("RemoveReflectionLight");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "light");
	item->SetAttribute("value", pset->getRemoveReflection1());
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "shadow");
	item->SetAttribute("value", pset->getRemoveReflection2());
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "outer");
	item->SetAttribute("value", pset->getRemoveReflection3());
	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useRemoveReflectionLight());
	list->LinkEndChild(item);
	
	list = pdoc->NewElement("AutoEnhancement");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "brightness");
	item->SetAttribute("value", pset->autoBright());
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "contrast");
	item->SetAttribute("value", pset->autoContrast());
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "ub");
	item->SetAttribute("value", pset->autoUB());
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "vr");
	item->SetAttribute("value", pset->autoVR());

	list = pdoc->NewElement("EdgeEnhancement");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "kernelSize");
	item->SetAttribute("value", pset->edgeKernelSize());


	list = pdoc->NewElement("ColorROI");
	group->LinkEndChild(list);

	auto center = pset->getCenterPointOfROI();
	auto radius = pset->getRadiusOfROI();

	for (int index = 0; index < 2; index++) {
		item = pdoc->NewElement("attribute");
		list->LinkEndChild(item);

		if (index == 0) {
			name = "center";
			item->SetAttribute("name", name.c_str());
			item->SetAttribute("x", center.first);
			item->SetAttribute("y", center.second);
		}
		else {
			name = "radius";
			item->SetAttribute("name", name.c_str());
			item->SetAttribute("r", radius);
		}
	}

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useROICoordinates());

	list = pdoc->NewElement("AutoFlash");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);

	name = "level";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("normal", pset->getAutoFlashLevelNormal());
	item->SetAttribute("min", pset->getAutoFlashLevelMin());
	item->SetAttribute("max", pset->getAutoFlashLevelMax());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "sizeBase";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashSizeBase(0));
	item->SetAttribute("value2", pset->getAutoFlashSizeBase(1));
	item->SetAttribute("value3", pset->getAutoFlashSizeBase(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "sizeHigh";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashSizeHigh(0));
	item->SetAttribute("value2", pset->getAutoFlashSizeHigh(1));
	item->SetAttribute("value3", pset->getAutoFlashSizeHigh(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "focusInt";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashFocusInt(0));
	item->SetAttribute("value2", pset->getAutoFlashFocusInt(1));
	item->SetAttribute("value3", pset->getAutoFlashFocusInt(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "defLevel";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashDefLevel(0));
	item->SetAttribute("value2", pset->getAutoFlashDefLevel(1));
	item->SetAttribute("value3", pset->getAutoFlashDefLevel(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "splitFocus";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashSplitFocus(0));
	item->SetAttribute("value2", pset->getAutoFlashSplitFocus(1));
	item->SetAttribute("value3", pset->getAutoFlashSplitFocus(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "retinaIr";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashRetinaIr(0));
	item->SetAttribute("value2", pset->getAutoFlashRetinaIr(1));
	item->SetAttribute("value3", pset->getAutoFlashRetinaIr(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "workDot1";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashWorkDot1(0));
	item->SetAttribute("value2", pset->getAutoFlashWorkDot1(1));
	item->SetAttribute("value3", pset->getAutoFlashWorkDot1(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	name = "workDot2";
	item->SetAttribute("name", name.c_str());
	item->SetAttribute("value1", pset->getAutoFlashWorkDot2(0));
	item->SetAttribute("value2", pset->getAutoFlashWorkDot2(1));
	item->SetAttribute("value3", pset->getAutoFlashWorkDot2(2));

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useAutoFlashLevel());
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfRetina(const RetinaSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("ImageEnhance");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);

	item->SetAttribute("name", "contrast");
	item->SetAttribute("clipLimit", pset->getEnhanceClipLimit());
	item->SetAttribute("clipScalar", pset->getEnhanceClipScalar());

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useImageEnhance());

	list = pdoc->NewElement("SplitFocus");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);

	item->SetAttribute("name", "center");
	item->SetAttribute("x", pset->getSplitCenter().first);
	item->SetAttribute("y", pset->getSplitCenter().second);

	/*
	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);

	item->SetAttribute("name", "kernel");
	item->SetAttribute("width", pset->getSplitKernel().first);
	item->SetAttribute("height", pset->getSplitKernel().second);
	*/

    list = pdoc->NewElement("RetinaROI");
    group->LinkEndChild(list);

    auto centerRoi = pset->getCenterPointOfROI();
	auto centerMask = pset->getCenterPointOfMask();
    auto radius = pset->getRadiusOfROI();

    for (int index = 0; index < 2; index++) {
        item = pdoc->NewElement("attribute");
        list->LinkEndChild(item);

		if (index == 0) {
			name = "center";
			item->SetAttribute("name", name.c_str());
			item->SetAttribute("x", centerRoi.first);
			item->SetAttribute("y", centerRoi.second);
		} 
		else {
			name = "radius";
			item->SetAttribute("name", name.c_str());
			item->SetAttribute("r", radius);
		}
    }

    item = pdoc->NewElement("status");
    list->LinkEndChild(item);
    item->SetAttribute("valid", pset->useROI());

	list = pdoc->NewElement("CenterMask");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	item->SetAttribute("name", "center");
	item->SetAttribute("x", centerMask.first);
	item->SetAttribute("y", centerMask.second);
	list->LinkEndChild(item);

	item = pdoc->NewElement("attribute");
	item->SetAttribute("name", "radius");
	item->SetAttribute("value", pset->getCenterMaskSize());
	list->LinkEndChild(item);

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useCenterMask());
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfCornea(const CorneaSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("SmallPupil");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "pixelsPerMM");
	item->SetAttribute("value", pset->getPixelsPerMM());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "smallPupilSize");
	item->SetAttribute("value", pset->getSmallPupilSize());

	return true;
}


bool OctConfig::ConfigFile::saveSectionOfFixation(const FixationSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("LcdFixation");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "centerOD");
	item->SetAttribute("row", pset->getCenterOD().first);
	item->SetAttribute("col", pset->getCenterOD().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "fundusOD");
	item->SetAttribute("row", pset->getFundusOD().first);
	item->SetAttribute("col", pset->getFundusOD().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "scanDiskOD");
	item->SetAttribute("row", pset->getScanDiskOD().first);
	item->SetAttribute("col", pset->getScanDiskOD().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftSideOD");
	item->SetAttribute("row", pset->getLeftSideOD(0).first);
	item->SetAttribute("col", pset->getLeftSideOD(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftUpOD");
	item->SetAttribute("row", pset->getLeftSideOD(1).first);
	item->SetAttribute("col", pset->getLeftSideOD(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftDownOD");
	item->SetAttribute("row", pset->getLeftSideOD(2).first);
	item->SetAttribute("col", pset->getLeftSideOD(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightSideOD");
	item->SetAttribute("row", pset->getRightSideOD(0).first);
	item->SetAttribute("col", pset->getRightSideOD(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightUpOD");
	item->SetAttribute("row", pset->getRightSideOD(1).first);
	item->SetAttribute("col", pset->getRightSideOD(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightDownOD");
	item->SetAttribute("row", pset->getRightSideOD(2).first);
	item->SetAttribute("col", pset->getRightSideOD(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "centerOS");
	item->SetAttribute("row", pset->getCenterOS().first);
	item->SetAttribute("col", pset->getCenterOS().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "fundusOS");
	item->SetAttribute("row", pset->getFundusOS().first);
	item->SetAttribute("col", pset->getFundusOS().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "scanDiskOS");
	item->SetAttribute("row", pset->getScanDiskOS().first);
	item->SetAttribute("col", pset->getScanDiskOS().second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftSideOS");
	item->SetAttribute("row", pset->getLeftSideOS(0).first);
	item->SetAttribute("col", pset->getLeftSideOS(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftUpOS");
	item->SetAttribute("row", pset->getLeftSideOS(1).first);
	item->SetAttribute("col", pset->getLeftSideOS(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "leftDownOS");
	item->SetAttribute("row", pset->getLeftSideOS(2).first);
	item->SetAttribute("col", pset->getLeftSideOS(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightSideOS");
	item->SetAttribute("row", pset->getRightSideOS(0).first);
	item->SetAttribute("col", pset->getRightSideOS(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightUpOS");
	item->SetAttribute("row", pset->getRightSideOS(1).first);
	item->SetAttribute("col", pset->getRightSideOS(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rightDownOS");
	item->SetAttribute("row", pset->getRightSideOS(2).first);
	item->SetAttribute("col", pset->getRightSideOS(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "brightness");
	item->SetAttribute("value", pset->getBrightness());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "blink");
	item->SetAttribute("value", pset->useLcdBlinkOn());
	item->SetAttribute("period", pset->getBlinkPeriod());
	item->SetAttribute("onTime", pset->getBlinkOnTime());
	item->SetAttribute("type", pset->getFixationType());

	item = pdoc->NewElement("status");
	list->LinkEndChild(item);
	item->SetAttribute("valid", pset->useLcdFixation());
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfSystemOptions(SystemSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("SystemInfo");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "SerialNumber");
	item->SetAttribute("value", pset->serialNumber().c_str());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "ModelType");
	item->SetAttribute("value", pset->getModelType());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "OctGrabberType");
	item->SetAttribute("value", pset->getOctGrabberType());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "OctLineCameraMode");
	item->SetAttribute("value", pset->getOctLineCameraMode());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "SldFaultDetection");
	item->SetAttribute("value", pset->getSldFaultDetection());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "RetinaTrackingSpeed");
	item->SetAttribute("value", pset->getRetinaTrackingSpeed());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "TriggerForePadding");
	item->SetAttribute("fore1", pset->getTriggerForePadd(0));
	item->SetAttribute("fore2", pset->getTriggerForePadd(1));
	item->SetAttribute("fore3", pset->getTriggerForePadd(2));

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "TriggerPostPadding");
	item->SetAttribute("post1", pset->getTriggerPostPadd(0));
	item->SetAttribute("post2", pset->getTriggerPostPadd(1));
	item->SetAttribute("post3", pset->getTriggerPostPadd(2));

	/*item->SetAttribute("name", "OctSoloVersion");
	item->SetAttribute("value", pset->useOctSoloVersion());*/
	/*
	XMLElement* list = pdoc->NewElement("SystemOptions");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "checkMainboardAtStartup");
	item->SetAttribute("value", pset->checkMainboardAtStartup());
	*/
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfPatternRange(PatternSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("PatternRange");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "retinaNormal");
	item->SetAttribute("scaleX", pset->retinaPatternScale(0).first);
	item->SetAttribute("scaleY", pset->retinaPatternScale(0).second);
	item->SetAttribute("offsetX", pset->retinaPatternOffset(0).first);
	item->SetAttribute("offsetY", pset->retinaPatternOffset(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "retinaFast");
	item->SetAttribute("scaleX", pset->retinaPatternScale(1).first);
	item->SetAttribute("scaleY", pset->retinaPatternScale(1).second);
	item->SetAttribute("offsetX", pset->retinaPatternOffset(1).first);
	item->SetAttribute("offsetY", pset->retinaPatternOffset(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "retinaFaster");
	item->SetAttribute("scaleX", pset->retinaPatternScale(2).first);
	item->SetAttribute("scaleY", pset->retinaPatternScale(2).second);
	item->SetAttribute("offsetX", pset->retinaPatternOffset(2).first);
	item->SetAttribute("offsetY", pset->retinaPatternOffset(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "corneaNormal");
	item->SetAttribute("scaleX", pset->corneaPatternScale(0).first);
	item->SetAttribute("scaleY", pset->corneaPatternScale(0).second);
	item->SetAttribute("offsetX", pset->corneaPatternOffset(0).first);
	item->SetAttribute("offsetY", pset->corneaPatternOffset(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "corneaFast");
	item->SetAttribute("scaleX", pset->corneaPatternScale(1).first);
	item->SetAttribute("scaleY", pset->corneaPatternScale(1).second);
	item->SetAttribute("offsetX", pset->corneaPatternOffset(1).first);
	item->SetAttribute("offsetY", pset->corneaPatternOffset(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "corneaFaster");
	item->SetAttribute("scaleX", pset->corneaPatternScale(2).first);
	item->SetAttribute("scaleY", pset->corneaPatternScale(2).second);
	item->SetAttribute("offsetX", pset->corneaPatternOffset(2).first);
	item->SetAttribute("offsetY", pset->corneaPatternOffset(2).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "topographyNormal");
	item->SetAttribute("scaleX", pset->topographyPatternScale(0).first);
	item->SetAttribute("scaleY", pset->topographyPatternScale(0).second);
	item->SetAttribute("offsetX", pset->topographyPatternOffset(0).first);
	item->SetAttribute("offsetY", pset->topographyPatternOffset(0).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "topographyFast");
	item->SetAttribute("scaleX", pset->topographyPatternScale(1).first);
	item->SetAttribute("scaleY", pset->topographyPatternScale(1).second);
	item->SetAttribute("offsetX", pset->topographyPatternOffset(1).first);
	item->SetAttribute("offsetY", pset->topographyPatternOffset(1).second);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "topographyFaster");
	item->SetAttribute("scaleX", pset->topographyPatternScale(2).first);
	item->SetAttribute("scaleY", pset->topographyPatternScale(2).second);
	item->SetAttribute("offsetX", pset->topographyPatternOffset(2).first);
	item->SetAttribute("offsetY", pset->topographyPatternOffset(2).second);

	list = pdoc->NewElement("ReferenceRange");
	group->LinkEndChild(list);

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "rangeExtent");
	item->SetAttribute("upper", pset->getReferenceRangeUpperSize());
	item->SetAttribute("lower", pset->getReferenceRangeLowerSize());
	return true;
}


bool OctConfig::ConfigFile::saveSectionOfAngioParams(AngioSettings * pset, tinyxml2::XMLElement * group)
{
	tinyxml2::XMLDocument* pdoc = getImpl().pDoc;
	string name;

	XMLElement* list = pdoc->NewElement("AngioParams");
	group->LinkEndChild(list);

	XMLElement* item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "decorrelation");
	item->SetAttribute("threshold", pset->getDecorThreshold());
	item->SetAttribute("averageOffset", pset->getAverageOffset());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "motionCorrect");
	item->SetAttribute("threshold", pset->getMotionThreshold());
	item->SetAttribute("overPoints", pset->getMotionOverPoints());
	item->SetAttribute("distRatio", pset->getMotionDistRatio());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "filterParams");
	item->SetAttribute("orients", pset->getFilterOrients());
	item->SetAttribute("sigma", pset->getFilterSigma());
	item->SetAttribute("divider", pset->getFilterDivider());
	item->SetAttribute("weight", pset->getFilterWeight());

	item = pdoc->NewElement("attribute");
	list->LinkEndChild(item);
	item->SetAttribute("name", "enhanceParams");
	item->SetAttribute("contrast", pset->getEnhanceParam());
	item->SetAttribute("biasField", pset->getBiasFieldSigma());
	return true;
}
