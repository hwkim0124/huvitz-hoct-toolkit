#include "stdafx.h"
#include "KeratoFocus.h"
#include "KeratoSetup.h"
#include "KeratoImage.h"
#include "KeratoImage2.h"
#include "KeratoPoint.h"
#include "KerSpot.h"

using namespace KerFocus;
using namespace std;


struct KeratoFocus::KeratoFocusImpl
{
	KeratoSetup kerSetup;
	KeratoImage kerImage;
	KeratoImage2 kerImage2;

	KeratoFocusImpl()
	{
	}
};


KeratoFocus::KeratoFocus() 
	: d_ptr(make_unique<KeratoFocusImpl>())
{
}


KerFocus::KeratoFocus::~KeratoFocus() = default;
KerFocus::KeratoFocus::KeratoFocus(KeratoFocus && rhs) = default;
KeratoFocus & KerFocus::KeratoFocus::operator=(KeratoFocus && rhs) = default;


KeratoSetup & KerFocus::KeratoFocus::getKeratoSetup(void) const
{
	return d_ptr->kerSetup;
}


void KerFocus::KeratoFocus::setKeratoSetup(KeratoSetup&& setup)
{
	getKeratoSetup() = std::move(setup);
	return;
}


KeratoImage & KerFocus::KeratoFocus::getKeratoImage(void) const
{
	return d_ptr->kerImage;
}


KeratoImage2 & KerFocus::KeratoFocus::getKeratoImage2(void) const
{
	return d_ptr->kerImage2;
}


void KerFocus::KeratoFocus::setKeratoImage(KeratoImage && image)
{
	getKeratoImage() = std::move(image);
	return;
}


void KerFocus::KeratoFocus::setKeratoImage2(KeratoImage2 && image)
{
	getKeratoImage2() = std::move(image);
	return;
}


bool KerFocus::KeratoFocus::setIRImage(const std::wstring & path, bool process)
{
	bool ret = getKeratoImage().loadFile(path);
	return ret;
}


bool KerFocus::KeratoFocus::loadImage(const unsigned char * bits, int width, int height, int padding)
{
	bool res = getKeratoImage().loadData(bits, width, height, padding);
	return res;
}


bool KerFocus::KeratoFocus::updateFocusStepDataFromImage(int diopt, int istep)
{
	KeratoSetup& setup = getKeratoSetup();
	KeratoImage& image = getKeratoImage();
	if (setup.updateFocusStepData(diopt, istep, image)) {
		return true;
	}
	return false;
}


int KerFocus::KeratoFocus::countOfMireSpots(void) const
{
	return getKeratoImage().getCountOfMireSpots();
}


int KerFocus::KeratoFocus::countOfFocusSpots(void) const
{
	return getKeratoImage().getCountOfFocusSpots();
}


bool KerFocus::KeratoFocus::getMireSpot(int index, float & cx, float & cy, int & size, int & thresh, CRect & rect) const
{
	if (index >= 0 && index < countOfMireSpots()) {
		KerSpot2<float> spot;
		if (getKeratoImage().getMireSpot(index, spot)) {
			cx = spot.center().x_;
			cy = spot.center().y_;
			size = spot.size_;
			thresh = spot.thresh_;
			rect = spot.window().getCRect();
			return true;
		}
	}
	return false;
}


bool KerFocus::KeratoFocus::getFocusSpot(int index, float & cx, float & cy, int & size, int & thresh, CRect & rect) const
{
	if (index >= 0 && index < countOfFocusSpots()) {
		KerSpot2<float> spot;
		if (getKeratoImage().getFocusSpot(index, spot)) {
			cx = spot.center().x_;
			cy = spot.center().y_;
			size = spot.size_;
			thresh = spot.thresh_;
			rect = spot.window().getCRect();
			return true;
		}
	}
	return false;
}


bool KerFocus::KeratoFocus::getMireCenter(float & cx, float & cy) const
{
	KeratoImage& image = getKeratoImage();
	return image.getEyeCenter(cx, cy);
}


bool KerFocus::KeratoFocus::getCenterOffset(float & cx, float & cy) const
{
	KeratoImage& image = getKeratoImage();

	return image.getCenterOffset(cx, cy);
}


bool KerFocus::KeratoFocus::getFocusDistance(float& dist) const
{
	// return getKeratoImage().getFocusDifference(dist);

	float mlen, flen;
	if (getKeratoImage().getMireLength(mlen) && getKeratoImage().getFocusLength(flen)) {
		return getKeratoSetup().getFocusDistance(mlen, flen, dist);
	}
	return false;
}


bool KerFocus::KeratoFocus::getFocusDifference(float & diff) const
{
	return getKeratoImage().getFocusDifference(diff);
}


bool KerFocus::KeratoFocus::getFocusLength(float & flen) const
{
	return getKeratoImage().getFocusLength(flen);
}


bool KerFocus::KeratoFocus::getMireLength(float & mlen) const
{
	return getKeratoImage().getMireLength(mlen);
}


bool KerFocus::KeratoFocus::getMireEllipse(float& cx, float& cy, float & m, float & n, float & a) const
{
	if (getKeratoImage().getMireEllipse(cx, cy, m, n, a)) {
		return true;
	}
	return false;
}


float KerFocus::KeratoFocus::getLightExtent(void) const
{
	return getKeratoImage().getLightExtent();
}


int KerFocus::KeratoFocus::getPupilStartX(void) const
{
	return getKeratoImage().getPupilStartX();
}


int KerFocus::KeratoFocus::getPupilCloseX(void) const
{
	return getKeratoImage().getPupilCloseX();
}


int KerFocus::KeratoFocus::getPupilWidth(void) const
{
	return (getKeratoImage().getPupilCloseX() - getKeratoImage().getPupilStartX() + 1);
}


KeratoFocus::KeratoFocusImpl & KerFocus::KeratoFocus::getImpl(void)
{
	return *d_ptr;
}


