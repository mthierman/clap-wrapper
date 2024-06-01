#include <Windows.h>

#include "detail/standalone/entry.h"
#include "app.h"
#include "host_window.h"
#include "helpers.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
int app(int argc, char* argv[])
{
  HostWindow hostWindow(argc, argv);

  return messageLoop();
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
