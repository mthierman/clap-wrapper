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

std::string narrow(std::wstring utf16);
std::wstring widen(std::string utf8);

struct Win32Gui
{
  void initialize(freeaudio::clap_wrapper::standalone::StandaloneHost* sah);
  void setPlugin(std::shared_ptr<Clap::Plugin> p);
  void activate();
  void runLoop();

  static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int onWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  std::wstring m_clapName{widen(HOSTED_CLAP_NAME)};
  std::shared_ptr<Clap::Plugin> m_plugin;
  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
