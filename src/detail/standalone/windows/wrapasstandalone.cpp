#include <Windows.h>

#include "wrapasstandalone.h"
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

  if (m_plugin->_ext._gui)
  {
    m_plugin->_ext._gui->create(m_plugin->_plugin, CLAP_WINDOW_API_WIN32, false);

    clap_window clapWindow;
    clapWindow.api = CLAP_WINDOW_API_WIN32;
    clapWindow.win32 = (void*)m_hwnd;
    m_plugin->_ext._gui->set_parent(m_plugin->_plugin, &clapWindow);
    m_plugin->_ext._gui->show(m_plugin->_plugin);
  }

  m_plugin->_ext._gui->set_scale(m_plugin->_plugin, static_cast<float>(::GetDpiForWindow(m_hwnd)) /
                                                        static_cast<float>(USER_DEFAULT_SCREEN_DPI));

  if (m_plugin->_ext._gui->can_resize(m_plugin->_plugin))
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
  m_plugin->_ext._gui->get_size(m_plugin->_plugin, &w, &h);

  RECT r{0, 0, 0, 0};
  r.right = w;
  r.bottom = h;

  ::AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, 0, 0, ::GetDpiForWindow(m_hwnd));

  ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

  ::ShowWindow(m_hwnd, SW_SHOWDEFAULT);
}

int Win32Gui::runLoop()
{
  MSG msg{};
  int r{0};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      return -1;
    }

    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }

  freeaudio::clap_wrapper::standalone::mainFinish();

  return 0;
}

LRESULT CALLBACK Win32Gui::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  Win32Gui* pWin32Gui = instance_from_wnd_proc<Win32Gui>(hWnd, uMsg, lParam);

  if (pWin32Gui)
  {
    switch (uMsg)
    {
      case WM_CREATE:
        return pWin32Gui->OnCreate(hWnd, uMsg, wParam, lParam);
      case WM_CLOSE:
        return pWin32Gui->OnClose(hWnd, uMsg, wParam, lParam);
      case WM_DESTROY:
        return pWin32Gui->OnDestroy(hWnd, uMsg, wParam, lParam);
      case WM_QUIT:
        return pWin32Gui->OnQuit(hWnd, uMsg, wParam, lParam);
      case WM_DPICHANGED:
        return pWin32Gui->OnDpiChanged(hWnd, uMsg, wParam, lParam);
      case WM_WINDOWPOSCHANGED:
        return pWin32Gui->OnWindowPosChanged(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int Win32Gui::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return 0;
}

int Win32Gui::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  ::DestroyWindow(hWnd);

  return 0;
}

int Win32Gui::OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (m_plugin && m_plugin->_ext._gui)
  {
    m_plugin->_ext._gui->hide(m_plugin->_plugin);
    m_plugin->_ext._gui->destroy(m_plugin->_plugin);
  }

  ::PostQuitMessage(0);

  return 0;
}

int Win32Gui::OnQuit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  return 0;
}

int Win32Gui::OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  m_plugin->_ext._gui->set_scale(m_plugin->_plugin, static_cast<float>(::GetDpiForWindow(hWnd)) /
                                                        static_cast<float>(USER_DEFAULT_SCREEN_DPI));

  uint32_t w{0};
  uint32_t h{0};
  m_plugin->_ext._gui->get_size(m_plugin->_plugin, &w, &h);

  RECT r{0, 0, 0, 0};
  r.right = w;
  r.bottom = h;

  ::AdjustWindowRectExForDpi(&r, WS_OVERLAPPEDWINDOW, 0, 0, ::GetDpiForWindow(hWnd));

  ::SetWindowPos(m_hwnd, nullptr, 0, 0, (r.right - r.left), (r.bottom - r.top),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

  return 0;
}

int Win32Gui::OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

std::string Win32Gui::narrow(std::wstring wstring)
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

std::wstring Win32Gui::widen(std::string string)
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

int main(int argc, char** argv)
{
  const clap_plugin_entry* entry{nullptr};

#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
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

  std::string pid{PLUGIN_ID};
  int pindex{PLUGIN_INDEX};

  auto plugin{
      freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, pid, pindex, 1, (char**)argv)};

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  win32Gui.setPlugin(plugin);

  win32Gui.activate();

  return win32Gui.runLoop();
}
