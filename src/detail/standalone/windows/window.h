#include <Windows.h>

#include <string>
#include <random>

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
template <typename T>
T* instance_from_wnd_proc(HWND hWnd, UINT uMsg, LPARAM lParam)
{
  T* self{nullptr};

  if (uMsg == WM_NCCREATE)
  {
    auto lpCreateStruct{reinterpret_cast<::CREATESTRUCTA*>(lParam)};
    self = static_cast<T*>(lpCreateStruct->lpCreateParams);
    ::SetWindowLongPtrA(hWnd, 0, reinterpret_cast<intptr_t>(self));
    self->m_hwnd = hWnd;
  }

  else
  {
    self = reinterpret_cast<T*>(::GetWindowLongPtrA(hWnd, 0));
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

struct Window
{
  Window();
  ~Window();

  static std::string narrow(std::wstring utf16);
  static std::wstring widen(std::string utf8);

  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  int OnClose(HWND, UINT, WPARAM, LPARAM);
  int OnDestroy(HWND, UINT, WPARAM, LPARAM);
  int OnDpiChanged(HWND, UINT, WPARAM, LPARAM);
  int OnWindowPosChanged(HWND, UINT, WPARAM, LPARAM);

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
