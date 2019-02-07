#include<Windows.h>
#include<gdiplus.h>
#include<string>
#include"resource.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"gdiplus.lib")
#define KEYS_NUM 255
class DPIInfo
{
public:
	DPIInfo() :zoom(1.0f)
	{
		HDC h = GetDC(0);
		sx = GetDeviceCaps(h, LOGPIXELSX);
		sy = GetDeviceCaps(h, LOGPIXELSY);
	}
	template<typename Tnum>Tnum X(Tnum n)const { return (Tnum)(n*sx*zoom / odpi); }
	template<typename Tnum>Tnum Y(Tnum n)const { return (Tnum)(n*sy*zoom / odpi); }
	template<typename Tnum>Tnum P(Tnum n)const { return (Tnum)(n*zoom); }
	void SetZoom(float z)
	{
		zoom = z;
	}
private:
	const int odpi = 96;
	int sx, sy;
	float zoom;
}S;
struct KeyManager
{
	bool keys[KEYS_NUM];
	Gdiplus::RectF rkeys[KEYS_NUM];
	TCHAR fontName[256];
	BYTE foreColorR, foreColorG, foreColorB, backColorR, backColorG, backColorB;
	float lineWidth;
	float fontSize;
	DWORD lastVkCode;
	void ResetKeySize()
	{
		lastVkCode = 0;
		GetPrivateProfileString(TEXT("keymon"), TEXT("fontName"), TEXT("����"), fontName, ARRAYSIZE(fontName) - 1, TEXT(".\\keymon.ini"));
		foreColorR = GetPrivateProfileInt(TEXT("keymon"), TEXT("foreColorR"), 255, TEXT(".\\keymon.ini"));
		foreColorG = GetPrivateProfileInt(TEXT("keymon"), TEXT("foreColorG"), 255, TEXT(".\\keymon.ini"));
		foreColorB = GetPrivateProfileInt(TEXT("keymon"), TEXT("foreColorB"), 255, TEXT(".\\keymon.ini"));
		backColorR = GetPrivateProfileInt(TEXT("keymon"), TEXT("backColorR"), 0, TEXT(".\\keymon.ini"));
		backColorG = GetPrivateProfileInt(TEXT("keymon"), TEXT("backColorG"), 0, TEXT(".\\keymon.ini"));
		backColorB = GetPrivateProfileInt(TEXT("keymon"), TEXT("backColorB"), 0, TEXT(".\\keymon.ini"));
		lineWidth = (float)GetPrivateProfileInt(TEXT("keymon"), TEXT("lineWidth"), 2, TEXT(".\\keymon.ini"));
		fontSize = (float)GetPrivateProfileInt(TEXT("keymon"), TEXT("fontSize"), 16, TEXT(".\\keymon.ini"));
		int nShowKeys = 0;
		for (int i = 0; i < 255; i++)
		{
			TCHAR line[256], szVkCode[16];
			wsprintf(szVkCode, TEXT("%d"), i);
			GetPrivateProfileString(TEXT("keymon"),szVkCode,TEXT(""),line,ARRAYSIZE(line)-1,TEXT(".\\keymon.ini"));
			if (line[0])
			{
				int l,t,r,b;
				TCHAR caps[16];
				swscanf_s(line, TEXT("%d,%d,%d,%d,%s"), &l, &t, &r, &b, caps, ARRAYSIZE(caps) - 1);
				rkeys[i] = { S.X((float)l),S.Y((float)t),S.X((float)r),S.Y((float)b) };
				szKeys[i] = caps;
				nShowKeys++;
			}
		}
		if (nShowKeys==0)
		{
			for (auto &s : szKeys)
				s = TEXT("");
			rkeys[VK_LSHIFT] = { S.X(8.0f),S.Y(15.0f),S.X(66.0f),S.Y(28.0f) };
			rkeys['Z'] = { S.X(80.0f),S.Y(15.0f),S.X(32.0f),S.Y(28.0f) };
			rkeys['X'] = { S.X(118.0f),S.Y(15.0f),S.X(32.0f),S.Y(28.0f) };
			rkeys['C'] = { S.X(156.0f),S.Y(15.0f),S.X(32.0f),S.Y(28.0f) };
			rkeys[VK_LEFT] = { S.X(195.0f),S.Y(29.0f),S.X(24.0f),S.Y(24.0f) };
			rkeys[VK_UP] = { S.X(219.0f),S.Y(5.0f),S.X(24.0f),S.Y(24.0f) };
			rkeys[VK_DOWN] = { S.X(219.0f),S.Y(29.0f),S.X(24.0f),S.Y(24.0f) };
			rkeys[VK_RIGHT] = { S.X(243.0f),S.Y(29.0f),S.X(24.0f),S.Y(24.0f) };
			szKeys[VK_LSHIFT]=TEXT("Shift");
			szKeys['Z']=TEXT("Z");
			szKeys['X']=TEXT("X");
			szKeys['C']=TEXT("C");
			szKeys[VK_LEFT]=TEXT("��");
			szKeys[VK_UP]=TEXT("��");
			szKeys[VK_DOWN]=TEXT("��");
			szKeys[VK_RIGHT]=TEXT("��");
		}
	}
	std::wstring szKeys[KEYS_NUM];
	void SetKey(DWORD key, bool v, HWND h)
	{
		if (keys[key] == v)return;
		keys[key] = v;
		lastVkCode = key;
		RECT r = { (LONG)rkeys[key].X,(LONG)rkeys[key].Y,(LONG)(rkeys[key].X + rkeys[key].Width),(LONG)(rkeys[key].Y + rkeys[key].Height) };
		InvalidateRect(h, &r, FALSE);
	}
}km;
HWND hWindow = NULL;
POINT posMouseClick;
HMENU hMenu = NULL;
class WindowSize
{
private:
	int owidth, oheight;
	float zoom_multiplier;
	float alpha;
	HWND m_hWindow;
	TCHAR menuTextFmt[20] = TEXT(""), menuAlphaFmt[20] = TEXT("");
	int wstyle, wexstyle;
public:
	WindowSize() :zoom_multiplier(1.0f), alpha(1.0f)
	{
	}
	void SetOriginalSize(HWND hwnd, int w, int h)
	{
		m_hWindow = hwnd;
		owidth = w;
		oheight = h;
		SetZoom(1.0f);
		SetAlpha(1.0f);
	}
	void SetStyle(int f)
	{
		wstyle = f;
	}
	int GetStyle()
	{
		return wstyle;
	}
	void SetExStyle(int f)
	{
		wexstyle = f;
	}
	int GetExStyle()
	{
		return wexstyle;
	}
	float GetZoom()
	{
		return zoom_multiplier;
	}
	void SetZoom(float z)
	{
		zoom_multiplier = z;
		S.SetZoom(zoom_multiplier);
		km.ResetKeySize();
		RECT rWindow;
		GetClientRect(m_hWindow, &rWindow);
		ClientToScreen(m_hWindow, (LPPOINT)&rWindow);
		rWindow.right = rWindow.left + (long)(owidth*zoom_multiplier);
		rWindow.bottom = rWindow.top + (long)(oheight*zoom_multiplier);
		AdjustWindowRectEx(&rWindow, wstyle, FALSE, wexstyle);
		MoveWindow(m_hWindow, rWindow.left, rWindow.top, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, TRUE);
		UpdateMenuText();
	}
	float GetAlpha()
	{
		return alpha;
	}
	void SetAlpha(float a)
	{
		alpha = a;
		BYTE bAlpha = (BYTE)min(alpha * 256.0f, 255.0f);
		SetLayeredWindowAttributes(hWindow, NULL, bAlpha, LWA_ALPHA);
		UpdateMenuText();
	}
	void UpdateMenuText()
	{
		TCHAR menuText[ARRAYSIZE(menuTextFmt)];
		MENUITEMINFO mii;
		mii.cbSize = sizeof mii;
		mii.fMask = MIIM_STRING;
		mii.fType = MFT_STRING;
		mii.dwTypeData = menuText;
		mii.cch = ARRAYSIZE(menuText) - 1;
		if (lstrcmp(menuTextFmt, TEXT("")) == 0)
		{
			GetMenuItemInfo(hMenu, 0, TRUE, &mii);
			lstrcpy(menuTextFmt, mii.dwTypeData);
		}
		wsprintf(menuText, menuTextFmt, (int)(zoom_multiplier*100.0f));
		SetMenuItemInfo(hMenu, 0, TRUE, &mii);
		mii.cch = ARRAYSIZE(menuText) - 1;
		if (lstrcmp(menuAlphaFmt, TEXT("")) == 0)
		{
			GetMenuItemInfo(hMenu, 1, TRUE, &mii);
			lstrcpy(menuAlphaFmt, mii.dwTypeData);
		}
		wsprintf(menuText, menuAlphaFmt, (int)(alpha*100.0f));
		SetMenuItemInfo(hMenu, 1, TRUE, &mii);
	}
}ws;
void OnPaint(HWND h, WPARAM w, LPARAM l)
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(h, &ps);
	RECT clientRect;
	GetClientRect(h, &clientRect);
	Gdiplus::GdiplusStartupInput gsi;
	ULONG_PTR gpToken;
	Gdiplus::GdiplusStartup(&gpToken, &gsi, NULL);
	Gdiplus::Graphics gr(hDC);
	Gdiplus::Font textFont(km.fontName, S.P(km.fontSize));
	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
	Gdiplus::Pen whitePen(Gdiplus::Color(km.foreColorR, km.foreColorG, km.foreColorB), S.P(km.lineWidth));
	Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(km.foreColorR, km.foreColorG, km.foreColorB)), blackBrush(Gdiplus::Color(km.backColorR, km.backColorG, km.backColorB));
	gr.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	gr.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
	if (km.lastVkCode)
	{
		if (km.szKeys[km.lastVkCode][0])
		{
			gr.FillRectangle(km.keys[km.lastVkCode] ? &whiteBrush : &blackBrush, km.rkeys[km.lastVkCode]);
			gr.DrawRectangle(&whitePen, km.rkeys[km.lastVkCode]);
			gr.DrawString(km.szKeys[km.lastVkCode].c_str(), km.szKeys[km.lastVkCode].size(), &textFont, km.rkeys[km.lastVkCode], &sf, km.keys[km.lastVkCode] ? &blackBrush : &whiteBrush);
		}
	}
	else
	{
		gr.FillRectangle(&blackBrush, clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
		for (int i = 0; i < ARRAYSIZE(km.keys); i++)
		{
			if (km.szKeys[i][0])
			{
				gr.FillRectangle(km.keys[i] ? &whiteBrush : &blackBrush, km.rkeys[i]);
				gr.DrawRectangle(&whitePen, km.rkeys[i]);
				gr.DrawString(km.szKeys[i].c_str(), km.szKeys[i].size(), &textFont, km.rkeys[i], &sf, km.keys[i] ? &blackBrush : &whiteBrush);
			}
		}
	}
	EndPaint(h, &ps);
}
LRESULT CALLBACK ProcessMessage(HWND h, UINT m, WPARAM w, LPARAM l)
{
	switch (m)
	{
	case WM_PAINT:OnPaint(h, w, l); break;
	case WM_CLOSE:PostQuitMessage(0); break;
	case WM_LBUTTONDBLCLK:
		RECT rWindow;
		if (GetWindowLongPtr(h,GWL_STYLE)&WS_CAPTION)
		{
			GetClientRect(h, &rWindow);
			ClientToScreen(h, (POINT*)&rWindow);
			ClientToScreen(h, (POINT*)&rWindow.right);
		}
		else
		{
			GetWindowRect(h, &rWindow);
			AdjustWindowRectEx(&rWindow, GetWindowLongPtr(h, GWL_STYLE) ^ WS_CAPTION, FALSE, GetWindowLongPtr(h, GWL_EXSTYLE));
		}
		ws.SetStyle(GetWindowLongPtr(h, GWL_STYLE) ^ WS_CAPTION);
		SetWindowLongPtr(h, GWL_STYLE, ws.GetStyle());
		SetWindowPos(h, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		MoveWindow(h, rWindow.left, rWindow.top, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, TRUE);
	case WM_SIZE:
		km.lastVkCode = 0;
		break;
	case WM_LBUTTONDOWN:
		SetCapture(h);
		posMouseClick.x = LOWORD(l);
		posMouseClick.y = HIWORD(l);
		break;
	case WM_LBUTTONUP:ReleaseCapture(); break;
	case WM_MOUSEMOVE://http://www.cplusplus.com/forum/windows/24514/
		if (GetCapture() == h)
		{
			RECT rWindow;
			GetWindowRect(h, &rWindow);
			SetWindowPos(h, NULL, rWindow.left + (short)LOWORD(l) - posMouseClick.x,
				rWindow.top + (short)HIWORD(l) - posMouseClick.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		break;
	case WM_RBUTTONDOWN:
		posMouseClick.x = LOWORD(l);
		posMouseClick.y = HIWORD(l);
		ClientToScreen(h, &posMouseClick);
		TrackPopupMenu(hMenu, TPM_LEFTALIGN, posMouseClick.x, posMouseClick.y, 0, h, IGNORE);
		break;
	case WM_COMMAND:
		switch (LOWORD(w))
		{
		case ID_MENU_HELP:
			MessageBox(h, TEXT("��������Լ��Ӽ��̵İ���״̬��ʵʱ��ʾ��\n* ˫�����ڿ������ر߿�\n\nBy lxfly2000"),
				TEXT("������ʾ����"), NULL);
			break;
		case ID_MENU_MINIMIZE:ShowWindow(h, SW_MINIMIZE); break;
		case ID_MENU_CLOSE:PostQuitMessage(0); break;
		case IDM_ZOOM_IN:ws.SetZoom(min(ws.GetZoom() + 0.25f, 8.0f)); break;
		case IDM_ZOOM_OUT:ws.SetZoom(max(ws.GetZoom() - 0.25f, 0.25f)); break;
		case IDM_ALPHA_INCREASE:ws.SetAlpha(min(ws.GetAlpha() + 0.25f, 1.0f)); break;
		case IDM_ALPHA_DECREASE:ws.SetAlpha(max(ws.GetAlpha() - 0.25f, 0.25f)); break;
		}
		break;
	}
	return DefWindowProc(h, m, w, l);
}
LRESULT CALLBACK ProcessHook(int c, WPARAM w, LPARAM l)
{
	if (c == HC_ACTION)
		switch (w)
		{
		case WM_KEYDOWN:case WM_KEYUP:case WM_SYSKEYDOWN:case WM_SYSKEYUP:
			km.SetKey(((PKBDLLHOOKSTRUCT)l)->vkCode, w == WM_KEYDOWN, hWindow);
			break;
		}
	return CallNextHookEx(NULL, c, w, l);
}
ATOM RegWindowClass(HINSTANCE hI, WNDPROC fP, LPCTSTR cn)
{
	WNDCLASSEX w = { sizeof w,CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,fP,0,0,hI,LoadIcon(hI,MAKEINTRESOURCE(IDI_ICON)),
		LoadCursor(NULL,IDC_ARROW),(HBRUSH)GetStockObject(BLACK_BRUSH),NULL,cn,NULL };
	return RegisterClassEx(&w);
}
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int iCmdShow)
{
	km.ResetKeySize();
	//���ɴ���
	PCTSTR wcl_name = TEXT("KeyWindow");
	if (!RegWindowClass(hInstance, ProcessMessage, wcl_name))return -1;
	LONG ww = S.X(GetPrivateProfileInt(TEXT("keymon"),TEXT("width"),275,TEXT(".\\keymon.ini"))), wh = S.Y(GetPrivateProfileInt(TEXT("keymon"),TEXT("height"),58,TEXT(".\\keymon.ini")));
	RECT rWindow = { 0,0,ww,wh };
	ws.SetStyle(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
	ws.SetExStyle(WS_EX_APPWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED);
	AdjustWindowRectEx(&rWindow, ws.GetStyle(), FALSE, ws.GetExStyle());
	hWindow = CreateWindowEx(ws.GetExStyle(), wcl_name, TEXT("����"), ws.GetStyle(),
		CW_USEDEFAULT, CW_USEDEFAULT, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hWindow, iCmdShow);
	hMenu = GetSubMenu(LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_APP)), 0);
	ws.SetOriginalSize(hWindow, ww, wh);
	HACCEL haApp = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_APP));
	//��ȡ����
	HHOOK hkKey = SetWindowsHookEx(WH_KEYBOARD_LL, ProcessHook, hInstance, 0);
	//��Ϣѭ��
	MSG m;
	while (GetMessage(&m, NULL, 0, 0))
	{
		TranslateAccelerator(hWindow, haApp, &m);
		TranslateMessage(&m);
		DispatchMessage(&m);
	}
	//�ͷŹ���
	UnhookWindowsHookEx(hkKey);
	return m.wParam;
}