#include <wil/com.h>
#include "detail/standalone/entry.h"
#include "host_window.h"
#include "helpers.h"

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
HostWindow::HostWindow(std::shared_ptr<Clap::Plugin> clapPlugin) : m_plugin{clapPlugin}
{
  auto windowName{widen(OUTPUT_NAME)};
  auto iconFromResource{loadIconFromResource()};

  ::WNDCLASSEXW wcex{sizeof(::WNDCLASSEXW)};
  wcex.lpszClassName = windowName.c_str();
  wcex.lpszMenuName = windowName.c_str();
  wcex.lpfnWndProc = HostWindow::wndProc;
  wcex.style = 0;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = sizeof(intptr_t);
  wcex.hInstance = nullptr;
  wcex.hbrBackground = loadBrushFromSystem();
  wcex.hCursor = loadCursorFromSystem();
  wcex.hIcon = iconFromResource ? iconFromResource : loadIconFromSystem();
  wcex.hIconSm = iconFromResource ? iconFromResource : loadIconFromSystem();

  auto atom{::RegisterClassExW(&wcex)};

  if (!atom)
  {
    errorBox({"Registering host window failed"});
    ::ExitProcess(EXIT_FAILURE);
  }

  ::CreateWindowExW(0, windowName.c_str(), windowName.c_str(), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                    nullptr, this);

  auto hMenu{::GetSystemMenu(m_hWnd.get(), FALSE)};

  ::MENUITEMINFOW seperator{sizeof(::MENUITEMINFOW)};
  seperator.fMask = MIIM_FTYPE;
  seperator.fType = MFT_SEPARATOR;

  ::MENUITEMINFOW audioIn{sizeof(::MENUITEMINFOW)};
  audioIn.fMask = MIIM_STRING | MIIM_ID;
  audioIn.wID = IDM_SETTINGS;
  audioIn.dwTypeData = const_cast<LPWSTR>(L"Audio/MIDI Settings");

  ::MENUITEMINFOW saveState{sizeof(::MENUITEMINFOW)};
  saveState.fMask = MIIM_STRING | MIIM_ID;
  saveState.wID = IDM_SAVE_STATE;
  saveState.dwTypeData = const_cast<LPWSTR>(L"Save plugin state...");

  ::MENUITEMINFOW loadState{sizeof(::MENUITEMINFOW)};
  loadState.fMask = MIIM_STRING | MIIM_ID;
  loadState.wID = IDM_LOAD_STATE;
  loadState.dwTypeData = const_cast<LPWSTR>(L"Load plugin state...");

  ::MENUITEMINFOW resetState{sizeof(::MENUITEMINFOW)};
  resetState.fMask = MIIM_STRING | MIIM_ID;
  resetState.wID = IDM_RESET_STATE;
  resetState.dwTypeData = const_cast<LPWSTR>(L"Reset to default plugin state");

  if (hMenu != INVALID_HANDLE_VALUE)
  {
    ::InsertMenuItemW(hMenu, 1, TRUE, &seperator);
    ::InsertMenuItemW(hMenu, 2, TRUE, &audioIn);
    ::InsertMenuItemW(hMenu, 3, TRUE, &seperator);
    ::InsertMenuItemW(hMenu, 4, TRUE, &saveState);
    ::InsertMenuItemW(hMenu, 5, TRUE, &loadState);
    ::InsertMenuItemW(hMenu, 6, TRUE, &seperator);
    ::InsertMenuItemW(hMenu, 7, TRUE, &resetState);
    ::InsertMenuItemW(hMenu, 8, TRUE, &seperator);
  }

  freeaudio::clap_wrapper::standalone::getStandaloneHost()->onRequestResize =
      [this](uint32_t width, uint32_t height) { return setWindowSize(width, height); };

  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (!pluginGui->is_api_supported(plugin, CLAP_WINDOW_API_WIN32, false))
  {
    errorBox({"CLAP_WINDOW_API_WIN32 is not supported"});
    ::ExitProcess(EXIT_FAILURE);
  }

  pluginGui->create(plugin, CLAP_WINDOW_API_WIN32, false);

  setPluginScale();

  uint32_t width{0};
  uint32_t height{0};

  if (pluginGui->can_resize(plugin))
  {
    // We can restore size here

    // width = previousWidth();
    // height = previousHeight();
    // pluginGui->adjust_size(plugin, &width, &height);
    // pluginGui->set_size(plugin, width, height);
  }
  else
  {
    // We can't resize, so disable WS_THICKFRAME and WS_MAXIMIZEBOX
    ::SetWindowLongPtrW(m_hWnd.get(), GWL_STYLE,
                        ::GetWindowLongPtrW(m_hWnd.get(), GWL_STYLE) & ~WS_OVERLAPPEDWINDOW |
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

    pluginGui->get_size(plugin, &width, &height);
    setWindowSize(width, height);
  }

  clap_window clapWindow;
  clapWindow.api = CLAP_WINDOW_API_WIN32;
  clapWindow.win32 = static_cast<void*>(m_hWnd.get());

  pluginGui->set_parent(plugin, &clapWindow);

  pluginGui->show(plugin);

  setWindowVisibility(true);

  freeaudio::clap_wrapper::standalone::mainStartAudio();
}

bool HostWindow::setWindowVisibility(bool visible)
{
  ::ShowWindow(m_hWnd.get(), visible ? SW_SHOW : SW_HIDE);

  return visible ? true : false;
}

bool HostWindow::getWindowVisibility()
{
  return ::IsWindowVisible(m_hWnd.get());
}

bool HostWindow::setWindowSize(uint32_t width, uint32_t height)
{
  RECT r{0, 0, 0, 0};
  r.right = width;
  r.bottom = height;

  if (m_hWnd)
  {
    ::AdjustWindowRectExForDpi(
        &r, ::GetWindowLongPtrW(m_hWnd.get(), GWL_STYLE), ::GetMenu(m_hWnd.get()) != nullptr,
        ::GetWindowLongPtrW(m_hWnd.get(), GWL_EXSTYLE), ::GetDpiForWindow(m_hWnd.get()));

    ::SetWindowPos(m_hWnd.get(), nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                   SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

    return true;
  }

  else
  {
    return false;
  }
}

bool HostWindow::setPluginScale()
{
  auto plugin{m_plugin->_plugin};
  auto pluginGui{m_plugin->_ext._gui};

  if (plugin && pluginGui)
  {
    return pluginGui->set_scale(plugin, static_cast<double>(::GetDpiForWindow(m_hWnd.get())) /
                                            static_cast<double>(USER_DEFAULT_SCREEN_DPI));
  }

  return false;
}

LRESULT CALLBACK HostWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{instance_from_wnd_proc<HostWindow>(hWnd, uMsg, lParam)};

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

int HostWindow::onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  setPluginScale();

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
    }
  }

  return 0;
}

