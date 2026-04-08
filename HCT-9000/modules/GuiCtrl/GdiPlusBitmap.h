#pragma once


class GdiPlusBitmap
{
public:
	Gdiplus::Bitmap* m_pBitmap;

public:
	GdiPlusBitmap() { m_pBitmap = NULL; }
	GdiPlusBitmap(LPCWSTR pFile) { m_pBitmap = NULL; Load(pFile); }
	virtual ~GdiPlusBitmap() { Empty(); }

	void Empty() { delete m_pBitmap; m_pBitmap = NULL; }

	bool Load(LPCWSTR pFile)
	{
		Empty();
		m_pBitmap = Gdiplus::Bitmap::FromFile(pFile);
		return m_pBitmap->GetLastStatus() == Gdiplus::Ok;
	}

	operator Gdiplus::Bitmap*() const { return m_pBitmap; }
};
