#pragma once

#include <vector>

#include <RtAudio.h>
#include <RtMidi.h>
#include <json/json.hpp>

using json = nlohmann::json;

namespace freeaudio::clap_wrapper::standalone::windows
{
struct Settings
{
  std::string inputSelection;
  std::string outputSelection;

  std::vector<RtAudio::DeviceInfo> outDevices;
  std::vector<RtAudio::DeviceInfo> inDevices;
  unsigned int sampleRate;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Settings, inputSelection, outputSelection, sampleRate)
};
}  // namespace freeaudio::clap_wrapper::standalone::windows
