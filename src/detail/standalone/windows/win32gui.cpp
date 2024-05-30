#include "win32gui.h"
#include "helpers.h"
#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
void Win32Gui::initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah)
{
  sah->win32Gui = this;

  sah->onRequestResize = [=](uint32_t width, uint32_t height) { return setWindowSize(width, height); };
}

void Win32Gui::setPlugin(std::shared_ptr<Clap::Plugin> p)
{
  m_plugin = p;
}

void Win32Gui::createWindow()
{
  std::wstring clapName{widen(OUTPUT_NAME)};

  WNDCLASSEXW wcex{sizeof(WNDCLASSEXW)};
  wcex.lpszClassName = clapName.c_str();
  wcex.lpszMenuName = clapName.c_str();
  wcex.lpfnWndProc = Win32Gui::wndProc;
  wcex.style = 0;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(intptr_t);
  wcex.hInstance = ::GetModuleHandleW(nullptr);
  wcex.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
  wcex.hCursor = reinterpret_cast<HCURSOR>(::LoadImageW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW),
                                                        IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
  wcex.hIcon = reinterpret_cast<HICON>(::LoadImageW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION),
                                                    IMAGE_ICON, 0, 0,
                                                    LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));
  wcex.hIconSm = reinterpret_cast<HICON>(
      ::LoadImageW(nullptr, reinterpret_cast<LPCWSTR>(IDI_APPLICATION), IMAGE_ICON, 0, 0,
                   LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));

  ::RegisterClassExW(&wcex);

  ::CreateWindowExW(0, clapName.c_str(), clapName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                    ::GetModuleHandleW(nullptr), this);

  auto hMenu{::GetSystemMenu(m_hwnd, FALSE)};

  MENUITEMINFOW seperator{sizeof(MENUITEMINFOW)};
  seperator.fMask = MIIM_FTYPE;
  seperator.fType = MFT_SEPARATOR;

  MENUITEMINFOW audioIn{sizeof(MENUITEMINFOW)};
  audioIn.fMask = MIIM_STRING | MIIM_ID;
  audioIn.wID = IDM_SETTINGS;
  audioIn.dwTypeData = const_cast<LPWSTR>(L"Settings");

  MENUITEMINFOW saveState{sizeof(MENUITEMINFOW)};
  saveState.fMask = MIIM_STRING | MIIM_ID;
  saveState.wID = IDM_SAVE_STATE;
  saveState.dwTypeData = const_cast<LPWSTR>(L"Save state...");

  MENUITEMINFOW loadState{sizeof(MENUITEMINFOW)};
  loadState.fMask = MIIM_STRING | MIIM_ID;
  loadState.wID = IDM_LOAD_STATE;
  loadState.dwTypeData = const_cast<LPWSTR>(L"Load state...");

  MENUITEMINFOW resetState{sizeof(MENUITEMINFOW)};
  resetState.fMask = MIIM_STRING | MIIM_ID;
  resetState.wID = IDM_RESET_STATE;
  resetState.dwTypeData = const_cast<LPWSTR>(L"Reset state...");

  if (hMenu != INVALID_HANDLE_VALUE)
  {
    ::InsertMenuItemW(hMenu, 1, TRUE, &seperator);
    ::InsertMenuItemW(hMenu, 2, TRUE, &audioIn);
    ::InsertMenuItemW(hMenu, 3, TRUE, &seperator);
    ::InsertMenuItemW(hMenu, 4, TRUE, &saveState);
    ::InsertMenuItemW(hMenu, 5, TRUE, &loadState);
    ::InsertMenuItemW(hMenu, 6, TRUE, &resetState);
    ::InsertMenuItemW(hMenu, 7, TRUE, &seperator);
  }
}

void Win32Gui::setupPlugin()
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  if (pluginGui)
  {
    if (!pluginGui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false))
    {
      LOG << "NO WIN32 " << std::endl;
    }

    pluginGui->create(plugin, CLAP_WINDOW_API_WIN32, false);
    clap_window clapWindow;
    clapWindow.api = CLAP_WINDOW_API_WIN32;
    clapWindow.win32 = static_cast<void*>(m_hwnd);

    pluginGui->set_scale(plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                     static_cast<float>(USER_DEFAULT_SCREEN_DPI));

    // We can check here if we had a previous size but we aren't saving state yet
    if (pluginGui->can_resize(plugin))
    {
    }
    else
    {
      ::SetWindowLongPtrW(m_hwnd, GWL_STYLE,
                          ::GetWindowLongPtrW(m_hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_OVERLAPPED |
                              WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    }

    uint32_t width{0};
    uint32_t height{0};
    pluginGui->get_size(plugin, &width, &height);

    setWindowSize(width, height);

    pluginGui->set_parent(plugin, &clapWindow);

    pluginGui->show(plugin);

    ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
  }
}

bool Win32Gui::setWindowSize(uint32_t width, uint32_t height)
{
  RECT r{0, 0, 0, 0};
  r.right = width;
  r.bottom = height;

  if (m_hwnd)
  {
    ::AdjustWindowRectExForDpi(&r, ::GetWindowLongPtrW(m_hwnd, GWL_STYLE), 0, 0,
                               ::GetDpiForWindow(m_hwnd));

    ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
  }

  return true;
}

void Win32Gui::runLoop()
{
  MSG msg{};
  int r{0};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      break;
    }

    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }
}

LRESULT CALLBACK Win32Gui::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Win32Gui* pWin32Gui = instance_from_wnd_proc<Win32Gui>(hWnd, uMsg, lParam);

  if (pWin32Gui)
  {
    switch (uMsg)
    {
      case WM_DESTROY:
        return pWin32Gui->onDestroy(hWnd, uMsg, wParam, lParam);
      case WM_DPICHANGED:
        return pWin32Gui->onDpiChanged(hWnd, uMsg, wParam, lParam);
      case WM_WINDOWPOSCHANGED:
        return pWin32Gui->onWindowPosChanged(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int Win32Gui::onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  if (m_plugin && pluginGui)
  {
    pluginGui->hide(plugin);
    pluginGui->destroy(plugin);
    m_plugin = nullptr;
  }

  ::PostQuitMessage(0);

  return 0;
}

int Win32Gui::onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  if (m_plugin && pluginGui)
  {
    pluginGui->set_scale(plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                     static_cast<float>(USER_DEFAULT_SCREEN_DPI));
  }

  return 0;
}

int Win32Gui::onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  if (pluginGui)
  {
    if (pluginGui->can_resize(plugin))
    {
      RECT r{0, 0, 0, 0};
      ::GetClientRect(hWnd, &r);

      uint32_t w = (r.right - r.left);
      uint32_t h = (r.bottom - r.top);

      pluginGui->adjust_size(plugin, &w, &h);
      pluginGui->set_size(plugin, w, h);
    }
  }

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
