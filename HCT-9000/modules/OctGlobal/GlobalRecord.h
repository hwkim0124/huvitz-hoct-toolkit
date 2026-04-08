#pragma once

#include "OctGlobalDef.h"

#include <memory>
#include <vector>


namespace OctGlobal
{
	class OCTGLOBAL_DLL_API GlobalRecord
	{
	public:
		GlobalRecord();
		virtual ~GlobalRecord();

	public:
		static void setOctImageSignalLevel(float level);
		static void setOctImageBackgroundLevel(float level);
		static void setOctImageNoiseLevel(float level);
		static float getOctImageBackgroundLevel(void);
		static float getOctImageNoiseLevel(void);
		static float getOctImageSignalLevel(void);

		static void writeSplitFocusOffset(float offset);
		static void writeSplitFocusSize(float size, float intensity);
		static void writeFlashPupilExtent(float extent);
		static void writeFlashPupilWidth(int width);

		static void readSplitFocusOffset(float& offset, unsigned long& count);
		static void readSplitFocusSize(float& size, float& intensity);
		static void readFlashPupilExtent(float& extent);
		static void readFlashPupilWidth(int& width);

		static void writeCorneaTrackTargetFound(bool flag, int lineIdx = 0);
		static void writeCorneaTrackAnteriorCurve(std::vector<int> curve, int lineIdx = 0);
		static void writeCorneaTrackAnteriorCenter(float xpos, float ypos, int lineIdx = 0);
		static void readCorneaTrackAnteriorCurve(std::vector<int>& curve, int lineIdx = 0);
		static void readCorneaTrackAnteriorCenter(float& xpos, float& ypos, int lineIdx = 0);
		static bool checkIfCorneaTrackTargetFound(bool isset = false, bool flag = false, int lineIdx = 0);

		static void writeRetinaTrackTargetDisplaced(bool flag);
		static void writeRetinaTrackTargetNotFound(bool flag);
		static void writeRetinaTrackEyelidCovered(bool flag);
		static bool checkIfRetinaTrackTargetDisplaced(bool isset = false, bool flag = false);
		static bool checkIfRetinaTrackTargetNotFound(bool isset = false, bool flag = false);
		static bool checkIfRetinaTrackEyelidCovered(bool isset = false, bool flag = false);

	private:
		struct GlobalRecordImpl;
		static std::unique_ptr<GlobalRecordImpl> d_ptr;
		static GlobalRecordImpl& getImpl(void);
	};
}