int HostWindow::onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
      auto coUninitialize{wil::CoInitializeEx(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)};

      auto fileSaveDialog{wil::CoCreateInstance<IFileSaveDialog>(CLSID_FileSaveDialog)};

      fileSaveDialog->SetDefaultExtension(m_fileTypes.at(0).pszName);
      fileSaveDialog->SetFileTypes(m_fileTypes.size(), m_fileTypes.data());

      fileSaveDialog->Show(m_hWnd.get());

      wil::com_ptr<IShellItem> shellItem;
      auto hr{fileSaveDialog->GetResult(&shellItem)};

      if (SUCCEEDED(hr))
      {
        wil::unique_cotaskmem_string result;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &result);

        auto saveFile{std::filesystem::path(result.get())};

        LOG << saveFile << std::endl;

        try
        {
          freeaudio::clap_wrapper::standalone::getStandaloneHost()->saveStandaloneAndPluginSettings(
              saveFile.parent_path(), saveFile.filename());
        }
        catch (const fs::filesystem_error& e)
        {
          errorBox({"Unable to save state: ", e.what()});
        }
      }

      return 0;
    }

    case IDM_LOAD_STATE:
    {
      auto coUninitialize{wil::CoInitializeEx(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)};

      auto fileOpenDialog{wil::CoCreateInstance<IFileOpenDialog>(CLSID_FileOpenDialog)};

      fileOpenDialog->SetDefaultExtension(m_fileTypes.at(0).pszName);
      fileOpenDialog->SetFileTypes(m_fileTypes.size(), m_fileTypes.data());

      fileOpenDialog->Show(m_hWnd.get());

      wil::com_ptr<IShellItem> shellItem;
      auto hr{fileOpenDialog->GetResult(&shellItem)};

      if (SUCCEEDED(hr))
      {
        wil::unique_cotaskmem_string result;
        shellItem->GetDisplayName(SIGDN_FILESYSPATH, &result);

        auto saveFile{std::filesystem::path(result.get())};

        LOG << saveFile << std::endl;

        try
        {
          if (fs::exists(saveFile))
          {
            freeaudio::clap_wrapper::standalone::getStandaloneHost()->tryLoadStandaloneAndPluginSettings(
                saveFile.parent_path(), saveFile.filename());
          }
        }
        catch (const fs::filesystem_error& e)
        {
          errorBox({"Unable to load state: ", e.what()});
        }
      }

      return 0;
    }

    case IDM_RESET_STATE:
    {
      freeaudio::clap_wrapper::standalone::getStandaloneHost()->loadDefaultPluginState(
          fs::temp_directory_path(), std::string(plugin->desc->id).append(".clapwrapper"));

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

  freeaudio::clap_wrapper::standalone::mainFinish();

  ::PostQuitMessage(0);

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
