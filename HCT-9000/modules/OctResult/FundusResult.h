#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class FundusOutput;
	class RetinaImage;
	class CorneaImage;


	class OCTRESULT_DLL_API FundusResult
	{
	public:
		FundusResult();
		FundusResult(const OctFundusMeasure& desc);
		virtual ~FundusResult();

		FundusResult(FundusResult&& rhs);
		FundusResult& operator=(FundusResult&& rhs);
		FundusResult(const FundusResult& rhs) = delete;
		FundusResult& operator=(const FundusResult& rhs) = delete;

	public:
		void setDescript(const OctFundusMeasure& desc);
		OctFundusMeasure& getDescript(void) const;

		EyeSide getEyeSide(void) const;
		bool isOD(void) const;

		FundusOutput& getFundusOutput(void) const;
		RetinaImage& getRetinaImage(void) const;
		CorneaImage& getCorneaImage(void) const;

		bool setRetinaImage(const OctRetinaImage& desc);
		bool setCorneaImage(const OctCorneaImage& desc);
		const OctRetinaImage* getRetinaImageDescript(void) const;
		const OctCorneaImage* getCorneaImageDescript(void) const;

		bool exportFiles(unsigned int imgQuality,
			std::wstring& dirPath, bool subPath = false,
			bool removeReflectionLight = false,
			const std::wstring& imageName = L"fundus",
			const std::wstring& thumbName = L"thumbnail",
			const std::wstring& retinaName = L"retina",
			const std::wstring& corneaName = L"cornea", 
			const std::wstring& frameExt = L"raw");

		bool importFiles(std::wstring& dirPath, 
			const std::wstring& imageName = L"fundus",
			const std::wstring& retinaName = L"retina",
			const std::wstring& corneaName = L"cornea",
			const std::wstring& frameExt = L"raw");

		bool exportAdjustParams(CString szVersion, float br, float ct, float ub, float vr, float cb, float gc,
			std::wstring& dirPath, const std::wstring& imageName = L"fundus");
		bool importAdjustParams(float& out_br, float& out_ct, float& out_ub, float& out_vr, float& out_cb, float& out_gc, std::wstring& dirPath, const std::wstring& imageName = L"fundus");

	protected:
		bool createExportDirectory(std::wstring& dirName);

	private:
		struct FundusResultImpl;
		std::unique_ptr<FundusResultImpl> d_ptr;
		FundusResultImpl& getImpl(void) const;
	};

	typedef std::vector<FundusResult> FundusResultList;
}
