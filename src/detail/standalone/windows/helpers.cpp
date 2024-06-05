#include "helpers.h"

namespace freeaudio::clap_wrapper::standalone::windows::helpers::details
{
auto CALLBACK DefaultWindowProcedure::wndProc(::HWND hWnd, ::UINT uMsg, ::WPARAM wParam,
                                              ::LPARAM lParam) -> ::LRESULT
{
  return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
}  // namespace freeaudio::clap_wrapper::standalone::windows::helpers::details

namespace freeaudio::clap_wrapper::standalone::windows::helpers
{
::HMODULE getInstance()
{
  ::HMODULE hInstance;
  ::GetModuleHandleExW(0, nullptr, &hInstance);

  return hInstance;
}

bool activateWindow(::HWND window)
{
  return ::ShowWindow(window, SW_NORMAL);
}

bool showWindow(::HWND window)
{
  return ::ShowWindow(window, SW_SHOW);
}

bool hideWindow(::HWND window)
{
  return ::ShowWindow(window, SW_HIDE);
}

bool closeWindow(::HWND window)
{
  return ::CloseWindow(window);
}

void abort(unsigned int exitCode)
{
  ::ExitProcess(exitCode);
}

void quit(unsigned int exitCode)
{
  ::PostQuitMessage(exitCode);
}

int messageLoop()
{
  ::MSG msg{};
  int r{};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      return EXIT_FAILURE;
    }

    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }

  return EXIT_SUCCESS;
}

std::string toUTF8(std::wstring wstring)
{
  if (wstring.empty()) return {};

  auto safeSize{safe_size<size_t, int>(wstring.length())};

  auto length{::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, wstring.data(),
                                    safeSize, nullptr, 0, nullptr, nullptr)};

  std::string utf8(length, 0);

  if (::WideCharToMultiByte(CP_UTF8, WC_NO_BEST_FIT_CHARS | WC_ERR_INVALID_CHARS, wstring.data(),
                            safeSize, utf8.data(), length, nullptr, nullptr) > 0)
    return utf8;

  else
    return {};
}

std::wstring toUTF16(std::string string)
{
  if (string.empty()) return {};

  auto safeSize{safe_size<size_t, int>(string.length())};

  auto length{::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.data(), safeSize, nullptr, 0)};

  std::wstring utf16(length, 0);

  if (::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, string.data(), safeSize, utf16.data(),
                            length) > 0)
    return utf16;

  else
    return {};
}

void log(std::initializer_list<std::string> args)
{
  std::string message;

  for (const auto& arg : args)
  {
    message.append(arg);
  }

  ::OutputDebugStringW(toUTF16(message).c_str());
  ::OutputDebugStringW(L"\n");
}

void messageBox(std::initializer_list<std::string> args)
{
  std::string message;

  for (auto arg : args)
  {
    message.append(arg);
  }

  ::MessageBoxW(nullptr, toUTF16(message).c_str(), nullptr, MB_OK | MB_ICONASTERISK);
}

void errorBox(std::initializer_list<std::string> args)
{
  std::string message;

  for (auto arg : args)
  {
    message.append(arg);
  }

  ::MessageBoxW(nullptr, toUTF16(message).c_str(), nullptr, MB_OK | MB_ICONHAND);
}

::HBRUSH loadBrushFromSystem(int name)
{
  return static_cast<::HBRUSH>(::GetStockObject(name));
}

::HCURSOR loadCursorFromSystem(LPSTR name)
{
  return static_cast<::HCURSOR>(
      ::LoadImageA(nullptr, name, IMAGE_CURSOR, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
}

::HICON loadIconFromSystem(LPSTR name)
{
  return static_cast<::HICON>(::LoadImageA(nullptr, name, IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
}

::HICON loadIconFromResource()
{
  ::HMODULE hInstance;
  ::GetModuleHandleExW(0, nullptr, &hInstance);

  return static_cast<::HICON>(
      ::LoadImageW(hInstance, MAKEINTRESOURCEW(1), IMAGE_ICON, 0, 0, LR_SHARED | LR_DEFAULTSIZE));
}
}  // namespace freeaudio::clap_wrapper::standalone::windows::helpers
