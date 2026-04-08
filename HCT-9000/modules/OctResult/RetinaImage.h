#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class OCTRESULT_DLL_API RetinaImage
	{
	public:
		RetinaImage();
		RetinaImage(const OctRetinaImage& desc);
		virtual ~RetinaImage();

		RetinaImage(RetinaImage&& rhs);
		RetinaImage& operator=(RetinaImage&& rhs);
		RetinaImage(const RetinaImage& rhs);
		RetinaImage& operator=(const RetinaImage& rhs);

	public:
		void setData(const OctRetinaImage& desc);
		const OctRetinaImage& getDescript(void) const;

		std::uint8_t* getBuffer(void) const;
		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;

		bool exportFile(std::wstring dirPath = L".", std::wstring imageName = L"retina");
		bool importFile(std::wstring dirPath = L".", std::wstring imageName = L"retina");

		bool importFileWithExt(std::wstring dirPath, std::wstring imageName, std::wstring imageExt);

	private:
		struct RetinaImageImpl;
		std::unique_ptr<RetinaImageImpl> d_ptr;
		RetinaImageImpl& getImpl(void) const;
	};
}

