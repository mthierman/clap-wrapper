#pragma once

#include <Windows.h>

#include <clap_proxy.h>
#include "detail/standalone/standalone_host.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
template <typename T>
T* instance_from_wnd_proc(HWND hWnd, UINT uMsg, LPARAM lParam)
{
  T* self{nullptr};

  if (uMsg == WM_NCCREATE)
  {
    auto lpCreateStruct{reinterpret_cast<::LPCREATESTRUCTW>(lParam)};
    self = static_cast<T*>(lpCreateStruct->lpCreateParams);
    ::SetWindowLongPtrW(hWnd, 0, reinterpret_cast<intptr_t>(self));
    self->m_hwnd = hWnd;
  }

  else
  {
    self = reinterpret_cast<T*>(::GetWindowLongPtrW(hWnd, 0));
  }

  return self;
}

template <typename T, typename U>
int safe_size(T value)
{
  constexpr int max{std::numeric_limits<U>::max()};
  if (value > static_cast<T>(max)) throw std::overflow_error("Unsafe size");

  return static_cast<U>(value);
}

struct Win32Gui
{
  void initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah);
  void setPlugin(std::shared_ptr<Clap::Plugin> p);
  void createWindow();
  void setupPlugin();
  int runLoop(int argc, char** argv);

  std::shared_ptr<Clap::Plugin> plugin;

  static std::string narrow(std::wstring utf16);
  static std::wstring widen(std::string utf8);

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
