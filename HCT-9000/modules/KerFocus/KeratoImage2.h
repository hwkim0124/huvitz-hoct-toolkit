#pragma once

#include "KerFocusDef.h"
#include "KeratoPoint.h"

#include <string>
#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace KerFocus
{
	class KERFOCUS_DLL_API KeratoImage2
	{
	public:
		KeratoImage2();
		virtual ~KeratoImage2();

		KeratoImage2(KeratoImage2&& rhs);
		KeratoImage2& operator=(KeratoImage2&& rhs);

	public:
		bool loadFile(const std::wstring& path);
		bool loadData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool isEmpty(void) const;

		int getKeratoSpotsCount(void) const;
		KerSpotVect& getKeratoSpots(void) const;

		int isFocusLength(void) const;
		bool getFocusLength(float& flen) const;

		bool isMireEllipse(void) const;
		bool getMireEllipse(float &cx, float &cy, float &m, float &n, float &a) const;
		bool getMireLength(float& mire) const;
		bool getFocusDifference(float &diff) const;
		bool getEyeCenter(float& cx, float& cy) const;
		bool getCenterOffset(float& cx, float& cy) const;

	private:
		struct KeratoImage2Impl;
		std::unique_ptr<KeratoImage2Impl> d_ptr;
		KeratoImage2Impl& getImpl(void) const;
		
		void createSampleImage(void);
		void applyDenoiseFilter(void);
		int scanKeratoSpots(void);

		bool feedKeratoSpotRect(int sx, int sy, int ex, int ey);
		int findKeratoSpotCenters(void);

		bool estimateCenterOfEye(void);
		bool isEyeFound(void) const;

		bool isEyeCenterAligned(void);
		bool assignFocusSpots(void);
		int getMireSpotsCount(void) const;
		bool measureMireRing(void);

		bool performAnalysis(void);
		void printKeratoSpots(void) const;

		CppUtil::CvImage& getImage(void) const;
		CppUtil::CvImage& getSample(void) const;
	};
}

