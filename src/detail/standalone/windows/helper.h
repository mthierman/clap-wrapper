#include <Windows.h>

#include <string>
#include <random>

#define IDM_SETTINGS 1001
#define IDM_SAVE_STATE 1002
#define IDM_LOAD_STATE 1003
#define IDM_RESET_STATE 1004

namespace freeaudio::clap_wrapper::standalone::windows
{
template <class T, HWND(T::*m_hwnd)>
T* InstanceFromWndProc(HWND hwnd, UINT umsg, LPARAM lparam)
{
  T* pInstance;

  if (umsg == WM_NCCREATE)
  {
    LPCREATESTRUCT pCreateStruct{reinterpret_cast<LPCREATESTRUCT>(lparam)};
    pInstance = reinterpret_cast<T*>(pCreateStruct->lpCreateParams);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pInstance));
    pInstance->*m_hwnd = hwnd;
  }

  else
    pInstance = reinterpret_cast<T*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

  return pInstance;
}

std::string narrow(std::wstring in)
{
  if (!in.empty())
  {
    auto inSize{static_cast<int>(in.size())};

    auto outSize{::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, in.data(),
                                       inSize, nullptr, 0, nullptr, nullptr)};

    if (outSize > 0)
    {
      std::string out;
      out.resize(static_cast<size_t>(outSize));

      if (::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, in.data(), inSize,
                                out.data(), outSize, nullptr, nullptr) > 0)
        return out;
    }
  }

  return {};
}

std::wstring widen(std::string in)
{
  if (!in.empty())
  {
    auto inSize{static_cast<int>(in.size())};

    auto outSize{::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.data(), inSize, nullptr, 0)};

    if (outSize > 0)
    {
      std::wstring out;
      out.resize(static_cast<size_t>(outSize));

      if (::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.data(), inSize, out.data(), outSize) >
          0)
        return out;
    }
  }

  return {};
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
