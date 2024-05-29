#include <Windows.h>

#include "wrapasstandalone.h"
#include "window.h"
#include "detail/standalone/standalone_details.h"
#include "detail/standalone/entry.h"

int main()
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
    auto clapPath = searchPath / (clapName + ".clap");

    if (fs::exists(clapPath) && !entry)
    {
      lib.load(clapPath);
      entry = lib._pluginEntry;
    }
  }
#endif

  if (!entry) return 0;

  std::string pid{PLUGIN_ID};
  int pindex{PLUGIN_INDEX};

  auto plugin{freeaudio::clap_wrapper::standalone::mainCreatePlugin(entry, pid, pindex, 1, __argv)};

  freeaudio::clap_wrapper::standalone::mainStartAudio();

  freeaudio::clap_wrapper::standalone::windows::Win32Gui win32Gui{};

  auto sah{freeaudio::clap_wrapper::standalone::getStandaloneHost()};

  sah->win32Gui = &win32Gui;

  win32Gui.plugin = plugin;

  if (plugin->_ext._gui)
  {
    auto ui{plugin->_ext._gui};
    auto p{plugin->_plugin};

    ui->create(p, CLAP_WINDOW_API_WIN32, false);

    freeaudio::clap_wrapper::standalone::windows::Window window;

    clap_window win;
    win.api = CLAP_WINDOW_API_WIN32;
    win.win32 = (void*)window.m_hwnd;
    ui->set_parent(p, &win);
    ui->show(p);
  }

  MSG msg{};
  int r{0};

  while ((r = ::GetMessageW(&msg, nullptr, 0, 0)) != 0)
  {
    if (r == -1)
    {
      return 0;
    }

    else
    {
      ::TranslateMessage(&msg);
      ::DispatchMessageW(&msg);
    }
  }

  win32Gui.plugin = nullptr;

  plugin = nullptr;

  freeaudio::clap_wrapper::standalone::mainFinish();

  return 0;
}
