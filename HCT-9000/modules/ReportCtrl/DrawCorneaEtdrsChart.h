#pragma once

class REPORTCTRL_DLL_API DrawCorneaEtdrsChart
{
public:
	typedef enum _CorneaThickDir {
		CORNEA_THICK_DIR_0 = 0,
		CORNEA_THICK_DIR_1,
		CORNEA_THICK_DIR_2,
		CORNEA_THICK_DIR_3,
		CORNEA_THICK_DIR_4,
		CORNEA_THICK_DIR_5,
		CORNEA_THICK_DIR_6,
		CORNEA_THICK_DIR_7,
		CORNEA_THICK_DIR_NUM
	} CorneaThickDir;

	typedef enum _CorneaThickSide {
		CORNEA_THICK_INNER = 0,
		CORNEA_THICK_OUTER,
		CORNEA_THICK_SIDE_NUM
	} CorneaThickSide;

public:
	DrawCorneaEtdrsChart();
	~DrawCorneaEtdrsChart();
	void initialize(EyeSide side);

	void clearData(void);
	void setCorneaEtdrsThicknessCenter(float value);
	void setCorneaEtdrsThicknessInner(CorneaThickDir dir, float value);
	void setCorneaEtdrsThicknessOuter(CorneaThickDir dir, float value);
	void drawCorneaEtdrsChart(CDC *pDC, CRect rect);

protected:
	EyeSide m_eyeSide;
	CString m_direction[4];

	float m_corneaEtdrsThicknessCenter;
	float m_corneaEtdrsThickness[CORNEA_THICK_SIDE_NUM][CORNEA_THICK_DIR_NUM];
};

