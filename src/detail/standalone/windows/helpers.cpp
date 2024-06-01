#include "helpers.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
const clap_plugin_entry* getClapPluginEntry()
{
  static const clap_plugin_entry* entry{nullptr};
  static Clap::Library library;

#ifdef STATICALLY_LINKED_CLAP_ENTRY
  static extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  auto clapFilename{std::string(HOSTED_CLAP_NAME).append(".clap")};

  for (const auto& searchPath : Clap::getValidCLAPSearchPaths())
  {
    auto clapPath{searchPath / clapFilename};

    if (fs::exists(clapPath) && !entry)
    {
      library.load(clapPath);
      entry = library._pluginEntry;
    }
  }
#endif

  return entry;
}

int messageLoop()
{
  MSG msg{};
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

std::string narrow(std::wstring wstring)
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

std::wstring widen(std::string string)
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

void messageBox(std::string message)
{
  ::MessageBoxW(nullptr, widen(message).c_str(), nullptr, MB_OK | MB_ICONASTERISK);
}

void errorBox(std::string message)
{
  ::MessageBoxW(nullptr, widen(message).c_str(), nullptr, MB_OK | MB_ICONHAND);
}

::HICON loadIconFromResource()
{
  return static_cast<::HICON>(
      ::LoadImageA(::GetModuleHandleA(nullptr), MAKEINTRESOURCEA(1), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
