#pragma once


#include "OctAngioDef.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioLayout;

	class OCTANGIO_DLL_API AngioMotion
	{
	public:
		AngioMotion();
		virtual ~AngioMotion();

		AngioMotion(AngioMotion&& rhs);
		AngioMotion& operator=(AngioMotion&& rhs);
		AngioMotion(const AngioMotion& rhs);
		AngioMotion& operator=(const AngioMotion& rhs);

	public:
		void clearAngioMotion(void);
		int getMotionVersion(void) const;
		void setMotionVersion(int version);	
		bool isMotionShiftSupported(void) const;

		void setBscanTranslations(const std::vector<float>& transX, const std::vector<float>& transY, const std::vector<float>& eccs);
		bool computeMotionShifts(const AngioLayout& layout, const std::vector<float>& angio);
		bool applyMotionCorrection(const AngioLayout& layout, std::vector<float>& angio);

		std::vector<int> getMotionShifts(void) const;
		void setMotionShifts(std::vector<int> shifts);

	public:
		void correctMotionArtifacts(const AngioLayout& layout, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers);
		void correctMotionArtifacts2(const AngioLayout& layout, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers, bool regist);

		bool correctBiasField(const AngioLayout& layout, std::vector<float>& projects, float sigma, bool isFovea=false);
		bool rotateVerticalScan(const AngioLayout& layout, bool direction, std::vector<float>& decorrs, std::vector<float>& differs, std::vector<float>& markers);
		bool rotateVerticalScan(const AngioLayout& layout, bool direction, std::vector<float>& project);
		int maskMotionAffectedLines(const AngioLayout& layout, std::vector<float>& vmask, std::vector<float>& dmask);

	protected:
		int splitMotionFreeStripes(const std::vector<float>& decorrs, const std::vector<float>& differs, const std::vector<float>& markers);
		bool measureStripesDisplacement(void);
		bool registMotionFreeStripes2(void);
		void interpolateEmptyLines(const std::vector<float>& markers);
		void resizeMotionFreeImages(void);

		float connectivityOfStripes(const std::vector<float>& stripe1, const std::vector<float>& stripe2,
									int size1, int size2, int pos1, int pos2, int width, int ksize = 5);
		float connectivityOfStripes2(const std::vector<float>& stripe1, const std::vector<float>& stripe2,
									int size1, int size2, int pos1, int pos2, int width, int ksize = 5);
		std::vector<float> featuresOfSplit(const std::vector<float>& image, int sidx, int eidx, int pos, int width, int height, int ksize = 5);
		std::vector<float> computeFeaturesOfSplits(const std::vector<float>& image, int sidx, int eidx, int pos, int width, int height, 
													const std::vector<float>& flows);

		bool makeMotionFreeStripes(const std::vector<float>& decorrs, const std::vector<float>& differs, const std::vector<float>& markers);
		bool registMotionFreeStripes(bool resize);
		bool composeMotionFreeProfiles(bool regist);

		AngioLayout& Layout(void) const;

	private:
		struct AngioMotionImpl;
		std::unique_ptr<AngioMotionImpl> d_ptr;
		AngioMotionImpl& getImpl(void) const;
	};
}

