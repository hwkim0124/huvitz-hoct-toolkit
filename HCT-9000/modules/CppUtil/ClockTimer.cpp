#include "stdafx.h"
#include "ClockTimer.h"

#include <map>

using namespace CppUtil;
using namespace std;


struct ClockTimer::ClockTimerImpl {

	LARGE_INTEGER freq;

	std::map<uint32_t, LARGE_INTEGER> startTimes;
	std::map<uint32_t, LARGE_INTEGER> recordTimes;

	ClockTimerImpl() {
		QueryPerformanceFrequency(&freq);
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ClockTimer::ClockTimerImpl> ClockTimer::d_ptr(new ClockTimerImpl());


ClockTimer::ClockTimer()
{
}


ClockTimer::~ClockTimer()
{
}


ClockTimer::ClockTimerImpl & CppUtil::ClockTimer::getImpl(void)
{
	return *d_ptr;
}


void CppUtil::ClockTimer::start(std::uint32_t tid)
{
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);
	d_ptr->startTimes[tid] = start;
	d_ptr->recordTimes[tid] = start;
	return;
}


double CppUtil::ClockTimer::elapsed_sec(std::uint32_t tid, bool whole)
{
	return sample(tid, whole) ;
}


double CppUtil::ClockTimer::elapsed_ms(std::uint32_t tid, bool whole)
{
	return sample(tid, whole) * 1e3;
}


double CppUtil::ClockTimer::sample(std::uint32_t tid, bool whole)
{
	LARGE_INTEGER present, record ;

	QueryPerformanceCounter(&present);
	record = (whole ? d_ptr->startTimes[tid] : d_ptr->recordTimes[tid]);

	double elapsed = (double)(present.QuadPart - record.QuadPart) / (double)(d_ptr->freq.QuadPart);
	d_ptr->recordTimes[tid] = present;
	return elapsed;
}


double CppUtil::ClockTimer::elapsedMsec(std::uint32_t tid)
{
	return elapsed_ms(tid, false);
}

