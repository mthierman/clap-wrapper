#pragma once

#include <Windows.h>

#include "detail/standalone/standalone_host.h"
#include "settings_window.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
struct HostWindow
{
  HostWindow();

  clap_window createClapWindow();
  void setPlugin(std::shared_ptr<Clap::Plugin> plugin);
  void setupPlugin();

  bool setWindowVisibility(bool visible);
  bool getWindowVisibility();
  bool setWindowSize(uint32_t width, uint32_t height);

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  freeaudio::clap_wrapper::standalone::StandaloneHost* m_standaloneHost;
  std::shared_ptr<Clap::Plugin> m_plugin;
  HWND m_hwnd;
  SettingsWindow m_settingsWindow;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
