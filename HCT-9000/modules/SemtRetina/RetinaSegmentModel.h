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

        static int inputWidth(void);
        static int inputHeight(void);
        static int outputWidth(void);
        static int outputHeight(void);
        static int numberOfClasses(void);
        static int outputProbMapSize(void);
        
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