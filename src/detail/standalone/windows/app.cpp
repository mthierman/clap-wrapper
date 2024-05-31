#include <Windows.h>

#include "detail/standalone/entry.h"
#include "app.h"
#include "host_window.h"
#include "helpers.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
int app(int argc, char* argv[])
{
  const clap_plugin_entry* entry{nullptr};
#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  auto lib{Clap::Library()};
  auto clapFilename{std::string(HOSTED_CLAP_NAME).append(".clap")};

  for (const auto& searchPath : Clap::getValidCLAPSearchPaths())
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
    errorBox("No entry as configured");
    return 3;
  }

  HostWindow hostWindow(argc, argv, entry);

  hostWindow.setupPlugin();

  hostWindow.setWindowVisibility(true);

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  MSG msg{};
  int r{};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      errorBox("Error in message loop");
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
