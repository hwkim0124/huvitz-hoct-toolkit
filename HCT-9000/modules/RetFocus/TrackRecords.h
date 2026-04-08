#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>
#include <memory>
#include <mutex>

namespace RetFocus
{
	class RETFOCUS_DLL_API TrackRecords
	{
	public:
		TrackRecords();
		~TrackRecords();

		TrackRecords(TrackRecords&& rhs);
		TrackRecords& operator=(TrackRecords&& rhs);

	public:
		void insertMovement(bool valid, bool covered, float mean, float stdev, float simval, float dx, float dy);
		void clearRecords(void);
		void setIsRecording(bool flag);
		void setInitialPosition(float x, float y);

		void getLastPosition(float& x_pos, float& y_pos);
		void getLastDrifts(float& dx, float& dy);

		bool isAvailable(int size) const;
		bool isConsistent(bool align) const;
		bool isIdentified(void) const;
		bool isDisplaced(void) const;

		bool isOutOfRange(void) const;
		bool isUnspotted(void) const;
		bool isCovered(void) const;
		bool isDrifted(void) const;
		bool isMismatched(void) const;

		bool isTargetAligned(void) const;
		bool isCenterAligned(void) const;

	private:
		struct TrackRecordsImpl;
		std::unique_ptr<TrackRecordsImpl> d_ptr;
		TrackRecordsImpl& impl(void) const;
	};
}

