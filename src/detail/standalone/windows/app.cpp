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
  hostWindow.setupPlugin();
  hostWindow.setWindowVisibility(true);

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  messageLoop();

  return freeaudio::clap_wrapper::standalone::mainFinish();
}
}  // namespace freeaudio::clap_wrapper::standalone::windows
