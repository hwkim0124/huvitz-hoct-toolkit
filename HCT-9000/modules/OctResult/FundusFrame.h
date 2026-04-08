#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>

namespace OctResult
{
	class OCTRESULT_DLL_API FundusFrame
	{
	public:
		FundusFrame();
		FundusFrame(const OctFundusFrame& desc);
		FundusFrame(const std::uint8_t* data, int width, int height);
		virtual ~FundusFrame();

		FundusFrame(FundusFrame&& rhs);
		FundusFrame& operator=(FundusFrame&& rhs);
		FundusFrame(const FundusFrame& rhs);
		FundusFrame& operator=(const FundusFrame& rhs);

	public:
		void setData(const std::uint8_t* data, int width, int height);
		void setData(const OctFundusFrame& desc);

		const std::uint8_t* getBuffer(void) const;
		const OctFundusFrame& getDescript(void) const;

		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;

		bool exportFile(const std::wstring& path);
		bool importFile(const std::wstring& path);

	private:
		struct FundusFrameImpl;
		std::unique_ptr<FundusFrameImpl> d_ptr;
		FundusFrameImpl& getImpl(void) const;
	};

	typedef std::vector<FundusFrame> FundusFrameList;
}
