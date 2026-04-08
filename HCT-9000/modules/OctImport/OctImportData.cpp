#include "stdafx.h"

#define OCTIMPORT_DATA_EXPORTS
#include "OctImportData.h"

using namespace octimport;

OctImportData::OctImportData()
{
}


OctImportData::~OctImportData()
{
}

OctImportData::OctImportData(CString csvFilePath)
{
	ReadCSV(csvFilePath);
}

void OctImportData::ReadCSV(CString csvFilePath) {
	CString csvLine;
	TRY
	{
		CStdioFile file(csvFilePath, CFile::modeRead);
	CString buffer;
	while (file.ReadString(csvLine)) {
		CSVRow row;
		CString token;
		CString delimiter = _T(",");
		CString quote = _T("\"");

		int start = 0;
		int end = csvLine.Find(delimiter);
		int quote_start = 0;
		BOOL ok = TRUE;

		do {
			if (end < 0) {
				token = csvLine.Right(csvLine.GetLength() - start);
				ok = FALSE;
			}
			else {
				token = csvLine.Mid(start, end - start);
				ok = TRUE;
			}

			quote_start = token.Find(quote);
			if (quote_start >= 0) {
				end = csvLine.Find(quote, start + quote_start + 1);
				if (end < 0) {
					token = csvLine.Right(csvLine.GetLength() - start);
					ok = FALSE;
				}
				else {
					end += 1;
					token = csvLine.Mid(start, end - start);
				}
			}

			row.cols.push_back(token);

			/*buffer.Format(_T("start=%d, end=%d, [%s]\n"), start, end, token);
			OutputDebugStringW(buffer);*/


			start = end + delimiter.GetLength();
			end = csvLine.Find(delimiter, start);

		} while (ok);
		rows.push_back(row);
	}
	}
		CATCH_ALL(e)
	{
		e->ReportError();
	}
	END_CATCH_ALL

}

CString OctImportData::GetValue(int col, int row) {
	if (row >= (int)rows.size()) {
		return _T("");
	}
	CSVRow r = rows.at(row);

	if (col >= (int)r.cols.size()) {
		return _T("");
	}

	return r.cols.at(col);
}

int OctImportData::GetColumnCount(int row) {
	if (row >= (int)rows.size()) {
		return 0;
	}
	return ((rows.at(row)).cols).size();
}

int OctImportData::GetRowCount() {
	return rows.size();
}
