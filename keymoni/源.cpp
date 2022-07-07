//INI格式：
//[keymoni]
//img=<path>
//keyboardoff=<left> <top> <width> <height>  ;同时作为窗口大小
//keyboardon=<left> <top> <width> <height>
//capsoff=<left> <top> <width> <height>
//capson=<left> <top> <width> <height>
//scrolloff=<left> <top> <width> <height>
//scrollon=<left> <top> <width> <height>
//numoff=<left> <top> <width> <height>
//numon=<left> <top> <width> <height>
//capslight=<left> <top> <width> <height>    ;相对于keyboardoff
//scrolllight=<left> <top> <width> <height>  ;相对于keyboardoff
//numlight=<left> <top> <width> <height>     ;相对于keyboardoff
//<keycode>=<left> <top> <width> <height>    ;相对于keyboardoff，但keyboardon也会以该值取相对位置


#include<Windows.h>
#include<gdiplus.h>
#include<string>
#include<sstream>
#include"resource.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib,"gdiplus.lib")
#define KEYS_NUM 255
#define CONFIG_FILE ".\\keymoni.ini"
std::wstring GetINIString(LPCTSTR file, LPCTSTR key, LPCTSTR def)
{
	TCHAR buf[MAX_PATH];
	GetPrivateProfileString(TEXT("keymoni"), key, def, buf, ARRAYSIZE(buf), file);
	return std::wstring(buf);
}
Gdiplus::Rect GetINIRect(LPCTSTR file, LPCTSTR key, int defLeft, int defTop, int defWidth, int defHeight)
{
	Gdiplus::Rect r;
	TCHAR buf[60];
	std::wostringstream def;
	def << defLeft << TEXT(" ") << defTop << TEXT(" ") << defWidth << TEXT(" ") << defHeight;
	GetPrivateProfileString(TEXT("keymoni"), key, def.str().c_str(), buf, ARRAYSIZE(buf), file);
	std::wistringstream(buf) >> r.X >> r.Y >> r.Width >> r.Height;
	return r;
}
class DPIInfo
{
public:
	DPIInfo() :zoom(1.0f)
	{
		HDC h = GetDC(0);
	}
	//获得用户倍率缩放后的数值
	template<typename Tnum>Tnum P(Tnum n)const { return (Tnum)(n*zoom); }
	void SetZoom(float z)
	{
		zoom = z;
	}
private:
	float zoom;
}S;

float GetSystemZoom()
{
	return static_cast<int>(GetDeviceCaps(GetDC(NULL), LOGPIXELSX) * 4 / USER_DEFAULT_SCREEN_DPI) / 4.0f;
}

HWND hWindow = NULL;
POINT posMouseClick;
HMENU hMenu = NULL;

