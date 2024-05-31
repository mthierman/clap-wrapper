#pragma once

#include <Windows.h>

namespace freeaudio::clap_wrapper::standalone::windows
{
struct SettingsWindow
{
  SettingsWindow();

  bool setWindowVisibility(bool visible);
  bool getWindowVisibility();

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
