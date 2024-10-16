#include <vector>
#include <NeoPixelBus.h>

/*
We initially used a 256 color palette before realizing it was not useful in practice and choosing a 16 color palette instead.
This 256 color palette was taken from https://lospec.com/palette-list/macintosh-8-bit-system-palette
It is the Macintosh 8-Bit default system palette stored in the ROM BIOS of various classic Macintosh computer systems.
// const std::array<RgbColor, 256> COLOR_LOOKUP_TABLE = {RgbColor(0, 0, 0), RgbColor(11, 11, 11), RgbColor(34, 34, 34), RgbColor(68, 68, 68), RgbColor(85, 85, 85), RgbColor(119, 119, 119), RgbColor(136, 136, 136), RgbColor(170, 170, 170), RgbColor(187, 187, 187), RgbColor(221, 221, 221), RgbColor(238, 238, 238), RgbColor(0, 0, 11), RgbColor(0, 0, 34), RgbColor(0, 0, 68), RgbColor(0, 0, 85), RgbColor(0, 0, 119), RgbColor(0, 0, 136), RgbColor(0, 0, 170), RgbColor(0, 0, 187), RgbColor(0, 0, 221), RgbColor(0, 0, 238), RgbColor(0, 11, 0), RgbColor(0, 34, 0), RgbColor(0, 68, 0), RgbColor(0, 85, 0), RgbColor(0, 119, 0), RgbColor(0, 136, 0), RgbColor(0, 170, 0), RgbColor(0, 187, 0), RgbColor(0, 221, 0), RgbColor(0, 238, 0), RgbColor(11, 0, 0), RgbColor(34, 0, 0), RgbColor(68, 0, 0), RgbColor(85, 0, 0), RgbColor(119, 0, 0), RgbColor(136, 0, 0), RgbColor(170, 0, 0), RgbColor(187, 0, 0), RgbColor(221, 0, 0), RgbColor(238, 0, 0), RgbColor(0, 0, 51), RgbColor(0, 0, 102), RgbColor(0, 0, 153), RgbColor(0, 0, 204), RgbColor(0, 0, 255), RgbColor(0, 51, 0), RgbColor(0, 51, 51), RgbColor(0, 51, 102), RgbColor(0, 51, 153), RgbColor(0, 51, 204), RgbColor(0, 51, 255), RgbColor(0, 102, 0), RgbColor(0, 102, 51), RgbColor(0, 102, 102), RgbColor(0, 102, 153), RgbColor(0, 102, 204), RgbColor(0, 102, 255), RgbColor(0, 153, 0), RgbColor(0, 153, 51), RgbColor(0, 153, 102), RgbColor(0, 153, 153), RgbColor(0, 153, 204), RgbColor(0, 153, 255), RgbColor(0, 204, 0), RgbColor(0, 204, 51), RgbColor(0, 204, 102), RgbColor(0, 204, 153), RgbColor(0, 204, 204), RgbColor(0, 204, 255), RgbColor(0, 255, 0), RgbColor(0, 255, 51), RgbColor(0, 255, 102), RgbColor(0, 255, 153), RgbColor(0, 255, 204), RgbColor(0, 255, 255), RgbColor(51, 0, 0), RgbColor(51, 0, 51), RgbColor(51, 0, 102), RgbColor(51, 0, 153), RgbColor(51, 0, 204), RgbColor(51, 0, 255), RgbColor(51, 51, 0), RgbColor(51, 51, 51), RgbColor(51, 51, 102), RgbColor(51, 51, 153), RgbColor(51, 51, 204), RgbColor(51, 51, 255), RgbColor(51, 102, 0), RgbColor(51, 102, 51), RgbColor(51, 102, 102), RgbColor(51, 102, 153), RgbColor(51, 102, 204), RgbColor(51, 102, 255), RgbColor(51, 153, 0), RgbColor(51, 153, 51), RgbColor(51, 153, 102), RgbColor(51, 153, 153), RgbColor(51, 153, 204), RgbColor(51, 153, 255), RgbColor(51, 204, 0), RgbColor(51, 204, 51), RgbColor(51, 204, 102), RgbColor(51, 204, 153), RgbColor(51, 204, 204), RgbColor(51, 204, 255), RgbColor(51, 255, 0), RgbColor(51, 255, 51), RgbColor(51, 255, 102), RgbColor(51, 255, 153), RgbColor(51, 255, 204), RgbColor(51, 255, 255), RgbColor(102, 0, 0), RgbColor(102, 0, 51), RgbColor(102, 0, 102), RgbColor(102, 0, 153), RgbColor(102, 0, 204), RgbColor(102, 0, 255), RgbColor(102, 51, 0), RgbColor(102, 51, 51), RgbColor(102, 51, 102), RgbColor(102, 51, 153), RgbColor(102, 51, 204), RgbColor(102, 51, 255), RgbColor(102, 102, 0), RgbColor(102, 102, 51), RgbColor(102, 102, 102), RgbColor(102, 102, 153), RgbColor(102, 102, 204), RgbColor(102, 102, 255), RgbColor(102, 153, 0), RgbColor(102, 153, 51), RgbColor(102, 153, 102), RgbColor(102, 153, 153), RgbColor(102, 153, 204), RgbColor(102, 153, 255), RgbColor(102, 204, 0), RgbColor(102, 204, 51), RgbColor(102, 204, 102), RgbColor(102, 204, 153), RgbColor(102, 204, 204), RgbColor(102, 204, 255), RgbColor(102, 255, 0), RgbColor(102, 255, 51), RgbColor(102, 255, 102), RgbColor(102, 255, 153), RgbColor(102, 255, 204), RgbColor(102, 255, 255), RgbColor(153, 0, 0), RgbColor(153, 0, 51), RgbColor(153, 0, 102), RgbColor(153, 0, 153), RgbColor(153, 0, 204), RgbColor(153, 0, 255), RgbColor(153, 51, 0), RgbColor(153, 51, 51), RgbColor(153, 51, 102), RgbColor(153, 51, 153), RgbColor(153, 51, 204), RgbColor(153, 51, 255), RgbColor(153, 102, 0), RgbColor(153, 102, 51), RgbColor(153, 102, 102), RgbColor(153, 102, 153), RgbColor(153, 102, 204), RgbColor(153, 102, 255), RgbColor(153, 153, 0), RgbColor(153, 153, 51), RgbColor(153, 153, 102), RgbColor(153, 153, 153), RgbColor(153, 153, 204), RgbColor(153, 153, 255), RgbColor(153, 204, 0), RgbColor(153, 204, 51), RgbColor(153, 204, 102), RgbColor(153, 204, 153), RgbColor(153, 204, 204), RgbColor(153, 204, 255), RgbColor(153, 255, 0), RgbColor(153, 255, 51), RgbColor(153, 255, 102), RgbColor(153, 255, 153), RgbColor(153, 255, 204), RgbColor(153, 255, 255), RgbColor(204, 0, 0), RgbColor(204, 0, 51), RgbColor(204, 0, 102), RgbColor(204, 0, 153), RgbColor(204, 0, 204), RgbColor(204, 0, 255), RgbColor(204, 51, 0), RgbColor(204, 51, 51), RgbColor(204, 51, 102), RgbColor(204, 51, 153), RgbColor(204, 51, 204), RgbColor(204, 51, 255), RgbColor(204, 102, 0), RgbColor(204, 102, 51), RgbColor(204, 102, 102), RgbColor(204, 102, 153), RgbColor(204, 102, 204), RgbColor(204, 102, 255), RgbColor(204, 153, 0), RgbColor(204, 153, 51), RgbColor(204, 153, 102), RgbColor(204, 153, 153), RgbColor(204, 153, 204), RgbColor(204, 153, 255), RgbColor(204, 204, 0), RgbColor(204, 204, 51), RgbColor(204, 204, 102), RgbColor(204, 204, 153), RgbColor(204, 204, 204), RgbColor(204, 204, 255), RgbColor(204, 255, 0), RgbColor(204, 255, 51), RgbColor(204, 255, 102), RgbColor(204, 255, 153), RgbColor(204, 255, 204), RgbColor(204, 255, 255), RgbColor(255, 0, 0), RgbColor(255, 0, 51), RgbColor(255, 0, 102), RgbColor(255, 0, 153), RgbColor(255, 0, 204), RgbColor(255, 0, 255), RgbColor(255, 51, 0), RgbColor(255, 51, 51), RgbColor(255, 51, 102), RgbColor(255, 51, 153), RgbColor(255, 51, 204), RgbColor(255, 51, 255), RgbColor(255, 102, 0), RgbColor(255, 102, 51), RgbColor(255, 102, 102), RgbColor(255, 102, 153), RgbColor(255, 102, 204), RgbColor(255, 102, 255), RgbColor(255, 153, 0), RgbColor(255, 153, 51), RgbColor(255, 153, 102), RgbColor(255, 153, 153), RgbColor(255, 153, 204), RgbColor(255, 153, 255), RgbColor(255, 204, 0), RgbColor(255, 204, 51), RgbColor(255, 204, 102), RgbColor(255, 204, 153), RgbColor(255, 204, 204), RgbColor(255, 204, 255), RgbColor(255, 255, 0), RgbColor(255, 255, 51), RgbColor(255, 255, 102), RgbColor(255, 255, 153), RgbColor(255, 255, 204), RgbColor(255, 255, 255)};

The 16 color palette is from Wikipedia https://en.wikipedia.org/wiki/List_of_software_palettes#Apple_Macintosh_default_16-color_palette

Here's a handy JS snippet to create the lookup table from the Hex values:
// const palette = ["ffffff", "fbf305", ...];
// palette.map(c => `RgbColor(${parseInt(c.slice(0,2),16)},${parseInt(c.slice(2,4),16)},${parseInt(c.slice(4,6),16)})`).join(",");
*/
#ifdef NODO
// Dimmed version (non gamma-corrected: just 2/3 of the original RGB values)
const RgbColor WHITE = RgbColor(168, 168, 168);
const std::array<RgbColor, 16> COLOR_LOOKUP_TABLE PROGMEM = {
    WHITE,
    RgbColor(166, 160, 3),
    RgbColor(110, 66, 2),
    RgbColor(146, 6, 4),
    RgbColor(160, 5, 87),
    RgbColor(71, 0, 165),
    RgbColor(0, 0, 139),
    RgbColor(1, 113, 154),
    RgbColor(20, 120, 13),
    RgbColor(0, 66, 12),
    RgbColor(57, 29, 3),
    RgbColor(95, 113, 58),
    RgbColor(192, 75, 127),
    RgbColor(84, 84, 84),
    RgbColor(42, 42, 42),
    RgbColor(0, 0, 0)};
