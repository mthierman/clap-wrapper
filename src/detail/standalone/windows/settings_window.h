#pragma once

#include <Windows.h>

namespace freeaudio::clap_wrapper::standalone::windows
{
struct SettingsWindow
{
  void createWindow();
  void showWindow();
  void hideWindow();
  bool checkVisibility();

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
