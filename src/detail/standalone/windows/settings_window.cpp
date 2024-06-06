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
  freeaudio::clap_wrapper::standalone::windows::helpers::createWindow(L"Audio/MIDI Settings", this);

  if (!m_hWnd)
  {
    helpers::errorBox({"Settings Window creation failed"});
    helpers::abort();
  }

  int width{400};
  int height{360};

  helpers::centerWindow(m_hWnd.get(), 400, 360);
}

LRESULT CALLBACK SettingsWindow::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  auto self{helpers::instanceFromWndProc<SettingsWindow>(hWnd, uMsg, lParam)};

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
  helpers::hideWindow(hWnd);

  return 0;
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
