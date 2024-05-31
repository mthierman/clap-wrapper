#include <Windows.h>
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
  std::string clapName{HOSTED_CLAP_NAME};
  auto searchPaths{Clap::getValidCLAPSearchPaths()};
  auto lib{Clap::Library()};

  for (const auto& searchPath : searchPaths)
  {
    auto clapPath{searchPath / (clapName.append(".clap"))};
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
}  // namespace freeaudio::clap_wrapper::standalone::windows
