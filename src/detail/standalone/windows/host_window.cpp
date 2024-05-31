#include "detail/standalone/entry.h"
#include "host_window.h"
#include "helpers.h"

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
HostWindow::HostWindow() : m_standaloneHost{freeaudio::clap_wrapper::standalone::getStandaloneHost()}
{
  std::wstring windowName{widen(OUTPUT_NAME)};

  auto icon{loadIconFromResource()};

  ::WNDCLASSEXW wcex{sizeof(::WNDCLASSEXW)};
  wcex.lpszClassName = windowName.c_str();
  wcex.lpszMenuName = windowName.c_str();
  wcex.lpfnWndProc = HostWindow::wndProc;
  wcex.style = 0;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(intptr_t);
  wcex.hInstance = ::GetModuleHandleW(nullptr);
  wcex.hbrBackground = reinterpret_cast<::HBRUSH>(::GetStockObject(BLACK_BRUSH));
  wcex.hCursor = reinterpret_cast<::HCURSOR>(::LoadImageW(
      nullptr, reinterpret_cast<::LPCWSTR>(IDC_ARROW), IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
  wcex.hIcon = icon ? icon
                    : reinterpret_cast<::HICON>(
                          ::LoadImageW(nullptr, reinterpret_cast<::LPCWSTR>(IDI_APPLICATION), IMAGE_ICON,
                                       0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));
  wcex.hIconSm = icon ? icon
                      : reinterpret_cast<::HICON>(::LoadImageW(
                            nullptr, reinterpret_cast<::LPCWSTR>(IDI_APPLICATION), IMAGE_ICON, 0, 0,
                            LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_SHARED));

  ::RegisterClassExW(&wcex);

  ::CreateWindowExW(0, windowName.c_str(), windowName.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
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

  // ::EnableMenuItem(hMenu, IDM_RESET_STATE, MF_DISABLED);

  m_standaloneHost->onRequestResize = [this](uint32_t width, uint32_t height)
  { return setWindowSize(width, height); };
}

clap_window HostWindow::createClapWindow()
{
  clap_window clapWindow;
  clapWindow.api = CLAP_WINDOW_API_WIN32;
  clapWindow.win32 = static_cast<void*>(m_hwnd);

  return clapWindow;
}

void HostWindow::setPlugin(std::shared_ptr<Clap::Plugin> plugin)
{
  m_plugin = plugin;
}

void HostWindow::setupPlugin()
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    if (!pluginGui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false))
    {
      errorBox("CLAP_WINDOW_API_WIN32 is not supported");
    }

    pluginGui->create(plugin, CLAP_WINDOW_API_WIN32, false);

    pluginGui->set_scale(plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                     static_cast<float>(USER_DEFAULT_SCREEN_DPI));

    uint32_t width{0};
    uint32_t height{0};

    if (pluginGui->can_resize(plugin))
    {
      // if resizable and has known size from previous session:
      // We should load size here, width = previousWidth, height = previousHeight instead of hardcoded values:
      // width = 1000;
      // height = 1000;
      // pluginGui->adjust_size(plugin, &width, &height);
      // pluginGui->set_size(plugin, width, height);
    }
    else
    {
      // We can't resize, so disable WS_THICKFRAME and WS_MAXIMIZEBOX
      ::SetWindowLongPtrW(m_hwnd, GWL_STYLE,
                          ::GetWindowLongPtrW(m_hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_OVERLAPPED |
                              WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    }

    pluginGui->get_size(plugin, &width, &height);

    setWindowSize(width, height);

    ::MONITORINFO mi{sizeof(::MONITORINFO)};
    ::GetMonitorInfoA(::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &mi);

    auto x = (static_cast<int>(mi.rcWork.right - mi.rcWork.left) - width) / 2;
    auto y = (static_cast<int>(mi.rcWork.bottom - mi.rcWork.top) - height) / 2;

    ::SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE);

    auto clapWindow{createClapWindow()};
    pluginGui->set_parent(plugin, &clapWindow);

    pluginGui->show(plugin);
  }
}

bool HostWindow::setWindowVisibility(bool visible)
{
  ::ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);

  return visible ? true : false;
}

bool HostWindow::getWindowVisibility()
{
  return ::IsWindowVisible(m_hwnd);
}

bool HostWindow::setWindowSize(uint32_t width, uint32_t height)
{
  RECT r{0, 0, 0, 0};
  r.right = width;
  r.bottom = height;

  if (m_hwnd)
  {
    ::AdjustWindowRectExForDpi(&r, ::GetWindowLongPtrW(m_hwnd, GWL_STYLE), ::GetMenu(m_hwnd) != nullptr,
                               ::GetWindowLongPtrW(m_hwnd, GWL_EXSTYLE), ::GetDpiForWindow(m_hwnd));

    ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

    return true;
  }

  else
  {
    return false;
  }
}

