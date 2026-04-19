#pragma once

#include "SemtRetinaDef.h"


namespace SemtRetina
{
	class SEMTRETINA_DLL_API RetinaSegmentModel
	{
	public:
		RetinaSegmentModel();
		virtual ~RetinaSegmentModel();

    public:
        static bool initializeNetwork();
        static bool isInitialized();
        static bool runInference(const unsigned char* bscan, int width, int height, float* probs);

        static int inputWidthS(void);
		static int inputWidthM(void);
		static int inputHeightS(void);
		static int inputHeightM(void);

        static int outputWidthS(void);
        static int outputWidthM(void);
        static int outputHeightS(void);
		static int outputHeightM(void);
        static int numberOfClasses(void);
        static int outputProbMapSizeS(void);
		static int outputProbMapSizeM(void);
        
        static int classIndexVitreous(void);
        static int classIndexRnfl(void);
        static int classIndexIplOpl(void);
        static int classIndexOnl(void);
        static int classIndexRpe(void);
        static int classIndexChoroid(void);
        static int classIndexSclera(void);
        static int classIndexDiscHead(void);

    protected:
        struct RetinaSegmentModelImpl;
        static std::unique_ptr<RetinaSegmentModelImpl> d_ptr;
        static RetinaSegmentModelImpl& impl(void);
	};
}