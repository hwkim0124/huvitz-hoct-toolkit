// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OCTIMPORT_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OCTIMPORT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef OCTIMPORT_EXPORTS
#define OCTIMPORT_API __declspec(dllexport)
#else
#define OCTIMPORT_API __declspec(dllimport)
#endif

#include <atlstr.h>

namespace octimport {
	// This class is exported from the OctImport.dll
	class OCTIMPORT_API COctImport {
	public:
		COctImport();
		COctImport(std::wstring path);
		~COctImport();

		bool Import();
		void SetDelete(int del);
		void SetUpdate(int upd);


	private:
		void RecurseDirectory(CString root);
		CString ImportPatient(CString csvFilePath);
		BOOL ImportScan(CString patientID, CString dirPath);
		// TODO: add your methods here.
	private:
		int isUpdate;
		int isDelete;
		CString mImportPath;
	};
}
