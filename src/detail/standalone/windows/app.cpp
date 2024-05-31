#include "app.h"
#include <exception>

namespace freeaudio::clap_wrapper::standalone::windows
{
int run(int argc, char** argv)
{
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
