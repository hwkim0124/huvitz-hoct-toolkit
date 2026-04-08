
// HctReportDoc.h : interface of the CHctReportDoc class
//


#pragma once


class CHctReportDoc : public CDocument
{
protected: // create from serialization only
	CHctReportDoc();
	DECLARE_DYNCREATE(CHctReportDoc)

// Attributes
public:
	enum DocType {
		DOC_SCAN_SEGMENT,
		DOC_WIDE_ANTERIOR, 
		DOC_ANTERIOR_LENS, 
		DOC_LENS_THICKNESS
	};

	DocType _docType = DocType::DOC_SCAN_SEGMENT;
	std::vector<WideCreator> _wides;
	std::vector<AnteriorLens> _anteriorLens;

// Operations
public:
	bool isScanSegment(void) {
		return _docType == DocType::DOC_SCAN_SEGMENT;
	}

	bool isWideAnterior(void) {
		return _docType == DocType::DOC_WIDE_ANTERIOR;
	}

	bool isAnteriorLens(void) {
		return _docType == DocType::DOC_ANTERIOR_LENS;
	}

	bool isLensThickness(void) {
		return _docType == DocType::DOC_LENS_THICKNESS;
	}

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// Implementation
public:
	virtual ~CHctReportDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void openWideAnteriorData(LPCTSTR path);
	void openAnteriorLensData(LPCTSTR path);
	void openLensThicknessData(LPCTSTR path);

	void openLineScanData(LPCTSTR path);
	void openCubeScanData(LPCTSTR path);
	void openRadialScanData(LPCTSTR path, int lineSize, int numLines);

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// Helper function that sets search content for a Search Handler
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
};
