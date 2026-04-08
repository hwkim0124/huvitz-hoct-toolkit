#pragma once

#include "RetFocusDef.h"
#include "ScanOptimizer.h"

#include <memory>
#include <string>

namespace OctDevice
{
	class MainBoard;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API ScanAutoMeasure : public ScanOptimizer
	{
	public:
		ScanAutoMeasure();
		virtual ~ScanAutoMeasure();

		enum FocusTarget {
			RETINA,
			CORNEA,
			LENS_FRONT,
			LENS_BACK, 
			AXIAL_LENGTH, 
			LENS_THICKNESS
		};


	public:
		static bool initiateAxialLength(OctDevice::MainBoard * board, bool reset = true);
		static bool initialize(OctDevice::MainBoard* board, FocusTarget target, bool compLens = false);
		static bool isInitialized(void);
		static void clearResult(void);

		static bool isCompensationLensMode(void);

		static bool recordPositionOfRetina(bool confirm=false);
		static bool recordPositionOfCornea(bool confirm = false);
		static bool recordPositionOfLensFront(bool confirm = false);
		static bool recordPositionOfLensBack(bool confirm = false);
		static bool makeDelayToProceed(int count = 5);
		static bool checkIfTargetExist(void);

		static bool moveToStartCorneaCapture(bool first = true, bool check = false);
		static bool moveToStartRetinaCapture(bool first = true, bool check = false);
		static bool moveToStartLensFrontCapture(bool first = true, bool meye = false);
		static bool moveToStartLensBackCapture(bool first = true);
		static bool returnToRetinaPosition(void);

		static bool isTargetRetinaFound(void);
		static bool isTargetCorneaFound(void);
		static bool isTargetLensFrontFound(void);
		static bool isTargetLensBackFound(void);

		static int getRetinaPosition(void);
		static int getCorneaPosition(void);
		static int getLensFrontPosition(void);
		static int getLensBackPosition(void);
		static void setCorneaPosition(int pos);
		static void setLensFrontPosition(int pos);
		static void setLensBackPosition(int pos);

		static float getRetinaDiopter(void);
		static float getCorneaDiopter(void);
		static float getLensFrontDiopter(void);
		static float getLensBackDiopter(void);
		static void setCorneaDiopter(float diopt);
		static void setLensFrontDiopter(float diopt);
		static void setLensBackDiopter(float diopt);

		static float getAxialLength(void);

		static bool isTargetFound(void);
		static bool isTargetQuality(bool isLensBack = false);
		static bool isTargetConfirmed(void);
		static bool renewQualityIndex(bool next = false);


	private:
		struct ScanAutoMeasureImpl;
		static std::unique_ptr<ScanAutoMeasureImpl> d_ptr;
		static ScanAutoMeasureImpl& getImpl(void);

		static OctDevice::MainBoard* getMainBoard(void);
	};
}

