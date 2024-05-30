#include "settings_window.h"
#include "helpers.h"
#include "detail/standalone/standalone_details.h"
#include <string>

namespace freeaudio::clap_wrapper::standalone::windows
{
void SettingsWindow::createWindow()
{
  std::wstring windowName{L"Audio/MIDI Settings"};

  WNDCLASSEXW wcex{sizeof(WNDCLASSEXW)};
  wcex.lpszClassName = windowName.c_str();
  wcex.lpszMenuName = windowName.c_str();
  wcex.lpfnWndProc = SettingsWindow::wndProc;
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

  ::CreateWindowExW(0, windowName.c_str(), windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                    ::GetModuleHandleW(nullptr), this);

  ::MONITORINFO mi{sizeof(::MONITORINFO)};
  ::GetMonitorInfoA(::MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST), &mi);

  LOG << "rcMonitor WIDTH: " << mi.rcMonitor.right - mi.rcMonitor.left << std::endl;
  LOG << "rcMonitor HEIGHT: " << mi.rcMonitor.bottom - mi.rcMonitor.top << std::endl;

  LOG << "rcWork WIDTH: " << mi.rcWork.right - mi.rcWork.left << std::endl;
  LOG << "rcWork HEIGHT: " << mi.rcWork.bottom - mi.rcWork.top << std::endl;

  auto screenWidth = static_cast<int>(mi.rcWork.right - mi.rcWork.left);
  auto screenHeight = static_cast<int>(mi.rcWork.bottom - mi.rcWork.top);

  auto x = (screenWidth - 600) / 2;
  auto y = (screenHeight - 400) / 2;

  LOG << "SETTINGS X:" << x << std::endl;
  LOG << "SETTINGS Y:" << y << std::endl;

  ::SetWindowPos(m_hwnd, nullptr, x, y, 600, 400, 0);
}

void SettingsWindow::showWindow()
{
  ::ShowWindow(m_hwnd, SW_SHOW);
}

void SettingsWindow::hideWindow()
{
  ::ShowWindow(m_hwnd, SW_HIDE);
}

bool SettingsWindow::checkVisibility()
{
  return ::IsWindowVisible(m_hwnd);
}

LRESULT CALLBACK SettingsWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{instance_from_wnd_proc<SettingsWindow>(hWnd, uMsg, lParam)};

  if (self)
  {
    switch (uMsg)
    {
      // case WM_CREATE:
      // return self->onCreate(hWnd, uMsg, wParam, lParam);
      case WM_CLOSE:
        return self->onClose(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int SettingsWindow::onClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  hideWindow();

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
