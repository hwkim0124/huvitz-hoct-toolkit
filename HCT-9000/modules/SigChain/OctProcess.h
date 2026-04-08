#pragma once

#include "SigChainDef.h"

#include <memory>
#include <vector>


namespace SigChain
{
	class SIGCHAIN_DLL_API OctProcess
	{
	public:
		OctProcess();
		virtual ~OctProcess();

		OctProcess(OctProcess&& rhs);
		OctProcess& operator=(OctProcess&& rhs);

		// Prevent copy construction and assignment. 
		OctProcess(const OctProcess& rhs) = delete;
		OctProcess& operator=(const OctProcess& rhs) = delete;

	public:
		bool initializeProcess(void);
		bool startProcess(bool enface);
		void closeProcess(void);
		void releaseProcess(void);
		bool isBusying(void);

		bool setPreviewFeature(int numImages, int numLines, int lineSize = LINE_CAMERA_CCD_PIXELS);
		bool setMeasureFeature(int numImages, int numLines, int lineSize = LINE_CAMERA_CCD_PIXELS);
		bool setEnfaceFeature(int numImages, int numLines, int lineSize = LINE_CAMERA_CCD_PIXELS);

		void receiveDataForPreview(std::uint16_t* data, int numLines, int idxOfImage);
		void receiveDataForMeasure(std::uint16_t* data, int numLines, int idxOfImage);
		void receiveDataForEnface(std::uint16_t* data, int numLines, int idxOfImage);

		void notifyMeasureDataReceived(void);

	protected:
		bool removeInitialSaturatedLines(std::uint16_t* data, int numLines);

	private:
		struct OctProcessImpl;
		std::unique_ptr<OctProcessImpl> d_ptr;
		OctProcessImpl& getImpl(void) const;

		friend class FrameBuffer;
	};

}
