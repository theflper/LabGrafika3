
// LabGrafika3View.cpp : implementation of the CLabGrafika3View class
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "LabGrafika3.h"
#endif

#include "LabGrafika3Doc.h"
#include "LabGrafika3View.h"
#include <cmath>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLabGrafika3View

IMPLEMENT_DYNCREATE(CLabGrafika3View, CView)

BEGIN_MESSAGE_MAP(CLabGrafika3View, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CLabGrafika3View construction/destruction
// CLab2View drawing

namespace
{
	DWORD transformMode(bool right)
	{
		if (right)
			return MWT_RIGHTMULTIPLY;

		return MWT_LEFTMULTIPLY;
	}
	CString putanjaDoFajla(LPCTSTR imeFajla)
	{
		TCHAR exePutanja[MAX_PATH] = {};
		DWORD duzina = GetModuleFileName(nullptr, exePutanja, MAX_PATH);
		if (duzina == 0)
			return CString();

		CString putanja(exePutanja, static_cast<int>(duzina));
		int poslednjaKosaCrta = putanja.ReverseFind(_T('\\'));
		if (poslednjaKosaCrta >= 0)
			putanja = putanja.Left(poslednjaKosaCrta + 1);

		return putanja + imeFajla;
	}

	void crtaj_kaktus(CDC* pDC, LPCTSTR imeFajla)
	{
		CString putanja = putanjaDoFajla(imeFajla);
		HENHMETAFILE hEmf = GetEnhMetaFile(putanja);
		if (hEmf == NULL)
			return;

		ENHMETAHEADER header = {};
		GetEnhMetaFileHeader(hEmf, sizeof(header), &header);
		CRect sourceRect(header.rclBounds.left, header.rclBounds.top,
			header.rclBounds.right, header.rclBounds.bottom);
		PlayEnhMetaFile(pDC->m_hDC, hEmf, &sourceRect);
		DeleteEnhMetaFile(hEmf);
	}

	BOOL primeniTransformaciju(CDC* pDC, const XFORM& transform, DWORD mode)
	{
		XFORM oldTransform;
		if (!pDC->GetWorldTransform(&oldTransform))
			return FALSE;

		XFORM result;
		if (mode == MWT_LEFTMULTIPLY)
			CombineTransform(&result, &transform, &oldTransform);
		else
			CombineTransform(&result, &oldTransform, &transform);

		return pDC->SetWorldTransform(&result);
	}
}
CLabGrafika3View::CLabGrafika3View() noexcept
{
	// TODO: add construction code here

}

CLabGrafika3View::~CLabGrafika3View()
{
}

BOOL CLabGrafika3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}
void CLabGrafika3View::Scale(CDC* pDC, float scaleX, float scaleY, bool right)
{
	if (pDC == nullptr)
		return;
	DWORD mode = transformMode(right);

	XFORM transform = { scaleX, 0.0f, 0.0f, scaleY, 0.0f, 0.0f };
	pDC->SetGraphicsMode(GM_ADVANCED);
	primeniTransformaciju(pDC, transform, mode);
}

void CLabGrafika3View::Rotate(CDC* pDC, float angle, bool right)
{
	if (pDC == nullptr)
		return ;
	DWORD mode = transformMode(right);

	const float radians = angle * 3.14159265358979323846f / 180.0f;
	const float cosine = std::cos(radians);
	const float sine = std::sin(radians);
	XFORM transform = { cosine, sine, -sine, cosine, 0.0f, 0.0f };
	pDC->SetGraphicsMode(GM_ADVANCED);
	primeniTransformaciju(pDC, transform, mode);
}

void CLabGrafika3View::Translate(CDC* pDC, float offsetX, float offsetY, bool right)
{
	if (pDC == nullptr)
		return;
	DWORD mode = transformMode(right);

	XFORM transform = { 1.0f, 0.0f, 0.0f, 1.0f, offsetX, offsetY };
	pDC->SetGraphicsMode(GM_ADVANCED);
	primeniTransformaciju(pDC, transform, mode);
}

void CLabGrafika3View::Mirror(CDC* pdc, bool mx, bool my, bool right)
{
	if (pdc == nullptr)
		return;
	DWORD mode = transformMode(right);
	XFORM transform = { mx ? -1.0f : 1.0f, 0.0f, 0.0f, my ? -1.0f : 1.0f, 0.0f, 0.0f };
	pdc->SetGraphicsMode(GM_ADVANCED);
	primeniTransformaciju(pdc, transform, mode);
}