#else
// Original values
const RgbColor WHITE = RgbColor(255, 255, 255);
const std::array<RgbColor, 16> COLOR_LOOKUP_TABLE PROGMEM = {
    WHITE,
    RgbColor(251, 243, 5),
    RgbColor(255, 100, 3),
    RgbColor(221, 9, 7),
    RgbColor(242, 8, 132),
    RgbColor(71, 0, 165),
    RgbColor(0, 0, 211),
    RgbColor(2, 171, 234),
    RgbColor(31, 183, 20),
    RgbColor(0, 100, 18),
    RgbColor(86, 44, 5),
    RgbColor(144, 113, 58),
    RgbColor(192, 192, 192),
    RgbColor(128, 128, 128),
    RgbColor(64, 64, 64),
    RgbColor(0, 0, 0)};
#endif

namespace Palette
{
  const int CHARS_PER_COLOR = 1; // use 2 for 256 colors palette.

  static std::vector<RgbColor> stringToRgb(String payload, RgbColor defaultColor)
  {
    std::string str = payload.c_str();

    std::vector<RgbColor> colors;
    colors.reserve(110); // Pre-allocate space for efficiency

    for (int i = 0; i < min(static_cast<int>(str.length()), 110 * CHARS_PER_COLOR); i += CHARS_PER_COLOR)
    {
      // Combine CHARS_PER_COLOR characters into a single byte value
      try
      {
        unsigned int colorIndex = std::stoul(str.substr(i, CHARS_PER_COLOR), nullptr, 16);
        // Check if the color index is within valid range
        if (colorIndex >= COLOR_LOOKUP_TABLE.size())
        {
          colors.push_back(defaultColor);
        }
        else
        {
          colors.push_back(COLOR_LOOKUP_TABLE[colorIndex]);
        }
      }
      // If the string is not parseable, assume default color
      catch (std::invalid_argument const &ex)
      {
        colors.push_back(defaultColor);
      }
    }

    return colors;
  }
}