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

  sah->onRequestResize = [this](uint32_t width, uint32_t height)
  {
    LOG << "onRequestResize" << std::endl;
    return setWindowSize(width, height);
  };
}

void Win32Gui::setPlugin(std::shared_ptr<Clap::Plugin> p)
{
  m_plugin = p;
}

void Win32Gui::createHostWindow()
{
  std::wstring windowName{widen(OUTPUT_NAME)};

  WNDCLASSEXW wcex{sizeof(WNDCLASSEXW)};
  wcex.lpszClassName = windowName.c_str();
  wcex.lpszMenuName = windowName.c_str();
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

  ::CreateWindowExW(0, windowName.c_str(), windowName.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                    ::GetModuleHandleW(nullptr), this);

  // https://www.codeproject.com/Articles/7503/An-examination-of-menus-from-a-beginner-s-point-of#systemmenu

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

void Win32Gui::createSettingsWindow()
{
  m_settingsWindow.createWindow();
}

clap_window Win32Gui::createClapWindow()
{
  clap_window clapWindow;
  clapWindow.api = CLAP_WINDOW_API_WIN32;
  clapWindow.win32 = static_cast<void*>(m_hwnd);

  return clapWindow;
}

void Win32Gui::setupPlugin()
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    if (!pluginGui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false))
    {
      LOG << "NO WIN32 " << std::endl;
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

void Win32Gui::showHostWindow()
{
  ::ShowWindow(m_hwnd, SW_SHOW);
}

void Win32Gui::hideHostWindow()
{
  ::ShowWindow(m_hwnd, SW_HIDE);
}

void Win32Gui::showSettingsWindow()
{
  m_settingsWindow.showWindow();
}

void Win32Gui::hideSettingsWindow()
{
  m_settingsWindow.hideWindow();
}

bool Win32Gui::checkSettingsVisibility()
{
  return m_settingsWindow.checkVisibility();
}

bool Win32Gui::setWindowSize(uint32_t width, uint32_t height)
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
  }

  return true;
}

LRESULT CALLBACK Win32Gui::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{instance_from_wnd_proc<Win32Gui>(hWnd, uMsg, lParam)};

  if (self)
  {
    switch (uMsg)
    {
      case WM_DPICHANGED:
        return self->onDpiChanged(hWnd, uMsg, wParam, lParam);
      case WM_WINDOWPOSCHANGED:
        return self->onWindowPosChanged(hWnd, uMsg, wParam, lParam);
      case WM_SYSCOMMAND:
        return self->onSysCommand(hWnd, uMsg, wParam, lParam);
      case WM_DESTROY:
        return self->onDestroy(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int Win32Gui::onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

int Win32Gui::onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

int Win32Gui::onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto sah{freeaudio::clap_wrapper::standalone::getStandaloneHost()};

  auto plugin{m_plugin->_plugin};
  auto pluginState{m_plugin->_ext._state};

  switch (wParam)
  {
    case IDM_SETTINGS:
    {
      checkSettingsVisibility() ? hideSettingsWindow() : showSettingsWindow();

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
      //

      return 0;
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int Win32Gui::onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
