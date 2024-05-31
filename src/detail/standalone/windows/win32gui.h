#pragma once

#include <Windows.h>

#include <clap_proxy.h>
#include "detail/standalone/standalone_host.h"
#include "settings_window.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
struct Win32Gui
{
  void initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah);
  void setPlugin(std::shared_ptr<Clap::Plugin> p);

  void createHostWindow();
  void createSettingsWindow();

  clap_window createClapWindow();
  void setupPlugin();

  void runLoop();

  void showHostWindow();
  void hideHostWindow();

  void showSettingsWindow();
  void hideSettingsWindow();
  bool checkSettingsVisibility();

  bool setWindowSize(uint32_t width, uint32_t height);

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  SettingsWindow m_settingsWindow;
  std::shared_ptr<Clap::Plugin> m_plugin;
  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
