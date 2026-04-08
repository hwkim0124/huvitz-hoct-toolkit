#pragma once

#include "OctResultDef.h"

#include <memory>


namespace OctResult
{
	class OCTRESULT_DLL_API ScanProfile
	{
	public:
		ScanProfile();
		ScanProfile(EyeSide side, PatternDomain domain, PatternType type);
		virtual ~ScanProfile();

		ScanProfile(ScanProfile&& rhs);
		ScanProfile& operator=(ScanProfile&& rhs);
		ScanProfile(const ScanProfile& rhs);
		ScanProfile& operator=(const ScanProfile& rhs);

	public:
		EyeSide eyeSide(void) const;
		PatternDomain patternDomain(void) const;
		PatternType patternType(void) const;

		float scanAngle(void) const;
		float scanRange(void) const;
		float scanOffsetX(void) const;
		float scanOffsetY(void) const;
		int numAverage(void) const;

	private:
		struct ScanProfileImpl;
		std::unique_ptr<ScanProfileImpl> d_ptr;
		ScanProfileImpl& getImpl(void) const;
	};
}
