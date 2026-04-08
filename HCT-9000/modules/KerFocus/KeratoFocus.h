#pragma once

#include "KerFocusDef.h"

#include <memory>
#include <string>

namespace KerFocus
{
	class KeratoSetup;
	class KeratoImage;
	class KeratoImage2;

	class KERFOCUS_DLL_API KeratoFocus
	{
	public:
		KeratoFocus();
		virtual ~KeratoFocus();

		KeratoFocus(KeratoFocus&& rhs);
		KeratoFocus& operator=(KeratoFocus&& rhs);

	public:
		KeratoSetup& getKeratoSetup(void) const;
		void setKeratoSetup(KeratoSetup&& setup);

		KeratoImage& getKeratoImage(void) const;
		KeratoImage2& getKeratoImage2(void) const;
		void setKeratoImage(KeratoImage&& image);
		void setKeratoImage2(KeratoImage2&& image);

		bool loadImage(const unsigned char* bits, int width, int height, int padding = 0);
		bool updateFocusStepDataFromImage(int diopt, int istep);

		int countOfMireSpots(void) const;
		int countOfFocusSpots(void) const;
		bool getMireSpot(int index, float& cx, float& cy, int& size, int& thresh, CRect& rect) const;
		bool getFocusSpot(int index, float& cx, float& cy, int& size, int& thresh, CRect& rect) const;

		bool getMireCenter(float& cx, float& cy) const;
		bool getCenterOffset(float& cx, float& cy) const;
		bool getFocusDistance(float& dist) const;
		bool getFocusDifference(float& diff) const;
		bool getFocusLength(float& flen) const;
		bool getMireLength(float& mlen) const;
		bool getMireEllipse(float& cx, float& cy, float& m, float& n, float& a) const;

		float getLightExtent(void) const;
		int getPupilStartX(void) const;
		int getPupilCloseX(void) const;
		int getPupilWidth(void) const;

		bool setIRImage(const std::wstring& path, bool process = true);

	private:
		struct KeratoFocusImpl;
		std::unique_ptr<KeratoFocusImpl> d_ptr;
		KeratoFocusImpl& getImpl(void);
	};
}