struct KeyManager
{
	bool keys[KEYS_NUM];
	Gdiplus::Rect rkeys[KEYS_NUM], rlightcap, rlightscroll, rlightnum;
	Gdiplus::Bitmap *bmpBack, *bmpFore, *bmpCapson, *bmpCapsoff, *bmpScrollon, *bmpScrolloff, *bmpNumon, *bmpNumoff;
	void ResetKeySize(LPCTSTR configFile)
	{
		std::wstring imgpath = GetINIString(configFile, TEXT("img"), TEXT(""));
		Gdiplus::Bitmap*img;
		if (imgpath.length())
			img = Gdiplus::Bitmap::FromFile(imgpath.c_str());
		else
			img = Gdiplus::Bitmap::FromResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BITMAP_DEFAULT));
		ReleaseImg();
		bmpBack = img->Clone(GetINIRect(configFile, TEXT("keyboardoff"), 0, 0, 106, 25), PixelFormat32bppARGB);
		bmpFore = img->Clone(GetINIRect(configFile, TEXT("keyboardon"), 0, 25, 106, 25), PixelFormat32bppARGB);
		bmpCapsoff = img->Clone(GetINIRect(configFile, TEXT("capsoff"), 0, 0, 0, 0), PixelFormat32bppARGB);
		bmpCapson = img->Clone(GetINIRect(configFile, TEXT("capson"), 0, 0, 0, 0), PixelFormat32bppARGB);
		bmpScrolloff = img->Clone(GetINIRect(configFile, TEXT("scrolloff"), 0, 0, 0, 0), PixelFormat32bppARGB);
		bmpScrollon = img->Clone(GetINIRect(configFile, TEXT("scrollon"), 0, 0, 0, 0), PixelFormat32bppARGB);
		bmpNumoff = img->Clone(GetINIRect(configFile, TEXT("numoff"), 0, 0, 0, 0), PixelFormat32bppARGB);
		bmpNumon = img->Clone(GetINIRect(configFile, TEXT("numon"), 0, 0, 0, 0), PixelFormat32bppARGB);
		delete img;

		int nShowKeys = 0;
		for (int i = 0; i < KEYS_NUM; i++)
		{
			TCHAR szVkCode[16];
			wsprintf(szVkCode, TEXT("%d"), i);
			Gdiplus::Rect rline = GetINIRect(configFile, szVkCode, 0, 0, 0, 0);
			if (rline.Width&&rline.Height)
			{
				rkeys[i] = rline;
				nShowKeys++;
			}
		}
		rlightcap = GetINIRect(configFile, TEXT("capslight"), 0, 0, 0, 0);
		rlightscroll = GetINIRect(configFile, TEXT("scrolllight"), 0, 0, 0, 0);
		rlightnum = GetINIRect(configFile, TEXT("numlight"), 0, 0, 0, 0);
		if (nShowKeys == 0)
		{
			rkeys[VK_LSHIFT] = { 1,6,26,13 };
			rkeys['Z'] = { 28,6,13,13 };
			rkeys['X'] = { 42,6,13,13 };
			rkeys['C'] = { 56,6,13,13 };
			rkeys[VK_LEFT] = { 70,13,11,11 };
			rkeys[VK_UP] = { 82,1,11,11 };
			rkeys[VK_DOWN] = { 82,13,11,11 };
			rkeys[VK_RIGHT] = { 94,13,11,11 };
		}
	}
	void SetKey(DWORD key, bool v, HWND h)
	{
		if (keys[key] == v)return;
		keys[key] = v;
		InvalidateRect(h, NULL, FALSE);
	}
	void DrawKey(Gdiplus::Graphics &gr, DWORD vk)
	{
		if (keys[vk])
			gr.DrawImage(bmpFore, rkeys[vk], rkeys[vk].X, rkeys[vk].Y, rkeys[vk].Width, rkeys[vk].Height, Gdiplus::UnitPixel);
		Gdiplus::Bitmap*b;
		switch (vk)
		{
		case VK_CAPITAL:
			b = (GetKeyState(vk) & 1) ? bmpCapson : bmpCapsoff;
			gr.DrawImage(b, rlightcap);
			break;
		case VK_SCROLL:
			b = (GetKeyState(vk) & 1) ? bmpScrollon : bmpScrolloff;
			gr.DrawImage(b, rlightscroll);
			break;
		case VK_NUMLOCK:
			b = (GetKeyState(vk) & 1) ? bmpNumon : bmpNumoff;
			gr.DrawImage(b, rlightnum);
			break;
		}
	}
	KeyManager() :bmpBack(nullptr), bmpFore(nullptr), bmpCapsoff(nullptr), bmpCapson(nullptr), bmpScrolloff(nullptr), bmpScrollon(nullptr),
		bmpNumoff(nullptr), bmpNumon(nullptr) {}
	void ReleaseImg()
	{
		delete bmpBack;
		delete bmpFore;
		delete bmpCapsoff;
		delete bmpCapson;
		delete bmpScrolloff;
		delete bmpScrollon;
		delete bmpNumoff;
		delete bmpNumon;
	}
}km;


