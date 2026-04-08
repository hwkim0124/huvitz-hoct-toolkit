#pragma once

#include "CppUtilDef.h"

#include <memory>


namespace CppUtil
{
	class CPPUTIL_DLL_API ClockTimer
	{
	public:
		ClockTimer();
		virtual ~ClockTimer();

	public:
		static void start(std::uint32_t tid = 0);

		static double elapsed_sec(std::uint32_t tid = 0, bool whole = false);
		static double elapsed_ms(std::uint32_t tid = 0, bool whole = false);

		static double elapsedMsec(std::uint32_t tid = 0);

	protected:
		static double sample(std::uint32_t tid, bool whole);

	private:
		struct ClockTimerImpl;
		static std::unique_ptr<ClockTimerImpl> d_ptr;
		static ClockTimerImpl& getImpl(void);
	};
}
