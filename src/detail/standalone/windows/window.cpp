#include "detail/standalone/entry.h"
#include "window.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
Window::Window()
{
  std::wstring clapName{widen(HOSTED_CLAP_NAME)};

  WNDCLASSEXW wcex{sizeof(WNDCLASSEXW)};
  wcex.lpszClassName = clapName.c_str();
  wcex.lpszMenuName = clapName.c_str();
  wcex.lpfnWndProc = Window::WndProc;
  wcex.style = 0;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
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

Window::~Window()
{
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Window* pWindow = InstanceFromWndProc<Window, &Window::m_hwnd>(hWnd, uMsg, lParam);

  if (pWindow)
  {
    switch (uMsg)
    {
      case WM_CREATE:
        return pWindow->OnCreate(hWnd, uMsg, wParam, lParam);
      case WM_CLOSE:
        return pWindow->OnClose(hWnd, uMsg, wParam, lParam);
      case WM_DESTROY:
        return pWindow->OnDestroy(hWnd, uMsg, wParam, lParam);
      case WM_DPICHANGED:
        return pWindow->OnDpiChanged(hWnd, uMsg, wParam, lParam);
      case WM_WINDOWPOSCHANGED:
        return pWindow->OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int Window::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // ::MessageBoxW(nullptr, L"OnCreate", L"", MB_OK);

  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  ui->set_scale(
      p, static_cast<float>(::GetDpiForWindow(hWnd)) / static_cast<float>(USER_DEFAULT_SCREEN_DPI));

  if (ui->can_resize(p))
  {
    // We can check here if we had a previous size but we aren't saving state yet
  }
  else
  {
    ::SetWindowLongPtrW(m_hwnd, GWL_STYLE,
                        ::GetWindowLongPtrW(m_hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_OVERLAPPED |
                            WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
  }

  uint32_t w{0};
  uint32_t h{0};
  ui->get_size(p, &w, &h);

  RECT r{0, 0, 0, 0};
  r.right = w;
  r.bottom = h;

  ::AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, 0, 0, ::GetDpiForWindow(hWnd));

  ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

  ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);

  return 0;
}

int Window::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  ::DestroyWindow(hWnd);

  return 0;
}

int Window::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};

  if (plugin && plugin->_ext._gui)
  {
    plugin->_ext._gui->hide(plugin->_plugin);
    plugin->_ext._gui->destroy(plugin->_plugin);
  }

  ::PostQuitMessage(0);

  return 0;
}

int Window::OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  ui->set_scale(
      p, static_cast<float>(::GetDpiForWindow(hWnd)) / static_cast<float>(USER_DEFAULT_SCREEN_DPI));

  uint32_t w{0};
  uint32_t h{0};
  ui->get_size(p, &w, &h);

  RECT r{0, 0, 0, 0};
  r.right = w;
  r.bottom = h;

  ::AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, 0, 0, ::GetDpiForWindow(hWnd));

  ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

  return 0;
}

int Window::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  // auto ui{plugin->_ext._gui};
  // auto p{plugin->_plugin};

  // auto dpi{::GetDpiForWindow(hWnd)};
  // auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  // if (ui->can_resize(p))
  // {
  //   RECT r{0, 0, 0, 0};
  //   ::GetClientRect(hWnd, &r);
  //   uint32_t w = (r.right - r.left);
  //   uint32_t h = (r.bottom - r.top);
  //   ui->adjust_size(p, &w, &h);
  //   ui->set_size(p, w, h);
  // }

  return 0;
}

std::string Window::narrow(std::wstring wstring)
{
  if (wstring.empty()) return {};

  auto safeSize{safe_size<size_t, int>(wstring.length())};

  auto length{::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, wstring.data(),
                                    safeSize, nullptr, 0, nullptr, nullptr)};

  std::string utf8(length, 0);

  if (::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, wstring.data(),
                            safeSize, utf8.data(), length, nullptr, nullptr) > 0)
    return utf8;

  else
    return {};
}

std::wstring Window::widen(std::string string)
{
  if (string.empty()) return {};

  auto safeSize{safe_size<size_t, int>(string.length())};

  auto length{::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.data(), safeSize, nullptr, 0)};

  std::wstring utf16(length, 0);

  if (::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.data(), safeSize, utf16.data(),
                            length) > 0)
    return utf16;

  else
    return {};
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
