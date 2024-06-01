#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"
#include "detail/standalone/windows/host_window.h"
#include "detail/standalone/windows/helpers.h"

int main(int argc, char **argv)
{
  const clap_plugin_entry *entry{nullptr};
#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  // Library shenanigans t/k
  std::string clapName{HOSTED_CLAP_NAME};
  LOG << "Loading " << clapName << std::endl;

  auto pts = Clap::getValidCLAPSearchPaths();

  auto lib = Clap::Library();

  for (const auto &searchPaths : pts)
  {
    auto clapPath = searchPaths / (clapName + ".clap");

    if (fs::exists(clapPath) && !entry)
    {
      lib.load(clapPath);
      entry = lib._pluginEntry;
    }
  }

#endif

  if (!entry)
  {
    std::cerr << "Clap Standalone: No Entry as configured" << std::endl;
    return 3;
  }

  auto plugin{freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, PLUGIN_ID, PLUGIN_INDEX, argc,
                                                                    (char **)argv)};

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  freeaudio::clap_wrapper::standalone::windows::HostWindow hostWindow;

  hostWindow.m_plugin = plugin;

  hostWindow.setupPlugin();

  hostWindow.setWindowVisibility(true);

  return freeaudio::clap_wrapper::standalone::windows::messageLoop();
}
