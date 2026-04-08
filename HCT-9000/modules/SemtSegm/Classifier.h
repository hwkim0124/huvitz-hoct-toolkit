#pragma once

#include "SemtSegmDef.h"

namespace SemtSegm
{
	class BscanSegmentator;
	class Sampling;

	class SEMTSEGM_DLL_API Classifier
	{
	public:
		Classifier(BscanSegmentator* pSegm);
		virtual ~Classifier();

		Classifier(Classifier&& rhs);
		Classifier& operator=(Classifier&& rhs);
		Classifier(const Classifier& rhs) = delete;
		Classifier& operator=(const Classifier& rhs) = delete;

	public:
		bool prepareInput();
		bool prepareInput(const OcularImage* image, const std::vector<int>& inner, int index=0);
		bool classifyPixels(bool save = false);

		int getAlignTopPosition() const;
		const std::vector<int>& getAlignShifts() const;

		const OcularImage* getInputImage(void) const;
		const OcularImage* getInputDenoised(void) const;

		const OcularImage* getScoreVIT() const;
		const OcularImage* getScoreNFL() const;
		const OcularImage* getScoreOPL() const;
		const OcularImage* getScoreONL() const;
		const OcularImage* getScoreRPE() const;
		const OcularImage* getScoreCHO() const;

		static const int INPUT_WIDTH = 256;
		static const int INPUT_HEIGHT = 256;
		static const int SOURCE_ROI_TOP = 48;
		static const int SOURCE_WIDTH = 256;
		static const int SOURCE_HEIGHT = 768;

	protected:
		bool createAlignImage(const OcularImage* image, const std::vector<int>& inner);
		bool composeLayerMaps();

	private:
		struct ClassifierImpl;
		std::unique_ptr<ClassifierImpl> d_ptr;
		ClassifierImpl& getImpl(void) const;
	};
}

