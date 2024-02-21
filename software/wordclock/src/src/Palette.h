#pragma once

#include <vector>
#include <NeoPixelBus.h>

namespace Palette
{
  std::vector<RgbColor> stringToRgb(String payload);
};