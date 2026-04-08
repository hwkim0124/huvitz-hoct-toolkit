#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class PhotoImage;

	class OCTRESULT_DLL_API PhotoProfile
	{
	public:
		PhotoProfile();
		virtual ~PhotoProfile();

		PhotoProfile(PhotoProfile&& rhs);
		PhotoProfile& operator=(PhotoProfile&& rhs);
		PhotoProfile(const PhotoProfile& rhs);
		PhotoProfile& operator=(const PhotoProfile& rhs);

	public:
		int addImage(PhotoImage& image);
		int addImage(PhotoImage&& image);
		PhotoImage* getImage(int index);
		int getImageCount(void) const;
		void clearImagePhotos(void);

	private:
		struct PhotoProfileImpl;
		std::unique_ptr<PhotoProfileImpl> d_ptr;
		PhotoProfileImpl& getImpl(void) const;
	};

	typedef std::vector<PhotoProfile> PhotoProfileList;
}

