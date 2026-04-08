#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class PhotoProfile;

	class OCTRESULT_DLL_API CameraResult
	{
	public:
		CameraResult();
		virtual ~CameraResult();

		CameraResult(CameraResult&& rhs);
		CameraResult& operator=(CameraResult&& rhs);
		CameraResult(const CameraResult& rhs);
		CameraResult& operator=(const CameraResult& rhs);

	public:
		PhotoProfile& getPhotoProfile(void) const;
		bool isEmpty(void) const;

	private:
		struct CameraResultImpl;
		std::unique_ptr<CameraResultImpl> d_ptr;
		CameraResultImpl& getImpl(void) const;
	};

	typedef std::vector<CameraResult> CameraResultList;
}