class WindowSize
{
private:
	int owidth, oheight;
	float zoom_multiplier;
	float alpha;
	HWND m_hWindow;
	TCHAR menuTextFmt[20] = TEXT(""), menuAlphaFmt[20] = TEXT("");
	int wstyle, wexstyle;
	TCHAR szUsingLayout[MAX_PATH];
public:
	WindowSize() :zoom_multiplier(1.0f), alpha(1.0f), szUsingLayout()
	{
	}
	void SetOriginalSize(HWND hwnd, int w, int h)
	{
		m_hWindow = hwnd;
		owidth = w;
		oheight = h;
		SetZoom(GetSystemZoom());
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
		km.ResetKeySize(szUsingLayout);
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
	void SetLayoutFile(LPCTSTR file)
	{
		lstrcpy(szUsingLayout, file);
		S.SetZoom(GetSystemZoom());
		Gdiplus::Rect r = GetINIRect(szUsingLayout, TEXT("keyboardoff"), 0, 0, 106, 25);
		SetOriginalSize(m_hWindow, r.Width, r.Height);
	}
	void ChooseLayoutFile()
	{
		TCHAR fileName[MAX_PATH];
		OPENFILENAME ofn{};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = m_hWindow;
		ofn.lpstrFilter = TEXT("配置设置\0*.ini\0所有文件\0*\0\0");
		ofn.lpstrFile = szUsingLayout;
		ofn.lpstrTitle = TEXT("选择布局文件");
		ofn.nMaxFile = ARRAYSIZE(szUsingLayout);
		ofn.lpstrFileTitle = fileName;
		ofn.nMaxFileTitle = ARRAYSIZE(fileName);
		ofn.Flags = OFN_FILEMUSTEXIST;
		ofn.lpstrDefExt = TEXT("ini");
		TCHAR* pl = wcsrchr(szUsingLayout, '\\');
		lstrcpy(fileName, pl ? pl + 1 : szUsingLayout);
		if (GetOpenFileName(&ofn))
			SetLayoutFile(ofn.lpstrFile);
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
	Gdiplus::Bitmap bmpBack(km.bmpBack->GetWidth(), km.bmpBack->GetHeight());
	Gdiplus::Graphics grFront(hDC), grBack(&bmpBack);
	grBack.DrawImage(km.bmpBack, 0, 0, bmpBack.GetWidth(), bmpBack.GetHeight());
	grFront.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	for (int i = 0; i < ARRAYSIZE(km.keys); i++)
	{
		if (km.rkeys[i].Width&&km.rkeys[i].Height)
			km.DrawKey(grBack, i);
	}
	grFront.DrawImage(&bmpBack, clientRect.left, clientRect.top, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
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
		if (GetWindowLongPtr(h, GWL_STYLE)&WS_CAPTION)
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
			MessageBox(h, TEXT("本程序可以监视键盘的按下状态并实时显示。\n* 双击窗口可以隐藏边框。\n\nBy lxfly2000"),
				TEXT("按键显示程序"), NULL);
			break;
		case ID_MENU_MINIMIZE:ShowWindow(h, SW_MINIMIZE); break;
		case ID_MENU_CLOSE:PostQuitMessage(0); break;
		case IDM_ZOOM_IN:ws.SetZoom(min(ws.GetZoom() + 0.25f, 8.0f)); break;
		case IDM_ZOOM_OUT:ws.SetZoom(max(ws.GetZoom() - 0.25f, 0.25f)); break;
		case IDM_ALPHA_INCREASE:ws.SetAlpha(min(ws.GetAlpha() + 0.25f, 1.0f)); break;
		case IDM_ALPHA_DECREASE:ws.SetAlpha(max(ws.GetAlpha() - 0.25f, 0.25f)); break;
		case ID_MENU_LAYOUT:ws.ChooseLayoutFile(); break;
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
			PKBDLLHOOKSTRUCT pk = (PKBDLLHOOKSTRUCT)l;
			if ((pk->vkCode == VK_RETURN) && (pk->flags&LLKHF_EXTENDED))
				pk->vkCode = VK_SEPARATOR;
			km.SetKey(pk->vkCode, !(pk->flags&LLKHF_UP), hWindow);
			break;
		}
	return CallNextHookEx(NULL, c, w, l);
}

ATOM RegWindowClass(HINSTANCE hI, WNDPROC fP, LPCTSTR cn)
{
	WNDCLASSEX w = { sizeof w,CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,fP,0,0,hI,LoadIcon(hI,MAKEINTRESOURCE(IDI_ICON_APP)),
		LoadCursor(NULL,IDC_ARROW),(HBRUSH)GetStockObject(BLACK_BRUSH),NULL,cn,NULL };
	return RegisterClassEx(&w);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR szCmdLine, int iCmdShow)
{
	Gdiplus::GdiplusStartupInput gsi;
	ULONG_PTR gpToken;
	Gdiplus::GdiplusStartup(&gpToken, &gsi, NULL);

	km.ResetKeySize(TEXT(CONFIG_FILE));
	//生成窗口
	PCTSTR wcl_name = TEXT("KeyWindow");
	if (!RegWindowClass(hInstance, ProcessMessage, wcl_name))
		return -1;
	Gdiplus::Rect r = GetINIRect(TEXT(CONFIG_FILE), TEXT("keyboardoff"), 0, 0, 106, 25);
	LONG ww = S.P(r.Width), wh = S.P(r.Height);
	RECT rWindow = { 0,0,ww,wh };
	ws.SetStyle(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
	ws.SetExStyle(WS_EX_APPWINDOW | WS_EX_TOPMOST | WS_EX_LAYERED);
	AdjustWindowRectEx(&rWindow, ws.GetStyle(), FALSE, ws.GetExStyle());
	hWindow = CreateWindowEx(ws.GetExStyle(), wcl_name, TEXT("按键"), ws.GetStyle(),
		CW_USEDEFAULT, CW_USEDEFAULT, rWindow.right - rWindow.left, rWindow.bottom - rWindow.top, NULL, NULL, hInstance, NULL);
	ShowWindow(hWindow, iCmdShow);
	hMenu = GetSubMenu(LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU_APP)), 0);
	ws.SetOriginalSize(hWindow, ww, wh);
	ws.SetLayoutFile(TEXT(CONFIG_FILE));
	HACCEL haApp = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_APP));
	//获取钩子
	HHOOK hkKey = SetWindowsHookEx(WH_KEYBOARD_LL, ProcessHook, hInstance, 0);
	//消息循环
	MSG m;
	while (GetMessage(&m, NULL, 0, 0))
	{
		TranslateAccelerator(hWindow, haApp, &m);
		TranslateMessage(&m);
		DispatchMessage(&m);
	}
	//释放钩子
	UnhookWindowsHookEx(hkKey);
	km.ReleaseImg();
	Gdiplus::GdiplusShutdown(gpToken);
	return m.wParam;
}