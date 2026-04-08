// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                     // MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

// Opencv library should be linked before using this Opencv to Gdiplus helper class, 
// which is modified version from the following source. 
// http://stackoverflow.com/questions/24725155/opencv-tesseract-how-to-replace-libpng-libtiff-etc-with-gdi-bitmap-load-in

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// Add USE_OPENCV_IN_CPPUTIL macro to the preprocessor definitions of target project, 
// or
#define USE_OPENCV_IN_CPPUTIL
#ifdef USE_OPENCV_IN_CPPUTIL 

// IMPORTANT:
// This must be included AFTER gdiplus !!
// (OpenCV #undefine's min(), max())
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>


//#define BOOST_ASIO_SEPARATE_COMPILATION
// #define BOOST_REGEX_NO_LIB
//#define BOOST_DATE_TIME_NO_LIB

// All boost library should be linked, only BOOST_LOG_DYN_LINK not work!
#define BOOST_ALL_DYN_LINK 1



#endif 