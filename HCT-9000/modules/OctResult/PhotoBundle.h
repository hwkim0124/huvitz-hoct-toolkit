#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class PhotoProfile;

	class OCTRESULT_DLL_API PhotoBundle
	{
	public:
		PhotoBundle();
		virtual ~PhotoBundle();

		PhotoBundle(PhotoBundle&& rhs);
		PhotoBundle& operator=(PhotoBundle&& rhs);
		PhotoBundle(const PhotoBundle& rhs);
		PhotoBundle& operator=(const PhotoBundle& rhs);

	public:
		int addSection(PhotoProfile& section);
		int addSection(PhotoProfile&& section);
		PhotoProfile* getSection(int index);
		PhotoProfile* getSectionLast(void);
		PhotoProfile* makeSection(int index);
		int getSectionCount(void) const;
		void clearAllSections(void);

	private:
		struct PhotoBundleImpl;
		std::unique_ptr<PhotoBundleImpl> d_ptr;
		PhotoBundleImpl& getImpl(void) const;
	};

}