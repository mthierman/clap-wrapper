#pragma once

#include <Windows.h>
#include <string>
#include <stdexcept>

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

std::string narrow(std::wstring wstring);
std::wstring widen(std::string string);
}  // namespace freeaudio::clap_wrapper::standalone::windows