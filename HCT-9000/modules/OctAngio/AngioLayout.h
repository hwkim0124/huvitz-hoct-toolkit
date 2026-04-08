#pragma once

#include "OctAngioDef.h"

#include <memory>


namespace OctAngio
{
	class OCTANGIO_DLL_API AngioLayout
	{
	public:
		AngioLayout();
		AngioLayout(int lines, int points, int repeats, bool vertical);
		virtual ~AngioLayout();

		AngioLayout(AngioLayout&& rhs);
		AngioLayout& operator=(AngioLayout&& rhs);
		AngioLayout(const AngioLayout& rhs);
		AngioLayout& operator=(const AngioLayout& rhs);

	public:
		void setupLayout(int lines, int points, int repeats, bool vertical);
		void setupRange(float rangeX, float rangeY, float centerX, float centerY, bool isDisc);

		int numberOfLines(void) const;
		int numberOfPoints(void) const;
		int numberOfRepeats(void) const;

		bool isVerticalScan(void) const;
		bool isDiscScan(void) const;
		bool isMacularScan(void) const;
		bool isFoveaCenter(void) const;

		float scanRangeX(void) const;
		float scanRangeY(void) const;
		float foveaCenterX(void) const;
		float foveaCenterY(void) const;

		bool getFoveaCenterInPixel(int& cx, int& cy) const;
		bool getFoveaRadiusInPixel(int& size1, int& size2, float radius=0.25f) const;

		int getWidth(void) const;
		int getHeight(void) const;
		int getSize(void) const;

		float getPixelSizeX(void) const;
		float getPixelSizeY(void) const;

	private:
		struct AngioLayoutImpl;
		std::unique_ptr<AngioLayoutImpl> d_ptr;
		AngioLayoutImpl& getImpl(void) const;
	};
}


