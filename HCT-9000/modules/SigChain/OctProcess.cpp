#include "stdafx.h"
#include "OctProcess.h"
#include "FrameBuffer.h"
#include "ChainSetup.h"
#include "ChainOutput.h"

#include "CppUtil2.h"

#include <mutex>
#include <atomic>
#include <functional>

using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct OctProcess::OctProcessImpl
{
	FrameBuffer frameBuffer;

	OctProcessImpl() {
	}
};


OctProcess::OctProcess() :
	d_ptr(make_unique<OctProcessImpl>())
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::OctProcess::~OctProcess() = default;
SigChain::OctProcess::OctProcess(OctProcess && rhs) = default;
OctProcess & SigChain::OctProcess::operator=(OctProcess && rhs) = default;


OctProcess::OctProcessImpl & SigChain::OctProcess::getImpl(void) const
{
	return *d_ptr;
}


bool SigChain::OctProcess::initializeProcess(void)
{
	if (!getImpl().frameBuffer.initializeFrameBuffer()) {
		return false;
	}

	return true;
}


bool SigChain::OctProcess::startProcess(bool enface)
{
	ChainOutput::clearResultOfLastImage();

	bool result = getImpl().frameBuffer.startFrameBuffer(enface);
	return result;
}


void SigChain::OctProcess::closeProcess(void)
{
	getImpl().frameBuffer.closeFrameBuffer();
	return ;
}


void SigChain::OctProcess::releaseProcess(void)
{
	closeProcess();

	getImpl().frameBuffer.releaseFrameBuffer();
	return;
}


bool SigChain::OctProcess::isBusying(void)
{
	bool result = getImpl().frameBuffer.isBusying();
	return result;
}


bool SigChain::OctProcess::setPreviewFeature(int numImages, int numLines, int lineSize)
{
	bool result = getImpl().frameBuffer.setDataLayoutForPreview(lineSize, numLines, numImages);
	return result;
}


bool SigChain::OctProcess::setMeasureFeature(int numImages, int numLines, int lineSize)
{
	bool result = getImpl().frameBuffer.setDataLayoutForMeasure(lineSize, numLines, numImages);
	return result;
}


bool SigChain::OctProcess::setEnfaceFeature(int numImages, int numLines, int lineSize)
{
	bool result = getImpl().frameBuffer.setDataLayoutForEnface(lineSize, numLines, numImages);
	return result;
}


void SigChain::OctProcess::receiveDataForPreview(std::uint16_t * data, int numLines, int idxOfImage)
{
	getImpl().frameBuffer.insertFrameDataForPreview(data, numLines, idxOfImage);
	return;
}


void SigChain::OctProcess::receiveDataForMeasure(std::uint16_t * data, int numLines, int idxOfImage)
{
	getImpl().frameBuffer.insertFrameDataForMeasure(data, numLines, idxOfImage);
	return;
}


void SigChain::OctProcess::receiveDataForEnface(std::uint16_t * data, int numLines, int idxOfImage)
{
	getImpl().frameBuffer.insertFrameDataForEnface(data, numLines, idxOfImage);
	return;
}


void SigChain::OctProcess::notifyMeasureDataReceived(void)
{
	getImpl().frameBuffer.notifyFrameDataCompleted(false);
	return;
}


bool SigChain::OctProcess::removeInitialSaturatedLines(std::uint16_t * data, int numLines)
{
	int satured = 3;
	if (numLines < satured) {
		return false;
	}

	// The line camera intensities at the positions starting a scan line are not stable, fully satured.  
	// It appears as the vertical stripes at the start of lines on the result image.  
	for (int i = 0; i < satured; i++) {
		unsigned short* dst = data + i * LINE_CAMERA_CCD_PIXELS;
		unsigned short* src = data + satured * LINE_CAMERA_CCD_PIXELS;
		memcpy((void*)dst, (void*)src, sizeof(unsigned short)*LINE_CAMERA_CCD_PIXELS);
	}

	return true;
}

