
#include "parameter.h"

namespace Clap::AUv2
{

Parameter::Parameter(clap_param_info_t &clap_param)
{
  _info = clap_param;
  _cfstring = CFStringCreateWithCString(NULL, _info.name, kCFStringEncodingUTF8);
}

void Parameter::resetInfo(const clap_param_info_t &i)
{
  _info = i;
  CFRelease(_cfstring);
  _cfstring = CFStringCreateWithCString(NULL, _info.name, kCFStringEncodingUTF8);
}
Parameter::~Parameter()
{
  CFRelease(_cfstring);
}

}  // namespace Clap::AUv2
