#pragma once

#include "SemtSegmDef.h"

namespace SemtSegm
{
	class BscanSegmentator;
	class Sampling;

	class SEMTSEGM_DLL_API Bordering
	{
	public:
		Bordering(BscanSegmentator* pSegm);
		virtual ~Bordering();

		Bordering(Bordering&& rhs);
		Bordering& operator=(Bordering&& rhs);
		Bordering(const Bordering& rhs) = delete;
		Bordering& operator=(const Bordering& rhs) = delete;

	public:
		bool establishOcularBound();
		bool locateOutlinePoints();
		bool reclaimOutlinePoints();
		bool interpolateOuterPoints(std::vector<int>& points);

		virtual void computeOuterWeights(const std::vector<int>& edges);
		virtual void computeInnerWeights();
		virtual void removeOuterOutliers(bool debris=false);
		virtual void removeInnerOutliers();

		virtual bool estimateInnerBorder();
		virtual bool estimateOuterBorder();
		virtual bool reclaimOuterBorder();

		virtual void inflateOuterPointsInSideEdges();
		virtual bool removeOuterPointsInOpticDisc();

		virtual bool detectReversedRegion();
		virtual bool detectOpticDiscRegion();
		virtual bool removeOuterPointsAtDiscBase();
		virtual bool inflateOpticDiscRegion();

		virtual bool buildInnerBoundary();
		virtual bool buildOuterBoundary();

		void setSampling(Sampling *sampling);
		bool isReversedRegion() const;
		bool isOpticDiscRegion() const;
		bool isOpticDiscOpening() const;

		int getOpticDiscX1() const;
		int getOpticDiscX2() const;
		int getOpticDiscBaseX() const;
		int getOpticDiscBaseY() const;

		std::vector<int>& getOuterEdges() const;
		std::vector<int>& getInnerEdges() const;
		std::vector<int>& getInnerBound() const;
		std::vector<int>& getOuterBound() const;
		std::vector<int>& getCenterBound() const;
		std::vector<int>& getOuterBorder() const;
		std::vector<int>& getInnerBorder() const;
		std::vector<int>& getOuterWeights() const;
		std::vector<int>& getInnerWeights() const;

	protected:
		const Sampling* sampling();

		bool interpolateByWeightFitt(const std::vector<int>& input, const std::vector<int>& weights, 
			std::vector<int>& output, bool sideFitt, float sideData = 0.025f);
		bool interpolateByLinearFitt(const std::vector<int>& input, std::vector<int>& output,
			bool sideFitt, int moveSpan = 3, float sideData = 0.025f);

	private:
		struct BorderingImpl;
		std::unique_ptr<BorderingImpl> d_ptr;
		BorderingImpl& getImpl(void) const;
	};
}

