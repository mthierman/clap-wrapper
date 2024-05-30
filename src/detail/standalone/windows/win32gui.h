#pragma once

#include <Windows.h>

#include <clap_proxy.h>
#include "detail/standalone/standalone_host.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
struct Win32Gui
{
  void initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah);
  void setPlugin(std::shared_ptr<Clap::Plugin> p);

  void createWindow();
  void setupPlugin();
  bool setWindowSize(uint32_t width, uint32_t height);

  void runLoop();

  // void setScale();
  // void resizeWindow();

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  std::shared_ptr<Clap::Plugin> m_plugin;
  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
