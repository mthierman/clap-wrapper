#include "win32gui.h"
#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"

int main(int argc, char** argv)
{
  const clap_plugin_entry* entry{nullptr};

#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  // Library shenanigans t/k
  auto clapName{std::string{HOSTED_CLAP_NAME}};
  LOG << "Loading " << clapName << std::endl;

  auto searchPaths{Clap::getValidCLAPSearchPaths()};

  auto lib{Clap::Library()};

  for (const auto& searchPath : searchPaths)
  {
    auto clapPath{searchPath / (clapName + ".clap")};

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

  freeaudio::clap_wrapper::standalone::windows::Win32Gui win32Gui{};

  win32Gui.initialize(freeaudio::clap_wrapper::standalone::getStandaloneHost());

  win32Gui.setPlugin(freeaudio::clap_wrapper::standalone::mainCreatePlugin(
      entry, std::string{PLUGIN_ID}, PLUGIN_INDEX, 1, (char**)argv));

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  win32Gui.createHostWindow();

  win32Gui.setupPlugin();

  win32Gui.runLoop();

  freeaudio::clap_wrapper::standalone::mainFinish();

  return 0;
}
