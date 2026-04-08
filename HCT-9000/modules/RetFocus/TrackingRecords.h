#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>
#include <memory>
#include <mutex>

namespace RetFocus
{
	class TrackingObject;

	class RETFOCUS_DLL_API TrackingRecords
	{
	public:
		TrackingRecords();
		~TrackingRecords();

		TrackingRecords(TrackingRecords&& rhs);
		TrackingRecords& operator=(TrackingRecords&& rhs);

	public:
		void beginSession(bool refer, float x = 0.0f, float y = 0.0f);
		void resetSession(void);
		void resumeSession(void);
        void closeSession(void);
        void insertRecord(const RetFocus::TrackingObject& obj);
		void insertRecord(float x, float y, float score);

		int getTrackingCount(void) const;
		bool isReferenceTarget(void) const;
		void registerReferenceTarget(float x, float y);
		bool checkTargetStabilized(int count = 2);

		bool getLastPosition(float& x, float& y, float& score) const;
		bool getLastDistance(float from_x, float from_y,float& xpos, float& ypos, float& dx, float& dy, float& dist, float& score) const;

		bool hasTargetValidated(void);
		bool hasTargetMotionFreed(void);
		bool hasTargetOccluded(void);

    protected:
		bool isRecording(void) const;
		bool isEmpty(void) const;
		int getScoreCount(void) const;
		
		void setReferencePosition(float x, float y);

		bool isTargetConsistent(void);
		bool isTargetOccluded(bool& occluding);
		bool isTargetDisplaced(void);
		bool isTargetOutranged(void);
		bool isTargetUnmatched(void);

		void clearRecords(void);
		bool getLastPosition(float& x, float& y, float& score, float& dist) const; 
        void addTrackingObject(const RetFocus::TrackingObject& obj);

    private:
        struct TrackingRecordsImpl;
        std::unique_ptr<TrackingRecordsImpl> d_ptr;
        TrackingRecordsImpl& impl(void) const;
	};
}