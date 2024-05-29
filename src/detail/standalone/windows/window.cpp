#include "window.h"
#include "helper.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
Window::Window()
{
  std::string clapName{HOSTED_CLAP_NAME};

  WNDCLASSEX wcex{sizeof(WNDCLASSEX)};
  wcex.lpszClassName = clapName.c_str();
  wcex.lpszMenuName = clapName.c_str();
  wcex.lpfnWndProc = Window::WndProc;
  wcex.style = 0;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = ::GetModuleHandle(nullptr);
  wcex.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(BLACK_BRUSH));
  wcex.hCursor = reinterpret_cast<HCURSOR>(::LoadImage(nullptr, reinterpret_cast<LPCSTR>(IDC_ARROW),
                                                       IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
  wcex.hIcon =
      reinterpret_cast<HICON>(::LoadImage(nullptr, reinterpret_cast<LPCSTR>(IDI_APPLICATION), IMAGE_ICON,
                                          0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));
  wcex.hIconSm =
      reinterpret_cast<HICON>(::LoadImage(nullptr, reinterpret_cast<LPCSTR>(IDI_APPLICATION), IMAGE_ICON,
                                          0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));

  ::RegisterClassEx(&wcex);

  ::CreateWindowEx(0, clapName.c_str(), clapName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                   ::GetModuleHandle(nullptr), this);

  auto hMenu{::GetSystemMenu(m_hwnd, FALSE)};

  MENUITEMINFO seperator{sizeof(MENUITEMINFO)};
  seperator.fMask = MIIM_FTYPE;
  seperator.fType = MFT_SEPARATOR;

  MENUITEMINFO audioIn{sizeof(MENUITEMINFO)};
  audioIn.fMask = MIIM_STRING | MIIM_ID;
  audioIn.wID = IDM_SETTINGS;
  audioIn.dwTypeData = const_cast<LPSTR>("Settings");

  MENUITEMINFO saveState{sizeof(MENUITEMINFO)};
  saveState.fMask = MIIM_STRING | MIIM_ID;
  saveState.wID = IDM_SAVE_STATE;
  saveState.dwTypeData = const_cast<LPSTR>("Save state...");

  MENUITEMINFO loadState{sizeof(MENUITEMINFO)};
  loadState.fMask = MIIM_STRING | MIIM_ID;
  loadState.wID = IDM_LOAD_STATE;
  loadState.dwTypeData = const_cast<LPSTR>("Load state...");

  MENUITEMINFO resetState{sizeof(MENUITEMINFO)};
  resetState.fMask = MIIM_STRING | MIIM_ID;
  resetState.wID = IDM_RESET_STATE;
  resetState.dwTypeData = const_cast<LPSTR>("Reset state...");

  if (hMenu != INVALID_HANDLE_VALUE)
  {
    ::InsertMenuItem(hMenu, 1, TRUE, &seperator);
    ::InsertMenuItem(hMenu, 2, TRUE, &audioIn);
    ::InsertMenuItem(hMenu, 3, TRUE, &seperator);
    ::InsertMenuItem(hMenu, 4, TRUE, &saveState);
    ::InsertMenuItem(hMenu, 5, TRUE, &loadState);
    ::InsertMenuItem(hMenu, 6, TRUE, &resetState);
    ::InsertMenuItem(hMenu, 7, TRUE, &seperator);
  }
}

Window::~Window()
{
}

LRESULT CALLBACK Window::WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
  Window* pWindow = InstanceFromWndProc<Window, &Window::m_hwnd>(h, m, l);

  if (pWindow)
  {
    switch (m)
    {
      case WM_CLOSE:
        return pWindow->OnClose(h, m, w, l);
      case WM_DESTROY:
        return pWindow->OnDestroy(h, m, w, l);
      case WM_DPICHANGED:
        return pWindow->OnDpiChanged(h, m, w, l);
      case WM_KEYDOWN:
        return pWindow->OnKeyDown(h, m, w, l);
      case WM_WINDOWPOSCHANGED:
        return pWindow->OnWindowPosChanged(h, m, w, l);
    }
  }

  return ::DefWindowProc(h, m, w, l);
}

int Window::OnClose(HWND h, UINT m, WPARAM w, LPARAM l)
{
  ::DestroyWindow(h);

  return 0;
}

int Window::OnDestroy(HWND h, UINT m, WPARAM w, LPARAM l)
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

int Window::OnDpiChanged(HWND h, UINT m, WPARAM w, LPARAM l)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  auto dpi{::GetDpiForWindow(h)};
  auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  ui->set_scale(p, scaleFactor);

  auto bounds{(RECT*)l};
  ::SetWindowPos(h, nullptr, bounds->left, bounds->top, (bounds->right - bounds->left),
                 (bounds->bottom - bounds->top), SWP_NOZORDER | SWP_NOACTIVATE);

  return 0;
}

int Window::OnKeyDown(HWND h, UINT m, WPARAM w, LPARAM l)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  switch (w)
  {
    case VK_F11:
    {
      if (ui->can_resize(p)) fullscreen();

      break;
    }

    default:
      return 0;
  }

  return 0;
}

int Window::OnWindowPosChanged(HWND h, UINT m, WPARAM w, LPARAM l)
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  auto dpi{::GetDpiForWindow(h)};
  auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  if (ui->can_resize(p))
  {
    RECT r{0, 0, 0, 0};
    ::GetClientRect(h, &r);
    uint32_t w = (r.right - r.left);
    uint32_t h = (r.bottom - r.top);
    ui->adjust_size(p, &w, &h);
    ui->set_size(p, w, h);
  }

  return 0;
}

bool Window::fullscreen()
{
  auto plugin{freeaudio::clap_wrapper::standalone::getMainPlugin()};
  auto ui{plugin->_ext._gui};
  auto p{plugin->_plugin};

  static RECT pos;

  auto style{::GetWindowLongPtr(m_hwnd, GWL_STYLE)};

  if (style & WS_OVERLAPPEDWINDOW)
  {
    MONITORINFO mi = {sizeof(mi)};
    ::GetWindowRect(m_hwnd, &pos);
    if (::GetMonitorInfo(::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &mi))
    {
      ::SetWindowLongPtr(m_hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
      ::SetWindowPos(m_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
    }

    return true;
  }

  else
  {
    ::SetWindowLongPtr(m_hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    ::SetWindowPos(m_hwnd, nullptr, pos.left, pos.top, (pos.right - pos.left), (pos.bottom - pos.top),
                   SWP_FRAMECHANGED);

    return false;
  }
}
}
