#pragma once

#include "KerFocusDef.h"
#include "KerSpot.h"

#include <string>
#include <memory>
#include <vector>

namespace CppUtil {
	class CvImage;
}


namespace KerFocus
{
	class KERFOCUS_DLL_API KeratoImage
	{
	public:
		KeratoImage();
		KeratoImage(const std::wstring& path);
		virtual ~KeratoImage();

		KeratoImage(KeratoImage&& rhs);
		KeratoImage& operator=(KeratoImage&& rhs);

	public:
		bool loadFile(const std::wstring& path);
		bool loadData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool isEmpty(void) const;

		bool isFocusLength(void) const;
		bool getFocusLength(float& flen) const;

		bool isMireEllipse(void) const;
		bool getMireEllipse(float & cx, float & cy, float & m, float & n, float & a) const;
		bool getMireLength(float & mire) const;
		bool getFocusDifference(float & diff) const;

		bool getMireSpot(int index, KerSpot2<float>& spot) const;
		bool getFocusSpot(int index, KerSpot2<float>& spot) const;
		int getCountOfMireSpots(void) const;
		int getCountOfFocusSpots(void) const;

		bool isEyeFound(void) const;
		bool getEyeCenter(float& cx, float& cy) const;
		bool getCenterOffset(float& cx, float& cy) const;

		float getLightExtent(void) const;
		int getPupilStartX(void) const;
		int getPupilCloseX(void) const;

	private:
		struct KeratoImageImpl;
		std::unique_ptr<KeratoImageImpl> d_ptr;
		KeratoImageImpl& getImpl(void) const;

		void initialize(void);
		bool performAnalysis(void);

		bool createSampleImage(void);
		void makeHistogram(void);
		void decideThresholds(void);
		bool assignPupilCenter(void);
		bool measurePupilDiameter(void);
		bool measureLightingArea(void);
		bool checkIfModelEye(void);

		int assignSpotWindows(void);
		void expandSpotWindows(int sx, int sy, int ex, int ey);
		int filterSpotWindows(void);

		int findSpotCenters(void);
		void sortSpotsByDistance(void);
		int matchHorzPairsOfSpots(void);
		int assignMireSpotsOnCenter(void);
		int assignMireSpotsOnLeft(void);
		int assignMireSpotsOnRight(void);
		int assignFocusSpots(void);

		bool calculateMireRing(void);
		bool calculateFocusLength(void);
		bool decideEyeCenter(void);

		int countMireSpotsOnCenter(void) const;
		int countMireSpotsOnLeft(void) const;
		int countMireSpotsOnRight(void) const;
		int countMireSpots(void) const;
		int countFocusSpots(void) const;
		int countAllSpots(void) const;

		CppUtil::CvImage& getImage(void) const;
		CppUtil::CvImage& getSample(void) const;
	};
}
