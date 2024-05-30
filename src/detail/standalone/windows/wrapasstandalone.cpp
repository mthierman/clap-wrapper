#include "wrapasstandalone.h"
#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
std::string narrow(std::wstring wstring)
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

std::wstring widen(std::string string)
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

void Win32Gui::initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah)
{
  sah->win32Gui = this;
}

void Win32Gui::setPlugin(std::shared_ptr<Clap::Plugin> p)
{
  m_plugin = p;
}

void Win32Gui::activate()
{
  std::wstring clapName{widen(HOSTED_CLAP_NAME)};

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

  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  if (pluginGui)
  {
    pluginGui->create(plugin, CLAP_WINDOW_API_WIN32, false);

    clap_window clapWindow;
    clapWindow.api = CLAP_WINDOW_API_WIN32;
    clapWindow.win32 = static_cast<void*>(m_hwnd);

    pluginGui->set_parent(plugin, &clapWindow);
    pluginGui->show(plugin);
  }

  setScale();

  if (pluginGui->can_resize(plugin))
  {
    // We can check here if we had a previous size but we aren't saving state yet
  }
  // else
  // {
  //   ::SetWindowLongPtrW(m_hwnd, GWL_STYLE,
  //                       ::GetWindowLongPtrW(m_hwnd, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW | WS_OVERLAPPED |
  //                           WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
  // }

  setWindowSize();

  ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
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

void Win32Gui::setScale()
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  pluginGui->set_scale(plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                   static_cast<float>(USER_DEFAULT_SCREEN_DPI));
}

void Win32Gui::setWindowSize()
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  uint32_t w{0};
  uint32_t h{0};
  pluginGui->get_size(plugin, &w, &h);

  RECT r{0, 0, 0, 0};
  r.right = w;
  r.bottom = h;

  ::AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, 0, 0, ::GetDpiForWindow(m_hwnd));

  ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}

void Win32Gui::resizeWindow()
{
  setScale();
  setWindowSize();
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
  resizeWindow();

  return 0;
}

int Win32Gui::onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto pluginGui{m_plugin->_ext._gui};
  auto plugin{m_plugin->_plugin};

  auto dpi{::GetDpiForWindow(hWnd)};
  auto scaleFactor{static_cast<float>(dpi) / static_cast<float>(USER_DEFAULT_SCREEN_DPI)};

  if (pluginGui->can_resize(plugin))
  {
    RECT r{0, 0, 0, 0};
    ::GetClientRect(hWnd, &r);

    uint32_t w = (r.right - r.left);
    uint32_t h = (r.bottom - r.top);

    pluginGui->adjust_size(plugin, &w, &h);
    pluginGui->set_size(plugin, w, h);
  }

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows

int main(int argc, char** argv)
{
  const clap_plugin_entry* entry{nullptr};

#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  // Library shenanigans t/k
  auto clapName{std::string{HOSTED_CLAP_NAME}};
  LOG << "Loading " << clapName << std::endl;

  auto searchPaths{Clap::getValidCLAPSearchPaths()};

  auto lib{Clap::Library()};

  for (const auto& searchPath : searchPaths)
  {
    auto clapPath{searchPath / (clapName + ".clap")};

    if (fs::exists(clapPath) && !entry)
    {
      lib.load(clapPath);
      entry = lib._pluginEntry;
    }
  }
#endif

  if (!entry)
  {
    std::cerr << "Clap Standalone: No Entry as configured" << std::endl;
    return 3;
  }

  freeaudio::clap_wrapper::standalone::windows::Win32Gui win32Gui{};

  win32Gui.initialize(freeaudio::clap_wrapper::standalone::getStandaloneHost());

  win32Gui.setPlugin(freeaudio::clap_wrapper::standalone::mainCreatePlugin(
      entry, std::string{PLUGIN_ID}, PLUGIN_INDEX, 1, (char**)argv));

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  win32Gui.activate();

  win32Gui.runLoop();

  freeaudio::clap_wrapper::standalone::mainFinish();

  return 0;
}
