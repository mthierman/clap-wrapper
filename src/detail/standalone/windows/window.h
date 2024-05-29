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
    self->m_hwnd.reset(hWnd);
  }

  else
  {
    self = reinterpret_cast<T*>(::GetWindowLongPtrA(hWnd, 0));
  }

  return self;
}

struct Window
{
  Window();
  ~Window();

  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  virtual int OnClose(HWND, UINT, WPARAM, LPARAM);
  virtual int OnDestroy(HWND, UINT, WPARAM, LPARAM);
  virtual int OnDpiChanged(HWND, UINT, WPARAM, LPARAM);
  virtual int OnKeyDown(HWND, UINT, WPARAM, LPARAM);
  virtual int OnWindowPosChanged(HWND, UINT, WPARAM, LPARAM);

  bool fullscreen();

  HWND m_hwnd;
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
