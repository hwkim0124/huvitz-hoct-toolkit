#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <vector>


namespace OctConfig
{
	class OCTCONFIG_DLL_API FundusSettings
	{
	public:
		FundusSettings();
		virtual ~FundusSettings();

		FundusSettings(FundusSettings&& rhs);
		FundusSettings& operator=(FundusSettings&& rhs);
		FundusSettings(const FundusSettings& rhs);
		FundusSettings& operator=(const FundusSettings& rhs);


	public:
		void initialize(void);
		void resetToDefaultValues(void);

		std::vector<double> getWhiteBalanceParameters(void) const;
		std::vector<int> getCorrectionCoordsX(void) const;
		std::vector<int> getCorrectionCoordsY(void) const;
		std::vector<int> getCorrectionCoordsX_FILR(void) const;
		std::vector<int> getCorrectionCoordsY_FILR(void) const;
		std::pair<int, int> getCenterPointOfROI(void) const;
		int getRadiusOfROI(void) const;
		int getAutoFlashLevelNormal(void) const;
		int getAutoFlashLevelMin(void) const;
		int getAutoFlashLevelMax(void) const;

		int getAutoFlashSizeBase(int index = 0) const;
		int getAutoFlashSizeHigh(int index = 0) const;
		int getAutoFlashFocusInt(int index = 0) const;
		int getAutoFlashDefLevel(int index = 0) const;

		int getAutoFlashSplitFocus(int index = 0) const;
		int getAutoFlashRetinaIr(int index = 0) const;
		int getAutoFlashWorkDot1(int index = 0) const;
		int getAutoFlashWorkDot2(int index = 0) const;
		int getRemoveReflection1(void) const;
		int getRemoveReflection2(void) const;
		int getRemoveReflection3(void) const;

		float autoBright(void) const;
		float autoContrast(void) const;
		float autoUB(void) const;
		float autoVR(void) const;
		float radialCorrectionRatio(void) const;
		int edgeKernelSize(void) const;
		int fundusImageType(int index) const;

		void setWhiteBalanceParameters(std::vector<double> params);
		void setCorrectionCoordsX(std::vector<int> xs);
		void setCorrectionCoordsY(std::vector<int> ys);
		void setCorrectionCoordsX_FILR(std::vector<int> xs);
		void setCorrectionCoordsY_FILR(std::vector<int> ys);
		void setCenterPointOfROI(std::pair<int, int> point);
		void setRadiusOfROI(int radius);
		void setRemoveReflection1(int radius);
		void setRemoveReflection2(int radius);
		void setRemoveReflection3(int radius);
		void setAutoFlashLevelNormal(int level);
		void setAutoFlashLevelMin(int level);
		void setAutoFlashLevelMax(int level);

		void setAutoFlashSizeBase(int value, int index = 0);
		void setAutoFlashSizeHigh(int value, int index = 0);
		void setAutoFlashFocusInt(int value, int index = 0);
		void setAutoFlashDefLevel(int value, int index = 0);

		void setAutoFlashSplitFocus(int value, int index = 0);
		void setAutoFlashRetinaIr(int value, int index = 0);
		void setAutoFlashWorkDot1(int value, int index = 0);
		void setAutoFlashWorkDot2(int value, int index = 0);

		void setAutoBright(float value);
		void setAutoContrast(float value);
		void setAutoUB(float value);
		void setAutoVR(float value);
		void setRadialCorrectionRatio(float value);
		void setEdgeKernelSize(int size);
		void setFundusImageType(int value);

		bool useWhiteBalance(bool isset = false, bool flag = false) const;
		bool useColorCorrection(bool isset = false, bool flag = false) const;
		bool useROICoordinates(bool isset = false, bool flag = false) const;
		bool useAutoFlashLevel(bool isset = false, bool flag = false) const;
		bool useRadialCorrection(bool isset = false, bool flag = false) const;
		bool useRemoveReflectionLight(bool isset = false, bool flag = false) const;

	protected:
		struct FundusSettingsImpl;
		std::unique_ptr<FundusSettingsImpl> d_ptr;
		FundusSettingsImpl& getImpl(void) const;
	};
}
