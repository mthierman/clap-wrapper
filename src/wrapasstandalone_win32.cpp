#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"
#include "detail/standalone/windows/host_window.h"
#include "detail/standalone/windows/helpers.h"

int main(int argc, char* argv[])
{
  const clap_plugin_entry* entry{nullptr};
#ifdef STATICALLY_LINKED_CLAP_ENTRY
  extern const clap_plugin_entry clap_entry;
  entry = &clap_entry;
#else
  auto clapFilename{std::string(HOSTED_CLAP_NAME).append(".clap")};
  LOG << "Loading " << clapFilename << std::endl;

  auto library{Clap::Library()};

  for (const auto& searchPaths : Clap::getValidCLAPSearchPaths())
  {
    auto clapPath{searchPaths / clapFilename};

    if (fs::exists(clapPath) && !entry)
    {
      library.load(clapPath);
      entry = library._pluginEntry;
    }
  }
#endif

  if (!entry)
  {
    freeaudio::clap_wrapper::standalone::windows::helpers::errorBox({"No entry as configured"});

    return 3;
  }

  auto clapPlugin{
      freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, PLUGIN_ID, PLUGIN_INDEX, argc, argv)};

  freeaudio::clap_wrapper::standalone::windows::HostWindow hostWindow{clapPlugin};

  return freeaudio::clap_wrapper::standalone::windows::helpers::messageLoop();
}
