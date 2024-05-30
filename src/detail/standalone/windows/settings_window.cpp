#include "settings_window.h"
#include "helpers.h"
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
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
