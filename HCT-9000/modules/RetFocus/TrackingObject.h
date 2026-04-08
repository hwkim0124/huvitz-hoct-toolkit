#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>


namespace CppUtil {
	class CvImage;
}

namespace RetFocus 
{
    class RETFOCUS_DLL_API TrackingObject
    {
    public:
        TrackingObject();
        virtual ~TrackingObject();

        TrackingObject(TrackingObject&& rhs);
        TrackingObject& operator=(TrackingObject&& rhs);

    public:
        void initializeTrackingObject(void);
        bool assignTrackingFrame(bool isOD, bool isDisc, const CppUtil::CvImage& frame);
        bool assessTrackingFrame(void) const;
        bool detectTargetInFrame(const CppUtil::CvImage& target) const;

        bool checkEyelidCovered(void) const;
        bool verifyObjectWithinBox(void) const;

        CppUtil::CvImage getObjectPatch(void) const;
        float objectCenterX(void) const;
        float objectCenterY(void) const;

        CppUtil::CvImage getTargetPatch(void) const;
        float targetMatchScore(void) const;
        float targetMatchedX(void) const;
        float targetMatchedY(void) const;
		void setTargetMatched(float x, float y, float score);

        float retinaPatchMean(void) const;
        float retinaPatchStdev(void) const;
        float objectPatchMean(void) const;
        float objectPatchStdev(void) const;
        float retinaUpsideMean(void) const;
        float retinaUpsideStdev(void) const;
        float retinaDownsideMean(void) const;
        float retinaDownsideStdev(void) const;

    protected:
        void getRetinaRegion(bool isOD, bool isDisc, int& x, int& y, int& w, int& h) const;
        void getRoiRegion(bool isOD, bool isDisc, int& x, int& y, int& w, int& h) const;
        void getUpsideRegion(int& x, int& y, int& w, int& h) const;
        void getDownsideRegion(int& x, int& y, int& w, int& h) const;

        bool estimateRetinaRegion(void) const; 
        bool estimateObjectRegion(void) const;


    private:
        struct TrackingObjectImpl;
        std::unique_ptr<TrackingObjectImpl> d_ptr;
        TrackingObjectImpl& impl(void) const;
    };
}
