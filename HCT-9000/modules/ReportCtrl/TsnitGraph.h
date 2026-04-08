#pragma once
#include "ReportCtrlDef.h"
#include "DrawThicknessGraph.h"

// TsnitGraph

class REPORTCTRL_DLL_API TsnitGraph : public CWnd, public DrawThicknessGraph
{
	DECLARE_DYNAMIC(TsnitGraph)

public:
	TsnitGraph();
	virtual ~TsnitGraph();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
};


