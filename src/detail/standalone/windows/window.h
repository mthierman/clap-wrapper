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
    auto lpCreateStruct{reinterpret_cast<::CREATESTRUCTW*>(lParam)};
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

template <class T, HWND(T::*m_hwnd)>
T* InstanceFromWndProc(HWND hWnd, UINT uMsg, LPARAM lParam)
{
  T* self{nullptr};

  if (uMsg == WM_NCCREATE)
  {
    auto pCreateStruct{reinterpret_cast<LPCREATESTRUCTW>(lParam)};
    self = reinterpret_cast<T*>(pCreateStruct->lpCreateParams);
    ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<intptr_t>(self));
    self->*m_hwnd = hWnd;
  }

  else
    self = reinterpret_cast<T*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA));

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

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnDpiChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  int OnWindowPosChanged(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
