#include <iostream>
#include <Windows.h>
#include "detail/standalone/windows/app.h"

int main(int argc, char* argv[])
{
  return freeaudio::clap_wrapper::standalone::windows::run(argc, argv);
}
