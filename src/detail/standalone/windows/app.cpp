#include <Windows.h>

#include "detail/standalone/entry.h"
#include "app.h"
#include "host_window.h"
#include "helpers.h"

namespace freeaudio::clap_wrapper::standalone::windows
{
int app(int argc, char* argv[])
{
  auto entry{getClapPluginEntry()};
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