void CLabGrafika3View::PushTransformacija(CDC* pDC)
{
	if (pDC == nullptr)
		return;

	pDC->SetGraphicsMode(GM_ADVANCED);

	XFORM currentTransform =
	{
		1.0f,
		0.0f,
		0.0f,
		1.0f,
		0.0f,
		0.0f
	};

	if (pDC->GetWorldTransform(&currentTransform))
		transformStack.push_back(currentTransform);
}

void CLabGrafika3View::PopTransformacija(CDC* pDC)
{
	if (transformStack.empty())
		return;
	//uzmi sa vrha steka kao prethodno stanje transformacija
	pDC->SetWorldTransform(&transformStack.back());
	//skini element sa vrha steka
	transformStack.pop_back();
}
void CLabGrafika3View::DrawPuzzlePiece(CDC* pDC, DImage& img, float x, float y, float angle, float sx, float sy, bool mx, bool my, bool isBlue)
{
	if (pDC == NULL)
		return;

	int width = img.Width();   // 256
	int height = img.Height(); // 256

	// Fiksni centar rotacije
	const float centerX = 125.0f;
	const float centerY = 136.0f;

	// Granice pravougaonika slagalice unutar bitmape
	int xSrc = 25 - 3;    // 22
	int ySrc = 36 - 3;    // 33
	int right = 226 + 3;  // 229
	int bottom = 236 + 3; // 239

	int nWidth = right - xSrc;    // 207 
	int nHeight = bottom - ySrc;  // 206 

	CBitmap* bitmap = img.GetBitmap();

	CDC sourceDC;
	CDC maskDC;
	CBitmap maskBitmap;

	if (!sourceDC.CreateCompatibleDC(pDC))
		return;
	if (!maskDC.CreateCompatibleDC(pDC))
		return;
	if (!maskBitmap.CreateBitmap(width, height, 1, 1, NULL))
		return;

	CBitmap* oldSourceBitmap = sourceDC.SelectObject(bitmap);
	CBitmap* oldMaskBitmap = maskDC.SelectObject(&maskBitmap);

	// 1. KREIRANJE MASKE (NOTSRCCOPY priprema masku gde je puzla bela, a pozadina crna za PlgBlt)
	COLORREF clrSaveBk = sourceDC.SetBkColor(RGB(0, 255, 0));
	maskDC.BitBlt(0, 0, width, height, &sourceDC, 0, 0, NOTSRCCOPY);
	sourceDC.SetBkColor(clrSaveBk);

	// 2. FILTRIRANJE PIKSELA (pretvaranje zelene pozadine u crnu)
	unsigned char* pixels = img.GetDIBBits();
	if (pixels)
	{
		int bpp = img.BPP();
		for (int i = 0; i < width * height; ++i)
		{
			int idx = i * bpp;
			unsigned char b = pixels[idx];
			unsigned char g = pixels[idx + 1];
			unsigned char r = pixels[idx + 2];

			if (r == 0 && g == 255 && b == 0)
			{
				pixels[idx] = 0;
				pixels[idx + 1] = 0;
				pixels[idx + 2] = 0;
			}
			else if (!isBlue)
			{
				int gr = 64 + (r + g + b) / 3;
				if (gr > 255) gr = 255;

				pixels[idx] = (unsigned char)gr;
				pixels[idx + 1] = (unsigned char)gr;
				pixels[idx + 2] = (unsigned char)gr;
			}
			else
			{
				pixels[idx] = (unsigned char)(b < 128 ? b + 127 : 255);
				pixels[idx + 1] = 0;
				pixels[idx + 2] = 0;
			}
		}
		img.Update();
	}

	// Osvežavamo sourceDC sa izmenjenim bitmapom
	sourceDC.SelectObject(img.GetBitmap());

	// 3. MATEMATIČKO RAČUNANJE TAČAKA ZA PlgBlt (Samo Translate i Rotate oko centra 125, 136)
	POINT lpPoint[3];

	float radians = angle * 3.14159265358979323846f / 180.0f;
	float cosA = std::cos(radians);
	float sinA = std::sin(radians);

	// 3 ključna ugla izvornog pravougaonika u lokalnom sistemu (u odnosu na centar 125, 136):
	// [0] = Gornji levi (UL) -> (25, 36)
	// [1] = Gornji desni (UR) -> (226, 36)
	// [2] = Donji levi (LL)  -> (25, 236)
	float localPts[3][2] = {
		{  22.0f - centerX,   33.0f - centerY }, // UL
		{ 229.0f - centerX,   33.0f - centerY }, // UR
		{  22.0f - centerX,  239.0f - centerY }  // LL
	};

	// Odredišni centar na ekranu kombinuje željenu poziciju (x, y) i unutrašnji centar bitmape
	float destCenterX = x + centerX;
	float destCenterY = y + centerY;

	for (int i = 0; i < 3; ++i)
	{
		float px = localPts[i][0];
		float py = localPts[i][1];

		// Rotacija oko centra
		float rx = px * cosA - py * sinA;
		float ry = px * sinA + py * cosA;

		// Prevođenje (Translate) na željenu poziciju na ekranu
		lpPoint[i].x = (LONG)(rx + destCenterX);
		lpPoint[i].y = (LONG)(ry + destCenterY);
	}

	// 4. CRTANJE SA PlgBlt (Automatski odrađuje rotaciju, pozicioniranje i providnost preko maske)
	pDC->PlgBlt(lpPoint, &sourceDC, xSrc, ySrc, nWidth, nHeight, maskBitmap, xSrc, ySrc);

	// Čišćenje resursa
	sourceDC.SelectObject(oldSourceBitmap);
	maskDC.SelectObject(oldMaskBitmap);
	sourceDC.DeleteDC();
	maskDC.DeleteDC();
}
// CLabGrafika3View drawing

