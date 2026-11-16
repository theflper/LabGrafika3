
// LabGrafika3View.h : interface of the CLabGrafika3View class
//

#pragma once
#include "DImage.h"
#include <vector>

class CLabGrafika3View : public CView
{
protected: // create from serialization only
	CLabGrafika3View() noexcept;
	DECLARE_DYNCREATE(CLabGrafika3View)


// Attributes
public:
	CLabGrafika3Doc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	int startPosX = 125; // na primer donji red
	int startPosY = 264; // srednja kolona (kao na slici)

// Implementation
public:
	virtual ~CLabGrafika3View();
	void Scale(CDC* pDC, float scaleX, float scaleY, bool right);
	void Rotate(CDC* pDC, float angle, bool right);
	void Translate(CDC* pDC, float offsetX, float offsetY, bool right);
	void Mirror(CDC* pdc, bool mx, bool my, bool right);
	void PushTransformacija(CDC* pDC);
	void PopTransformacija(CDC* pDC);
	void DrawPuzzlePiece(CDC *pDC, DImage &img, float x, float y, float angle, float sx, float sy, bool mx, bool my, bool isBlue);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	std::vector<XFORM> transformStack;
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

#ifndef _DEBUG  // debug version in LabGrafika3View.cpp
inline CLabGrafika3Doc* CLabGrafika3View::GetDocument() const
   { return reinterpret_cast<CLabGrafika3Doc*>(m_pDocument); }
#endif

