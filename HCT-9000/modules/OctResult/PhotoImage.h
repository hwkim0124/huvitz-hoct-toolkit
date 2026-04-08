#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class OCTRESULT_DLL_API PhotoImage
	{
	public:
		PhotoImage();
		virtual ~PhotoImage();

		PhotoImage(PhotoImage&& rhs);
		PhotoImage& operator=(PhotoImage&& rhs);
		PhotoImage(const PhotoImage& rhs);
		PhotoImage& operator=(const PhotoImage& rhs);


	private:
		struct PhotoImageImpl;
		std::unique_ptr<PhotoImageImpl> d_ptr;
		PhotoImageImpl& getImpl(void) const;
	};

	typedef std::vector<PhotoImage> PhotoImageList;
}
