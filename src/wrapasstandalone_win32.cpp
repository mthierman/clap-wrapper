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
  auto clapName{std::string(HOSTED_CLAP_NAME)};
  LOG << "Loading " << clapName << std::endl;

  auto lib{Clap::Library()};

  for (const auto& searchPaths : Clap::getValidCLAPSearchPaths())
  {
    auto clapPath{searchPaths / (clapName + ".clap")};

    if (fs::exists(clapPath) && !entry)
    {
      lib.load(clapPath);
      entry = lib._pluginEntry;
    }
  }
#endif

  if (!entry)
  {
    freeaudio::clap_wrapper::standalone::windows::helpers::errorBox({"No entry as configured"});
    freeaudio::clap_wrapper::standalone::windows::helpers::abort(3);
  }

  auto clapPlugin{
      freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, PLUGIN_ID, PLUGIN_INDEX, argc, argv)};

  freeaudio::clap_wrapper::standalone::windows::HostWindow hostWindow{clapPlugin};

  return freeaudio::clap_wrapper::standalone::windows::helpers::messageLoop();
}
