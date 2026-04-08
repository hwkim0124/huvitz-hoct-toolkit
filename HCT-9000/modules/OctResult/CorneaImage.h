#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class OCTRESULT_DLL_API CorneaImage
	{
	public:
		CorneaImage();
		CorneaImage(const OctCorneaImage& desc);
		virtual ~CorneaImage();

		CorneaImage(CorneaImage&& rhs);
		CorneaImage& operator=(CorneaImage&& rhs);
		CorneaImage(const CorneaImage& rhs);
		CorneaImage& operator=(const CorneaImage& rhs);

	public:
		void setData(const OctCorneaImage& desc);
		const OctCorneaImage& getDescript(void) const;

		std::uint8_t* getBuffer(void) const;
		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;
		
		bool exportFile(std::wstring dirPath, std::wstring imageName);
		bool importFile(std::wstring dirPath, std::wstring imageName);

		bool importFileWithExt(std::wstring dirPath, std::wstring imageName, std::wstring imageExt);

	private:
		struct CorneaImageImpl;
		std::unique_ptr<CorneaImageImpl> d_ptr;
		CorneaImageImpl& getImpl(void) const;
	};
}
