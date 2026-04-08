#pragma once

#include <cstdint>

typedef struct {
	std::uint16_t retinaIR;
	std::uint16_t retinaIR_bright;
	std::uint16_t corneaIR;
	std::uint16_t keratoRing;
	std::uint16_t keratoFocus;
	std::uint16_t splitFocus;
	std::uint16_t externLed;
	std::uint16_t workingDot1;
	std::uint16_t workingDot2;
	std::uint8_t reserved[256];
} LightLeds_t ;

typedef struct {
	float again;
	float dgain;
	std::uint8_t reserved[64];
} CameraProfile_t ;

typedef struct {
	float again;
	float dgain;
	float again2;
	float dgain2;
	float again3;
	float dgain3;
	std::uint8_t reserved[64];
} IrCameraPreset_t ;

typedef struct {
	IrCameraPreset_t retinaIR;
	CameraProfile_t corneaIR;
	CameraProfile_t fundusColor;
	std::uint8_t reserved[256];
} SystemCameras_t ;

typedef struct {
	float params[2];
	std::uint32_t status;
	std::uint8_t reserved[64];
} WhiteBalance_t ;

typedef struct {
	std::uint16_t coordXs[4];
	std::uint16_t coordYs[4];
	std::uint16_t coordXs_FILR[7];
	std::uint16_t coordYs_FILR[7];
	std::uint32_t status;
	std::uint8_t reserved[36];     //  reserved[64] - Values before adding FILR Parameter
} LevelCorrection_t;

typedef struct {
	float ratio;
	std::uint32_t status;
	std::uint8_t reserved[64];
} RadialCorrection_t;

typedef struct {
	float bright;
	float contrast;
	float ub;
	float vr;
	std::uint8_t reserved[64];
} AutoEnhancement_t;

typedef struct {
	std::uint16_t kernelSize;
	std::uint8_t reserved[64];
} EdgeEnhancement_t;

typedef struct {
	std::uint16_t center[2];
	std::uint16_t radius;
	std::uint32_t status;
	std::uint8_t reserved[64];
} ColorROI_t;

typedef struct {
	std::uint16_t light;
	std::uint16_t shadow;
	std::uint16_t outer;
	std::uint32_t status;
	std::uint8_t reserved[64];
} RemoveReflectionLight_t;

typedef struct {
	WhiteBalance_t whiteBalance;
	RadialCorrection_t radialCorr;
	LevelCorrection_t levelCorr;
	AutoEnhancement_t autoEnhance;
	EdgeEnhancement_t edgeEnhance;
	ColorROI_t colorRoi;
	RemoveReflectionLight_t removeReflectionlight;
	std::uint8_t reserved[180]; //reserved[256];
} FundusColor_t;

typedef struct {
	std::uint16_t levelNorm;
	std::uint16_t levelMin;
	std::uint16_t levelMax;
	std::uint16_t focusInt[3];
	std::uint16_t sizeBase[3];
	std::uint16_t sizeHigh[3];
	std::uint16_t defLevel[3];
	std::uint16_t splitFocus[3];
	std::uint16_t retinaIr[3];
	std::uint16_t workDot1[3];
	std::uint16_t workDot2[3];
	std::uint16_t status;
	std::uint8_t reserved[40];
} AutoFlash_t;

typedef struct {
	AutoFlash_t autoFlash;
	std::uint8_t reserved[64];
} FundusCapture_t ;

typedef struct {
	std::uint32_t status;
	float clipLimit;
	std::uint8_t reserved[60];
} ImageEnhance_t ;

typedef struct {
	std::uint16_t center[2];
	std::uint8_t reserved[64];
} SplitFocus_t ;

typedef struct {
	std::uint16_t center[2];
	std::uint16_t radius;
	std::uint32_t status;
	std::uint8_t reserved[66];
} RetinaROI_t ;

typedef struct {
	std::uint16_t radius;
	std::uint32_t status;
	std::uint16_t center[2];
	std::uint8_t reserved[10];
} RetinaCenterMask_t ;

typedef struct {
	ImageEnhance_t imageEnhance;
	SplitFocus_t splitFocus;
	RetinaROI_t retinaRoi;
	RetinaCenterMask_t centerMask;
	std::uint8_t reserved[160]; // [168];
} RetinaImage_t ;


typedef struct {
	std::uint16_t pixelsPerMM;
	float smallPupilSize;
	std::uint8_t reserved[66];
} CorneaImage_t ;

typedef struct {
	std::int16_t center[2];
	std::int16_t fundus[2];
	std::int16_t scanDisk[2];
	std::int16_t leftSide[3][2];
	std::int16_t rightSide[3][2];
	std::uint8_t reserved[64];
} LcdFixation_t;

typedef struct {
	LcdFixation_t od;
	LcdFixation_t os;

	std::uint16_t bright;
	std::uint16_t blink;
	std::uint16_t onTime;
	std::uint16_t period;
	std::uint32_t status;
	std::uint16_t type;
	std::uint8_t reserved[254];
} InternalFixation_t;

typedef struct {
	float scaleX;
	float scaleY;
	float offsetX;
	float offsetY;
	std::uint8_t reserved[64];
} PatternRange_t ;

typedef struct {
	std::uint32_t upperRangeSize;
	std::uint32_t lowerRangeSize;
	std::uint8_t reserved[64];
} ReferenceRange_t ;

typedef struct {
	PatternRange_t retina[3];
	PatternRange_t cornea[3];
	PatternRange_t topography[3];
	std::uint8_t reserved[16]; //reserved[256] - Values before adding topography;
} PatternSettings_t;

typedef struct {
	std::uint8_t serialNo[16];
//	std::uint32_t octSoloVersion;
	std::uint8_t modelType;
	std::uint8_t octGrabberType;
	std::uint8_t reserved[66]; // reserved[64] - Values before adding modelType
} SystemInfo_t ;


typedef struct {
	std::uint8_t octLineCameraMode;
	std::uint8_t sldFaultDetection;
	std::uint8_t retinaTrackingSpeed;
	std::uint8_t temp;
	std::uint16_t sldFaultTimer;
	std::uint16_t sldFaultThreshold;
	std::uint16_t triggerForePadd[3];
	std::uint16_t triggerPostPadd[3];
	std::uint8_t reserved[44];
} SystemOptions_t;

typedef struct {
	SystemInfo_t sysInfo;
	SystemOptions_t sysOptions;
	std::uint8_t reserved[192];
} SystemSettings_t ;

typedef struct {
	LightLeds_t lightLeds;
	SystemCameras_t cameras;
	FundusColor_t fundusColor;
	FundusCapture_t fundusCapture;
	RetinaImage_t retinaImage;
	CorneaImage_t corneaImage;
	InternalFixation_t internalFix;
	PatternSettings_t patterns;
	ReferenceRange_t referRange;
	SystemSettings_t systemSet;
	std::uint8_t reserved[2048]; 
	std::uint64_t chksum; //8 bytes.SysSerial~

} SysCal2_st;