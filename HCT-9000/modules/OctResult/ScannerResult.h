#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace OctResult
{
	class EnfaceOutput;
	class PreviewOutput;
	class PatternOutput2;
	class RetinaImage;
	class CorneaImage;


	class OCTRESULT_DLL_API ScannerResult
	{
	public:
		ScannerResult();
		ScannerResult(const OctScanMeasure& desc);
		virtual ~ScannerResult();

		ScannerResult(ScannerResult&& rhs);
		ScannerResult& operator=(ScannerResult&& rhs);
		ScannerResult(const ScannerResult& rhs) = delete;
		ScannerResult& operator=(const ScannerResult& rhs) = delete;

	public:
		void setDescript(const OctScanMeasure& desc);
		OctScanMeasure& getDescript(void) const;

		EyeSide getEyeSide(void) const;
		bool isOD(void) const;

		PreviewOutput& getPreviewOutput(void) const;
		PatternOutput2& getPatternOutput(void) const;
		EnfaceOutput& getEnfaceOutput(void) const;

		RetinaImage& getRetinaImage(void) const;
		CorneaImage& getCorneaImage(void) const;

		bool setRetinaImage(const OctRetinaImage& desc);
		bool setCorneaImage(const OctCorneaImage& desc);
		const OctRetinaImage* getRetinaImageDescript(void) const;
		const OctCorneaImage* getCorneaImageDescript(void) const;

		bool exportFiles(std::wstring& dirPath, bool subPath = false,
						const std::wstring& imagePrefix = L"",
						const std::wstring& enfaceName = L"enface",
						const std::wstring& previewName = L"preview",
						const std::wstring& retinaName = L"retina",
						const std::wstring& corneaName = L"cornea");
		bool importFiles(std::wstring& dirPath, 
						int numImages = 0,
						const std::wstring& imagePrefix = L"",
						const std::wstring& enfaceName = L"enface",
						const std::wstring& previewName = L"preview",
						const std::wstring& retinaName = L"retina",
						const std::wstring& corneaName = L"cornea");

	protected:
		bool createExportDirectory(std::wstring& dirName);

	private:
		struct ScannerResultImpl;
		std::unique_ptr<ScannerResultImpl> d_ptr;
		ScannerResultImpl& getImpl(void) const;
	};

	typedef std::vector<ScannerResult> ScannerResultList;
	// typedef std::multimap<EyeSide, ScannerResult> ScannerResultList;
}
