#pragma once
#include <vector>
#include <atlstr.h>

#ifdef OCTIMPORT_DATA_EXPORTS
#define OCTIMPORT_DATA_API __declspec(dllexport)
#else
#define OCTIMPORT_DATA_API __declspec(dllimport)
#endif

namespace octimport {
	class OCTIMPORT_DATA_API OctImportData
	{
	public:
		OctImportData();
		OctImportData(CString csvFilePath);
		~OctImportData();

		void ReadCSV(CString csvFilePath);
		CString GetValue(int col, int row);
		int GetColumnCount(int row);
		int GetRowCount();
	private:
		struct CSVRow {
			std::vector<CString> cols;
		};
		std::vector<CSVRow> rows;
	};
}
