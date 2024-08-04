// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

/* From MSDN: In the WM_SYSCOMMAND message, the four low-order bits of
 * wParam are used by Windows, and should be masked off, so we shouldn't
 * attempt to store information in them. Hence all these identifiers have
 * the low 4 bits clear. Also, identifiers should < 0xF000. */

#define IDM_NEW_WINDOW		0x0010
#define IDM_NEW_SESSION		0x0020
#define IDM_COPY_ALL		0x0030
#define IDM_TTY_SETTING		0x0040
#define	IDM_CLEAR_SB		0x0050
#define IDM_ABOUT_ITERM		0x0060


class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CTTYView m_viewTTY;
	CTabView m_viewTab;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return FALSE;
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PUTTY_NOTIFY, OnPuTTYNotify)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnEnterSizeMove)
		MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

	LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		// handled, no background painting needed
		return 1;
	}

	LRESULT OnPuTTYNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		wchar_t title[128] = { 0 };
		swprintf((wchar_t*)title, 128, L"iTerm - [%d : %d]", (int)wParam, (int)lParam);
		SetWindowTextW(title);

		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HMENU m;

		m_viewTTY.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VSCROLL);
		m_viewTab.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

		m_viewTab.ModifyStyle(0,
			CTCS_TOOLTIPS |
			CTCS_SCROLL |
			CTCS_CLOSEBUTTON |
			CTCS_DRAGREARRANGE
		);

		m_viewTab.InsertItem(0, L"cmd.exe[00]", -1, L"command line", true);

		m = GetSystemMenu(FALSE);
		AppendMenu(m, MF_SEPARATOR, 0, 0);
		AppendMenu(m, MF_ENABLED, IDM_NEW_WINDOW, L"New Window");
		AppendMenu(m, MF_ENABLED, IDM_NEW_SESSION, L"Ne&w Session");
		AppendMenu(m, MF_ENABLED, IDM_TTY_SETTING, L"TTY Settings...");
		AppendMenu(m, MF_ENABLED, IDM_COPY_ALL, L"C&opy All to Clipboard");
		AppendMenu(m, MF_ENABLED, IDM_CLEAR_SB, L"C&lear Scrollback");
		AppendMenu(m, MF_SEPARATOR, 0, 0);
		AppendMenu(m, MF_ENABLED, IDM_ABOUT_ITERM, L"&About iTerm");

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		ShowCursor(TRUE);
		bHandled = FALSE;
		return 1;
	}

	LRESULT OnEnterSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PuTTY_EnterSizing();
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnExitSizeMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		PuTTY_ExitSizing();
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSetFocus(UINT message, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_viewTTY.SetFocus();
		terminal_has_focus = true;

		bHandled = FALSE;
		return 0;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		RECT rect = {};
		GetClientRect(&rect);

		int bottom = rect.bottom;

		if (m_viewTab.IsWindow())
		{
			rect.bottom = rect.top + TTYTAB_WINDOW_HEIGHT;
			::SetWindowPos(m_viewTab, NULL, rect.left, rect.top,
				rect.right - rect.left, TTYTAB_WINDOW_HEIGHT,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}

		if (m_viewTTY.IsWindow())
		{
			rect.top += TTYTAB_WINDOW_HEIGHT;
			rect.bottom = bottom;
			::SetWindowPos(m_viewTTY, NULL, rect.left, rect.top,
				rect.right - rect.left, rect.bottom - rect.top,
				SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return 0;
	}

	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR)
		{
			if (pNMHDR->hwndFrom == m_viewTab)
			{
				NMCTCITEM* pNMCTCITEM = (NMCTCITEM*)lParam;
				
				int idx = pNMCTCITEM->iItem;
				int count = m_viewTab.GetItemCount();
#if 0
				int sel = m_viewTab.GetCurSel();
#endif 
				m_viewTTY.SetFocus();

				switch (pNMHDR->code)
				{
				case NM_CLICK:
					if (idx >= 0 && idx < count)
					{
						XCustomTabItem* pItem = m_viewTab.GetItem(idx);
						if (pItem)
						{
							void* handle = pItem->GetPrivateData();
							BOOL bRet = PuTTY_SwitchSession(handle);
							if (bRet)
							{
								return 0;
							}
							else
							{
								return 1;
							}
						}
					}
					break;
				case CTCN_CLOSE:
					if (idx >= 0 && idx < count && count > 1)
					{
						int choice = MessageBox(L"Are you sure to close this session?", L"Close Session", MB_YESNO);
						if (choice == IDYES)
						{
							XCustomTabItem* pItem = m_viewTab.GetItem(idx);
							if (pItem)
							{
								void* handle = pItem->GetPrivateData();
								int nRet = PuTTY_RemoveSession(handle);
								if (nRet == SELECT_RIGHTSIDE || nRet == SELECT_LEFTSIDE)
								{
									m_viewTab.DeleteItem(idx, true, false);
								}
							}
						}
					}
					break;
				default:
					bHandled = FALSE;
				}
			}
		}
		return 0;
	}

	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		switch (wParam & ~0xF) /* low 4 bits reserved to Windows */
		{
		case IDM_NEW_SESSION:
		{
			void* term = PuTTY_NewSession();
			if (term)
			{
				wchar_t title[128] = { 0 };
				int idx = m_viewTab.GetItemCount();

				swprintf((wchar_t*)title, 128, L"cmd.exe[%02d]", idx);

				m_viewTab.InsertItem(idx, title, -1, L"command line", true);

				XCustomTabItem* pIterm = m_viewTab.GetItem(idx);
				if (pIterm)
				{
					pIterm->SetPrivateData(term);
				}
				m_viewTTY.SetFocus();
			}
			else
			{
				MessageBox(L"You can open 60 tabs at maximum", L"Maximum Tabs Are Reached", MB_OK);
			}
		}
			break;
		case IDM_NEW_WINDOW:
		{
			WCHAR exeFile[MAX_PATH + 1] = { 0 };
			DWORD len = GetModuleFileNameW(HINST_THISCOMPONENT, exeFile, MAX_PATH);
			if (len)
			{
				STARTUPINFOW si = { 0 };
				PROCESS_INFORMATION pi = { 0 };
				si.cb = sizeof(STARTUPINFOW);
				CreateProcessW(NULL, exeFile, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
			}
		}
			break;
#if 0
		case IDM_ABOUT:
		{
			CAboutDlg dlg;
			dlg.DoModal();
		}
			break;
#endif 
		default:
			bHandled = FALSE;
			break;
		}
		return 0;
	}

	HWND GetTTYWindowHandle()
	{
		return m_viewTTY.m_hWnd;
	}

	void AttachTerminalHandle(void* term)
	{
		int idx = m_viewTab.GetCurSel();
		XCustomTabItem* pIterm = m_viewTab.GetItem(idx);
		if (pIterm)
		{
			pIterm->SetPrivateData(term);
		}
	}
};
