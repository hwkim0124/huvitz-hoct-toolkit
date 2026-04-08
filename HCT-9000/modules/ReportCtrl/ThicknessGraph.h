#pragma once
#include "ReportCtrlDef.h"
#include "DrawThicknessGraph.h"

// ThicknessGraph

class REPORTCTRL_DLL_API ThicknessGraph : public CWnd, public DrawThicknessGraph
{
	DECLARE_DYNAMIC(ThicknessGraph)

public:
	ThicknessGraph();
	virtual ~ThicknessGraph();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
};


