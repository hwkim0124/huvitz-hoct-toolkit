#pragma once
#include "LayeredBscan.h"
#include "BscanSegmentator.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API OpticDiscBscan :
		public LayeredBscan
	{
	public:
		OpticDiscBscan();
		virtual ~OpticDiscBscan();

		OpticDiscBscan(OpticDiscBscan&& rhs);
		OpticDiscBscan& operator=(OpticDiscBscan&& rhs);
		OpticDiscBscan(const OpticDiscBscan& rhs) = delete;
		OpticDiscBscan& operator=(const OpticDiscBscan& rhs) = delete;

	public:
		bool performSegmentation() override;

		int getOpticDiscX1() const;
		int getOpticDiscX2() const;
		int getOpticDiscPixels() const;
		int getOpticCupX1() const;
		int getOpticCupX2() const;
		int getOpticCupPixels() const;

		bool isOpticDiscRegion() const;
		bool isOpticCupRegion() const;

	protected:
		void resetBscanSegmentator() override;

	private:
		struct OpticDiscBscanImpl;
		std::unique_ptr<OpticDiscBscanImpl> d_ptr;
		OpticDiscBscanImpl& getImpl(void) const;
	};
}
