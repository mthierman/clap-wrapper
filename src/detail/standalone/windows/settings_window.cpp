#include <string>

#include "detail/standalone/standalone_details.h"
#include "settings_window.h"
#include "helpers.h"

#define ID_COMBOBOX1 101
#define ID_COMBOBOX2 102
#define ID_COMBOBOX3 103
#define ID_BUTTON1 104
#define ID_BUTTON2 105

namespace freeaudio::clap_wrapper::standalone::windows
{
SettingsWindow::SettingsWindow()
{
  auto windowName{std::wstring(L"Audio/MIDI Settings")};
  auto iconFromResource{loadIconFromResource()};

  ::WNDCLASSEXW wcex{sizeof(::WNDCLASSEXW)};
  wcex.lpszClassName = windowName.c_str();
  wcex.lpszMenuName = windowName.c_str();
  wcex.lpfnWndProc = SettingsWindow::wndProc;
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
    errorBox({"Registering settings window failed"});
    ::ExitProcess(EXIT_FAILURE);
  }

  ::CreateWindowExW(0, windowName.c_str(), windowName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, this);

  // ::MONITORINFO mi{sizeof(::MONITORINFO)};
  // ::GetMonitorInfoA(::MonitorFromWindow(m_hWnd.get(), MONITOR_DEFAULTTONEAREST), &mi);
  // int width{600};
  // int height{400};
  // auto x = (static_cast<int>(mi.rcWork.right - mi.rcWork.left) - width) / 2;
  // auto y = (static_cast<int>(mi.rcWork.bottom - mi.rcWork.top) - height) / 2;
  // ::SetWindowPos(m_hWnd.get(), nullptr, x, y, width, height, 0);

  setWindowVisibility(false);
}

bool SettingsWindow::setWindowVisibility(bool visible)
{
  ::ShowWindow(m_hWnd.get(), visible ? SW_SHOW : SW_HIDE);

  return visible ? true : false;
}

bool SettingsWindow::getWindowVisibility()
{
  return ::IsWindowVisible(m_hWnd.get());
}

LRESULT CALLBACK SettingsWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{instance_from_wnd_proc<SettingsWindow>(hWnd, uMsg, lParam)};

  if (self)
  {
    switch (uMsg)
    {
      case WM_CREATE:
        return self->onCreate(hWnd, uMsg, wParam, lParam);
      case WM_CLOSE:
        return self->onClose(hWnd, uMsg, wParam, lParam);
    }
  }

  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

int SettingsWindow::onCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  CreateWindow("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 10, 10, 150, 100,
               m_hWnd.get(), (HMENU)ID_COMBOBOX1, NULL, NULL);
  CreateWindow("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 10, 50, 150, 100,
               m_hWnd.get(), (HMENU)ID_COMBOBOX2, NULL, NULL);
  CreateWindow("COMBOBOX", NULL, WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 10, 90, 150, 100,
               m_hWnd.get(), (HMENU)ID_COMBOBOX3, NULL, NULL);

  CreateWindow("BUTTON", "Button 1", WS_CHILD | WS_VISIBLE, 10, 130, 80, 30, m_hWnd.get(),
               (HMENU)ID_BUTTON1, NULL, NULL);
  CreateWindow("BUTTON", "Button 2", WS_CHILD | WS_VISIBLE, 100, 130, 80, 30, m_hWnd.get(),
               (HMENU)ID_BUTTON2, NULL, NULL);

  return 0;
}

int SettingsWindow::onClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  setWindowVisibility(false);

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
