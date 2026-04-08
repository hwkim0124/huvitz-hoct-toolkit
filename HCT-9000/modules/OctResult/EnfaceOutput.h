#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class EnfaceImage;

	class OCTRESULT_DLL_API EnfaceOutput
	{
	public:
		EnfaceOutput();
		virtual ~EnfaceOutput();

		EnfaceOutput(EnfaceOutput&& rhs);
		EnfaceOutput& operator=(EnfaceOutput&& rhs);
		EnfaceOutput(const EnfaceOutput& rhs) = delete;
		EnfaceOutput& operator=(const EnfaceOutput& rhs) = delete;

	public:
		const OctEnfaceImage* getEnfaceImageDescript(void);
		bool setEnfaceImage(const OctEnfaceImage& enface, bool process = true);

		bool exportImage(const std::wstring& dirName, const std::wstring& fileName = L"enface");
		bool importImage(const std::wstring& dirName, const std::wstring& fileName = L"enface");

	protected:

	private:
		struct EnfaceOutputImpl;
		std::unique_ptr<EnfaceOutputImpl> d_ptr;
		EnfaceOutputImpl& getImpl(void) const;

		const EnfaceImage* getEnfaceImage(void);
	};

	typedef std::vector<EnfaceOutput> EnfaceOutputList;
}