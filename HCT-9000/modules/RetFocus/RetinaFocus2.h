#pragma once

#include "RetFocusDef.h"

#include <memory>
#include <string>


namespace RetFocus
{
	class RetinaFrame;

	class RETFOCUS_DLL_API RetinaFocus2
	{
	public:
		RetinaFocus2();
		virtual ~RetinaFocus2();

		RetinaFocus2(RetinaFocus2&& rhs);
		RetinaFocus2& operator=(RetinaFocus2&& rhs);

	public:
		RetinaFrame& getRetinaFrame(void) const;
		void setRetinaFrame(RetinaFrame&& image);

		bool isSplitFocus(void) const;
		bool isUpperSplit(void) const;
		bool isLowerSplit(void) const;
		float getSplitOffset(void) const;

		bool loadImage(const unsigned char* bits, int width, int height, int padding = 0);

	private:
		struct RetinaFocus2Impl;
		std::unique_ptr<RetinaFocus2Impl> d_ptr;
		RetinaFocus2Impl& getImpl(void);
	};
}

