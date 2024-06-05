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
HostWindow::HostWindow(std::shared_ptr<Clap::Plugin> clapPlugin)
  : m_clapPlugin{clapPlugin}
  , m_plugin{m_clapPlugin->_plugin}
  , m_pluginGui{m_clapPlugin->_ext._gui}
  , m_pluginState{m_clapPlugin->_ext._state}
{
  if (!m_plugin)
  {
    helpers::errorBox({"Plugin is null"});
    helpers::abort();
  }

  if (!m_pluginGui)
  {
    helpers::errorBox({"Plugin GUI is null"});
    helpers::abort();
  }

  freeaudio::clap_wrapper::standalone::windows::helpers::createWindow(
      helpers::toUTF16(OUTPUT_NAME).c_str(), this);

  if (!m_hWnd)
  {
    helpers::errorBox({"Window creation failed"});
    helpers::abort();
  }

  setupMenu();

  setupStandaloneHost();

  if (!checkApi())
  {
    helpers::errorBox({"CLAP_WINDOW_API_WIN32 is not supported"});
    helpers::abort();
  }

  setupPlugin();

  m_pluginGui->show(m_plugin);

  helpers::activateWindow(m_hWnd.get());

  freeaudio::clap_wrapper::standalone::mainStartAudio();
}

void HostWindow::setupMenu()
{
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
}

void HostWindow::setupStandaloneHost()
{
  freeaudio::clap_wrapper::standalone::getStandaloneHost()->onRequestResize =
      [this](uint32_t width, uint32_t height) { return setWindowSize(width, height); };
}

bool HostWindow::checkApi()
{
  return m_pluginGui->is_api_supported(m_plugin, CLAP_WINDOW_API_WIN32, false);
}

void HostWindow::setupPlugin()
{
  m_pluginGui->create(m_plugin, CLAP_WINDOW_API_WIN32, false);

  m_pluginGui->set_scale(m_plugin, helpers::getCurrentScale(m_hWnd.get()));

  uint32_t width{0};
  uint32_t height{0};

  if (m_pluginGui->can_resize(m_plugin))
  {
    // We can restore size here
    // pluginGui->adjust_size(plugin, &previousWidth, &previousHeight);
    // pluginGui->set_size(plugin, previousWidth, previousHeight);
  }
  else
  {
    // We can't resize, so disable WS_THICKFRAME and WS_MAXIMIZEBOX
    ::SetWindowLongPtrW(m_hWnd.get(), GWL_STYLE,
                        ::GetWindowLongPtrW(m_hWnd.get(), GWL_STYLE) & ~WS_OVERLAPPEDWINDOW |
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

    m_pluginGui->get_size(m_plugin, &width, &height);
    setWindowSize(width, height);
  }

  clap_window clapWindow;
  clapWindow.api = CLAP_WINDOW_API_WIN32;
  clapWindow.win32 = static_cast<void*>(m_hWnd.get());

  m_pluginGui->set_parent(m_plugin, &clapWindow);
}

bool HostWindow::setWindowSize(uint32_t width, uint32_t height)
{
  ::RECT rect{0, 0, 0, 0};
  rect.right = width;
  rect.bottom = height;

  if (!m_hWnd)
  {
    return false;
  }

  ::AdjustWindowRectExForDpi(
      &rect, ::GetWindowLongPtrW(m_hWnd.get(), GWL_STYLE), ::GetMenu(m_hWnd.get()) != nullptr,
      ::GetWindowLongPtrW(m_hWnd.get(), GWL_EXSTYLE), helpers::getCurrentDpi(m_hWnd.get()));

  ::SetWindowPos(m_hWnd.get(), nullptr, 0, 0, (rect.right - rect.left), (rect.bottom - rect.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

  return true;
}

LRESULT CALLBACK HostWindow::wndProc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
  auto self{helpers::instance_from_wnd_proc<HostWindow>(hWnd, uMsg, lParam)};

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

int HostWindow::onDpiChanged(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
  m_pluginGui->set_scale(m_plugin, helpers::getCurrentScale(hWnd));

  return 0;
}

int HostWindow::onWindowPosChanged(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
  if (m_plugin && m_pluginGui)
  {
    if (m_pluginGui->can_resize(m_plugin))
    {
      ::RECT r{0, 0, 0, 0};
      ::GetClientRect(hWnd, &r);

      uint32_t width = (r.right - r.left);
      uint32_t height = (r.bottom - r.top);

      m_pluginGui->adjust_size(m_plugin, &width, &height);
      m_pluginGui->set_size(m_plugin, width, height);
    }
  }

  return 0;
}

int HostWindow::onSysCommand(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
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
          helpers::errorBox({"Unable to save state: ", e.what()});
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
          helpers::errorBox({"Unable to load state: ", e.what()});
        }
      }

      return 0;
    }

    case IDM_RESET_STATE:
    {
      freeaudio::clap_wrapper::standalone::getStandaloneHost()->loadDefaultPluginState(
          fs::temp_directory_path(), std::string(m_plugin->desc->id).append(".clapwrapper"));

      return 0;
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int HostWindow::onDestroy(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
  if (m_plugin && m_pluginGui)
  {
    m_pluginGui->hide(m_plugin);
    m_pluginGui->destroy(m_plugin);
    m_clapPlugin.reset();
  }

  freeaudio::clap_wrapper::standalone::mainFinish();

  helpers::quit();

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
