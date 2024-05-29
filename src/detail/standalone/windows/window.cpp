#include "detail/standalone/entry.h"
#include "window.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
Window::Window()
{
  std::wstring clapName{widen(HOSTED_CLAP_NAME)};

  ::MessageBoxW(nullptr, clapName.c_str(), clapName.c_str(), MB_OK);

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

  // auto hMenu{::GetSystemMenu(m_hwnd, FALSE)};

  // MENUITEMINFO seperator{sizeof(MENUITEMINFO)};
  // seperator.fMask = MIIM_FTYPE;
  // seperator.fType = MFT_SEPARATOR;

  // MENUITEMINFO audioIn{sizeof(MENUITEMINFO)};
  // audioIn.fMask = MIIM_STRING | MIIM_ID;
  // audioIn.wID = IDM_SETTINGS;
  // audioIn.dwTypeData = const_cast<LPSTR>("Settings");

  // MENUITEMINFO saveState{sizeof(MENUITEMINFO)};
  // saveState.fMask = MIIM_STRING | MIIM_ID;
  // saveState.wID = IDM_SAVE_STATE;
  // saveState.dwTypeData = const_cast<LPSTR>("Save state...");

  // MENUITEMINFO loadState{sizeof(MENUITEMINFO)};
  // loadState.fMask = MIIM_STRING | MIIM_ID;
  // loadState.wID = IDM_LOAD_STATE;
  // loadState.dwTypeData = const_cast<LPSTR>("Load state...");

  // MENUITEMINFO resetState{sizeof(MENUITEMINFO)};
  // resetState.fMask = MIIM_STRING | MIIM_ID;
  // resetState.wID = IDM_RESET_STATE;
  // resetState.dwTypeData = const_cast<LPSTR>("Reset state...");

  // if (hMenu != INVALID_HANDLE_VALUE)
  // {
  //   ::InsertMenuItem(hMenu, 1, TRUE, &seperator);
  //   ::InsertMenuItem(hMenu, 2, TRUE, &audioIn);
  //   ::InsertMenuItem(hMenu, 3, TRUE, &seperator);
  //   ::InsertMenuItem(hMenu, 4, TRUE, &saveState);
  //   ::InsertMenuItem(hMenu, 5, TRUE, &loadState);
  //   ::InsertMenuItem(hMenu, 6, TRUE, &resetState);
  //   ::InsertMenuItem(hMenu, 7, TRUE, &seperator);
  // }
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

  auto dpi{::GetDpiForWindow(hWnd)};
  auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  ui->set_scale(p, scaleFactor);

  auto bounds{(RECT*)lParam};
  ::SetWindowPos(hWnd, nullptr, bounds->left, bounds->top, (bounds->right - bounds->left),
                 (bounds->bottom - bounds->top), SWP_NOZORDER | SWP_NOACTIVATE);

  return 0;
}

int Window::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  auto dpi{::GetDpiForWindow(hWnd)};
  auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  if (ui->can_resize(p))
  {
    RECT r{0, 0, 0, 0};
    ::GetClientRect(hWnd, &r);
    uint32_t w = (r.right - r.left);
    uint32_t h = (r.bottom - r.top);
    ui->adjust_size(p, &w, &h);
    ui->set_size(p, w, h);
  }

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
