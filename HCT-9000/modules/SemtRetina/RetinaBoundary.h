#pragma once

#include "SemtRetinaDef.h"

namespace SemtRetina
{
	class RetinaSegmenter;

	class SEMTRETINA_DLL_API RetinaBoundary
	{
	public:
		RetinaBoundary(RetinaSegmenter* segm);
		virtual ~RetinaBoundary();

		RetinaBoundary(RetinaBoundary&& rhs);
		RetinaBoundary& operator=(RetinaBoundary&& rhs);
		RetinaBoundary(const RetinaBoundary& rhs) = delete;
		RetinaBoundary& operator=(const RetinaBoundary& rhs) = delete;

	public:
		std::vector<int>& sampleYs(void) const;
		std::vector<int>& sourceYs(void) const;

	protected:
		const RetinaSegmenter* retinaSegmenter(void) const;

		bool searchPathMinCost();
		bool searchPathMinCostInRange(void);
		bool resizeBoundaryPath(std::vector<int> path, int src_w, int src_h, int targ_w, int targ_h, std::vector<int>& outs);
		std::vector<int> smoothOptimalPath(int filt_size, int degree, bool nerve_head, std::vector<int> ilms = std::vector<int>());

		std::vector<int>& upperYs(void) const;
		std::vector<int>& lowerYs(void) const;
		std::vector<int>& deltaYs(void) const;
		std::vector<int>& optimalPath(void) const;

		cv::Mat& pathEdgeMat(void) const;
		cv::Mat& pathProbMat(void) const;
		cv::Mat& pathCostMat(void) const;

	private:
		struct RetinaBoundaryImpl;
		std::unique_ptr<RetinaBoundaryImpl> d_ptr;
		RetinaBoundaryImpl& impl(void) const;
	};

}