void CLabGrafika3View::OnDraw(CDC* pDC)
{
	CLabGrafika3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	Mirror(pDC, true, false, false);
	Translate(pDC, -500, 0, false);
	// Dobijamo veličinu klijentskog prozora
	CRect rect;
	GetClientRect(&rect);

	// Kreiranje memorijskog DC-ja za Anti-Flicker
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

	// Belo ispunjavanje pozadine memorijskog DC-ja
	memDC.FillSolidRect(&rect, RGB(255, 255, 255));

	// Crtanje grida na memDC
	CPen pen(PS_SOLID, 1, RGB(200, 200, 200));
	CPen* pOldPen = memDC.SelectObject(&pen);
	for (int i = 0; i <= 500; i += 25)
	{
		memDC.MoveTo(i, 0);
		memDC.LineTo(i, 500);
		memDC.MoveTo(0, i);
		memDC.LineTo(500, i);
	}
	memDC.SelectObject(pOldPen);

	// Učitavanje i iscrtavanje puzle pozivanjem DrawPuzzlePiece na memDC
	DImage img;
	img.Load(putanjaDoFajla(_T("monalisa0.bmp"))); // ili odgovarajući fajl puzle
	DrawPuzzlePiece(&memDC, img, 275, -35, 205, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa1.bmp")));
	DrawPuzzlePiece(&memDC, img, 125, 115, 57.5f, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa8.bmp")));
	DrawPuzzlePiece(&memDC, img, 275, 115, 107, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa5.bmp")));
	DrawPuzzlePiece(&memDC, img, 125, -35, 235, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa2.bmp")));
	DrawPuzzlePiece(&memDC, img, -25, 115, 72, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa3.bmp")));
	DrawPuzzlePiece(&memDC, img, 275, 265, 19, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa4.bmp")));
	DrawPuzzlePiece(&memDC, img, -25, -35, -76, 1, 1, false, false, false);
	img.Load(putanjaDoFajla(_T("monalisa6.bmp")));
	DrawPuzzlePiece(&memDC, img, startPosX, startPosY, 113, 1, 1, false, false, true);
	img.Load(putanjaDoFajla(_T("monalisa7.bmp")));
	DrawPuzzlePiece(&memDC, img, -25, 265, 161, 1, 1, false, false, false);
	// Prenos sadržaja sa memorijskog DC-ja na ekran (glavni pDC) u jednom koraku
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	// Čišćenje resursa
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
}


// CLabGrafika3View printing

BOOL CLabGrafika3View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLabGrafika3View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CLabGrafika3View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CLabGrafika3View diagnostics

#ifdef _DEBUG
void CLabGrafika3View::AssertValid() const
{
	CView::AssertValid();
}

void CLabGrafika3View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLabGrafika3Doc* CLabGrafika3View::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLabGrafika3Doc)));
	return (CLabGrafika3Doc*)m_pDocument;
}
#endif //_DEBUG


// CLabGrafika3View message handlers

void CLabGrafika3View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_LEFT)  {startPosX += 5; Invalidate();}
	if (nChar == VK_RIGHT) { startPosX -= 5; Invalidate(); }
	if (nChar == VK_UP) { startPosY -= 5; Invalidate();}
	if (nChar == VK_DOWN) { startPosY += 5; Invalidate(); }
	// TODO: Add your message handler code here and/or call default
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CLabGrafika3View::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	return true;
}
