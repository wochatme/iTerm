#ifndef PUTTY_WINTERM_H
#define PUTTY_WINTERM_H
#if 0
#define WM_PUTTY_NOTIFY		(WM_USER + 100)

extern bool terminal_has_focus;

int PuTTY_Init(HINSTANCE hInstance);

int PuTTY_Term();

int PuTTY_MessageLoop(BOOL bHasMessage, MSG* pMSG);

int PuTTY_AttachWindow(HWND hWnd, HWND hWndParent, int heightTab);

LRESULT PuTTY_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* bHandled);

void PuTTY_EnterSizing(void);
void PuTTY_ExitSizing(void);
#endif 

#endif /* PUTTY_WINTERM_H */