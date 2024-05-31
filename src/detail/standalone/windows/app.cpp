#include <Windows.h>
#include <iostream>
#include "detail/standalone/entry.h"
#include "app.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
int run(int argc, char* argv[])
{
  const clap_plugin_entry* entry{nullptr};
#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  auto lib{Clap::Library()};

  auto searchPaths{Clap::getValidCLAPSearchPaths()};
  auto clapFilename{std::string(HOSTED_CLAP_NAME).append(".clap")};

  for (const auto& searchPath : searchPaths)
  {
    auto clapPath{searchPath / clapFilename};

    if (fs::exists(clapPath) && !entry)
    {
      lib.load(clapPath);
      entry = lib._pluginEntry;
    }
  }
#endif

  if (!entry)
  {
    ::MessageBoxW(nullptr, L"Clap Standalone: No entry as configured", nullptr, MB_OK | MB_ICONERROR);
    return 3;
  }

  auto plugin{
      freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, PLUGIN_ID, PLUGIN_INDEX, argc, argv)};

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  MSG msg{};
  int r{};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      break;
    }

    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }

  return freeaudio::clap_wrapper::standalone::mainFinish();
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
