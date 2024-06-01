#pragma once

#include <Windows.h>
#include <ShlObj.h>

#include "detail/standalone/standalone_host.h"
#include "settings_window.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
struct HostWindow
{
  HostWindow(int argc, char** argv);

  bool setWindowVisibility(bool visible);
  bool getWindowVisibility();
  bool setWindowSize(uint32_t width, uint32_t height);
  bool setPluginScale();

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  std::pair<int, char**> m_args;
  const clap_plugin_entry* m_entry;
  std::shared_ptr<Clap::Plugin> m_plugin;
  freeaudio::clap_wrapper::standalone::StandaloneHost* m_standaloneHost;
  HWND m_hwnd;
  SettingsWindow m_settingsWindow;
  std::vector<COMDLG_FILTERSPEC> m_fileTypes{{L"clapwrapper", L"*.clapwrapper"}};
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
