#pragma once

#include <atlstr.h>
#include <vector>

namespace FileUtil {

	/// Enumerates leaf "result" directory paths under rootPath (Device / Date / Exam / Result).
	std::vector<CString> FindResultFolders(const CString& rootPath);

	/// Counts files 000.jpg, 001.jpg, ... under resultFolder; stops at the first missing file.
	int GetSequentialOctImageCount(const CString& resultFolder, int maxIndex = 1000);   

	/// Checks if a file exists in the folder.
	bool FileExistsInFolder(const CString& folderPath, const CString& fileName);

	CString getExamFolderName(const CString& fullPath);
	CString getPatientFolderName(const CString& fullPath);

	void SaveBitmapToPng(Gdiplus::Bitmap* bitmap, const CString& filePath);

} // namespace FileUtil