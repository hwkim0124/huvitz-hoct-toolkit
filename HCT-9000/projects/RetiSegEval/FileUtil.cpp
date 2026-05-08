#include "stdafx.h"

#include "FileUtil.h"

#include <cstdlib>

namespace {

bool IsSubdirectoryEntry(const WIN32_FIND_DATA& fd)
{
    return (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0
        && _tcscmp(fd.cFileName, _T(".")) != 0
        && _tcscmp(fd.cFileName, _T("..")) != 0;
}

/// Locates the GDI+ encoder CLSID for a MIME type (e.g. L"image/png").
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT num = 0;
    UINT size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;

    Gdiplus::ImageCodecInfo* pImageCodecInfo =
        static_cast<Gdiplus::ImageCodecInfo*>(malloc(size));
    if (pImageCodecInfo == nullptr)
        return -1;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return static_cast<int>(j);
        }
    }
    free(pImageCodecInfo);
    return -1;
}

} // namespace

namespace FileUtil {

    std::vector<CString> FindResultFolders(const CString& rootPath)
    {
        std::vector<CString> resultFolders;

        CString searchDevice = rootPath + _T("\\*");
        WIN32_FIND_DATA findDevice;
        HANDLE hDevice = FindFirstFile(searchDevice, &findDevice);
        if (hDevice == INVALID_HANDLE_VALUE)
            return resultFolders;

        do {
            if (!IsSubdirectoryEntry(findDevice))
                continue;

            CString deviceFolder = rootPath + _T("\\") + findDevice.cFileName;
            CString searchDate = deviceFolder + _T("\\*");
            WIN32_FIND_DATA findDate;
            HANDLE hDate = FindFirstFile(searchDate, &findDate);
            if (hDate == INVALID_HANDLE_VALUE)
                continue;

            do {
                if (!IsSubdirectoryEntry(findDate))
                    continue;

                CString dateFolder = deviceFolder + _T("\\") + findDate.cFileName;
                CString searchExam = dateFolder + _T("\\*");
                WIN32_FIND_DATA findExam;
                HANDLE hExam = FindFirstFile(searchExam, &findExam);
                if (hExam == INVALID_HANDLE_VALUE)
                    continue;

                do {
                    if (!IsSubdirectoryEntry(findExam))
                        continue;

                    CString examFolder = dateFolder + _T("\\") + findExam.cFileName;
                    CString searchResult = examFolder + _T("\\*");
                    WIN32_FIND_DATA findResult;
                    HANDLE hResult = FindFirstFile(searchResult, &findResult);
                    if (hResult == INVALID_HANDLE_VALUE)
                        continue;

                    do {
                        if (!IsSubdirectoryEntry(findResult))
                            continue;

                        CString resultFolder = examFolder + _T("\\") + findResult.cFileName;
                        resultFolders.push_back(resultFolder);
                    } while (FindNextFile(hResult, &findResult));

                    FindClose(hResult);
                } while (FindNextFile(hExam, &findExam));

                FindClose(hExam);
            } while (FindNextFile(hDate, &findDate));

            FindClose(hDate);
        } while (FindNextFile(hDevice, &findDevice));

        FindClose(hDevice);
        return resultFolders;
    }

    bool FileExistsInFolder(const CString& folderPath, const CString& fileName)
    {
        CString filePath = folderPath + _T("\\") + fileName;
        DWORD fileAttr = GetFileAttributes(filePath);
        return fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY);
    }

    CString getExamFolderName(const CString& fullPath)
    {
        CString name = _T("");
        CString path = fullPath;
        if (path.Right(1) == _T("\\")) {
            path = path.Left(path.GetLength() - 1);
        }
        int pos = path.ReverseFind(_T('\\'));
        if (pos != -1) {
            name = path.Mid(pos + 1);
        }
        return name;
    }

    CString getPatientFolderName(const CString& fullPath)
    {
        CString name = _T("");
        CString path = fullPath;
        if (path.Right(1) == _T("\\")) {
            path = path.Left(path.GetLength() - 1);
        }
        int pos = path.ReverseFind(_T('\\'));
        if (pos != -1) {
            path = path.Left(pos);
            pos = path.ReverseFind(_T('\\'));
            if (pos != -1) {
                name = path.Mid(pos + 1);
            }
        }
        return name;
    }

    int GetSequentialOctImageCount(const CString& resultFolder, int maxIndex)
    {
        int count = 0;
        for (int i = 0; i < maxIndex; ++i) {
            CString fileName;
            fileName.Format(_T("%03d.jpg"), i);
            CString filePath = resultFolder + _T("\\") + fileName;
            DWORD fileAttr = GetFileAttributes(filePath);
            if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY))
                ++count;
            else
                break;
        }
        return count;
    }

    void SaveBitmapToPng(Gdiplus::Bitmap* bitmap, const CString& filePath)
    {
        if (bitmap == nullptr)
            return;

        CLSID pngClsid;
        if (GetEncoderClsid(L"image/png", &pngClsid) < 0)
            return;

        // GDI+ Save expects a wide-character path.
        const WCHAR* wszPath = filePath.GetString();
        bitmap->Save(wszPath, &pngClsid, nullptr);
    }

} // namespace FileUtil