LRESULT CALLBACK HostWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{instance_from_wnd_proc<HostWindow>(hWnd, uMsg, lParam)};

  if (self && self->m_plugin)
  {
    switch (uMsg)
    {
      case WM_DPICHANGED:
        return self->onDpiChanged(hWnd, uMsg, wParam, lParam);
      case WM_WINDOWPOSCHANGED:
        return self->onWindowPosChanged(hWnd, uMsg, wParam, lParam);
      case WM_KEYDOWN:
        return self->onKeyDown(hWnd, uMsg, wParam, lParam);
      case WM_SYSCOMMAND:
        return self->onSysCommand(hWnd, uMsg, wParam, lParam);
      case WM_DESTROY:
        return self->onDestroy(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int HostWindow::onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    pluginGui->set_scale(plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                     static_cast<float>(USER_DEFAULT_SCREEN_DPI));
  }

  return 0;
}

int HostWindow::onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    if (pluginGui->can_resize(plugin))
    {
      RECT r{0, 0, 0, 0};
      ::GetClientRect(hWnd, &r);

      uint32_t width = (r.right - r.left);
      uint32_t height = (r.bottom - r.top);

      pluginGui->adjust_size(plugin, &width, &height);
      pluginGui->set_size(plugin, width, height);

      // We can constrain aspect ratio like this, but it's very janky...
      // pluginGui->get_size(plugin, &width, &height);
      // setWindowSize(width, height);
    }
  }

  return 0;
}

int HostWindow::onKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  switch (wParam)
  {
    case VK_F11:
    {
      if (pluginGui->can_resize(plugin))
      {
        static RECT pos{0, 0, 0, 0};
        auto style{::GetWindowLongPtrW(m_hwnd, GWL_STYLE)};

        if (style & WS_OVERLAPPEDWINDOW)
        {
          ::MONITORINFO mi{sizeof(mi)};
          ::GetWindowRect(m_hwnd, &pos);
          if (::GetMonitorInfoW(::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &mi))
          {
            ::SetWindowLongPtrW(m_hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            ::SetWindowPos(m_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                           mi.rcMonitor.right - mi.rcMonitor.left,
                           mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
          }
        }

        else
        {
          ::SetWindowLongPtrW(m_hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
          ::SetWindowPos(m_hwnd, nullptr, pos.left, pos.top, (pos.right - pos.left),
                         (pos.bottom - pos.top), SWP_FRAMECHANGED);
        }
      }

      break;
    }
    default:
      return 0;
  }

  return 0;
}

int HostWindow::onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto sah{freeaudio::clap_wrapper::standalone::getStandaloneHost()};

  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};
  auto pluginState{m_plugin->_ext._state};

  switch (wParam)
  {
    case IDM_SETTINGS:
    {
      m_settingsWindow.setWindowVisibility(!m_settingsWindow.getWindowVisibility());

      return 0;
    }

    case IDM_SAVE_STATE:
    {
      auto settingsPath{freeaudio::clap_wrapper::standalone::getStandaloneSettingsPath()};

      if (settingsPath.has_value())
      {
        try
        {
          auto savePath{settingsPath.value() / plugin->desc->id};

          fs::create_directories(savePath);
          sah->saveStandaloneAndPluginSettings(savePath, "settings.clapwrapper");
        }
        catch (const fs::filesystem_error& e)
        {
          ::MessageBoxW(nullptr, L"Unable to save file", nullptr, MB_OK | MB_ICONERROR);
        }
      }

      return 0;
    }

    case IDM_LOAD_STATE:
    {
      auto settingsPath{freeaudio::clap_wrapper::standalone::getStandaloneSettingsPath()};

      if (settingsPath.has_value())
      {
        try
        {
          auto loadPath{settingsPath.value() / plugin->desc->id};

          if (fs::exists(loadPath / "settings.clapwrapper"))
          {
            sah->tryLoadStandaloneAndPluginSettings(loadPath, "settings.clapwrapper");
          }
        }
        catch (const fs::filesystem_error& e)
        {
          ::MessageBoxW(nullptr, L"Unable to open file", nullptr, MB_OK | MB_ICONERROR);
        }
      }

      return 0;
    }

    case IDM_RESET_STATE:
    {
      pluginGui->destroy(plugin);
      m_standaloneHost->stopAudioThread();
      m_standaloneHost->stopMIDIThread();
      m_plugin->deactivate();
      m_plugin->terminate();

      auto plugin{freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, PLUGIN_ID, PLUGIN_INDEX,
                                                                        argc, argv)};

      setPlugin(m_plugin);

      return 0;
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int HostWindow::onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    pluginGui->hide(plugin);
    pluginGui->destroy(plugin);
    m_plugin.reset();
  }

  ::PostQuitMessage(0);

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
