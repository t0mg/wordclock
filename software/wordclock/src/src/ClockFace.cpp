#include "logging.h"

#include <NeoPixelBus.h> // Only need NeoTopology

#include "ClockFace.h"

#include "nodo.h" // Nodo stuff

// The number of LEDs connected before the start of the matrix.
#define NEOPIXEL_SIGNALS 4

// Matrix dimensions.
#define NEOPIXEL_ROWS 11
#define NEOPIXEL_COLUMNS 10

// Number of LEDs on the whole strip.
#define NEOPIXEL_COUNT (NEOPIXEL_ROWS * NEOPIXEL_COLUMNS + NEOPIXEL_SIGNALS)

// static
int ClockFace::pixelCount()
{
  return NEOPIXEL_COUNT;
}

void ClockFace::clearDisplay(void)
{
  std::fill(_state.begin(), _state.end(), false);
}

ClockFace::ClockFace(LightSensorPosition position) : _hour(-1), _minute(-1), _second(-1),
                                                     _position(position), _state(NEOPIXEL_COUNT){};

void ClockFace::setLightSensorPosition(LightSensorPosition position)
{
  _position = position;
}

uint16_t ClockFace::map(int16_t x, int16_t y)
{
  switch (_position)
  {
  case LightSensorPosition::Top:
  {
    static NeoTopology<ColumnMajorAlternating90Layout> sensor_on_top(
        NEOPIXEL_ROWS, NEOPIXEL_COLUMNS);
#ifdef NODO
    int ind, inc;
    // do conversion from normal coordinates to Nodo coordinates
    ind = sensor_on_top.Map(x, y);
    if (ind < 11)
      inc = 1; // one pixel before first row
    else if (ind < 99)
      inc = 2; // two pixels before second row
    else
      inc = 3; // three pixels
    // mirror pixel grid
    ind = 11 * (ind / 11) + 10 - ind % 11;
    return ind + inc;
#else
    return sensor_on_top.Map(x, y) + NEOPIXEL_SIGNALS;
#endif
  }
  case LightSensorPosition::Bottom:
  {
    static NeoTopology<ColumnMajorAlternating270Layout> sensor_on_bottom(
        NEOPIXEL_ROWS, NEOPIXEL_COLUMNS);
    return sensor_on_bottom.Map(x, y) + NEOPIXEL_SIGNALS;
  }
  default:
    DCHECK(false, static_cast<int>(_position));
    return 0;
  }
}

uint16_t ClockFace::mapMinute(Corners corner)
{
  switch (_position)
  {
  case LightSensorPosition::Bottom:
    return (static_cast<uint16_t>(corner) + 2) % 4;
  case LightSensorPosition::Top:
#ifdef NODO
    switch (static_cast<uint16_t>(corner))
    {
    case 0:
      return 113;
    case 1:
      return 101;
    case 2:
      return 12;
    default:
      return 0;
    }
#else
    return static_cast<uint16_t>(corner);
#endif
  default:
    DCHECK(false, static_cast<int>(corner));
    return 0;
  }
}

// Lit a segment in updateState.
void ClockFace::updateSegment(int x, int y, int length)
{
  for (int i = x; i <= x + length - 1; i++)
    _state[map(i, y)] = true;
}

//
// Constants to match the ClockFace.
//
// Custom French face. Can show time with some less usual variations like
// MINUIT TROIS QUARTS or DEUX HEURES PILE. This also includes all the letters
// of the alphabets as well as !. Letters in lowercase below are not used
// by the clock.

// ILbESTjDEUX
// QUATRETROIS
// NEUFUNESEPT
// HUITSIXCINQ
// MIDIXMINUIT
// ONZEwHEURES
// MOINSyLEDIX
// ETTROISDEMI
// VINGT-CINQk
// QUARTSPILE!

// Alt version: same except last 3 lines
// ETbQUARTpmm
// VINGT-CINQu
// ETyDEMIEpam

// Comment out to not use alt version
#define FR_ALT

// All the segments of words on the board. The first too numbers are the
// coordinate of the first letter of the word, the last is the length. A word
// must always be on one row.
#define FR_S_IL 0, 0, 2
#define FR_S_EST 3, 0, 3

#define FR_H_UNE 4, 2, 3
#define FR_H_DEUX 7, 0, 4
#define FR_H_TROIS 6, 1, 5
#define FR_H_QUATRE 0, 1, 6
#define FR_H_CINQ 7, 3, 4
#define FR_H_SIX 4, 3, 3
#define FR_H_SEPT 7, 2, 4
#define FR_H_HUIT 0, 3, 4
#define FR_H_NEUF 0, 2, 4
#define FR_H_DIX 2, 4, 3
#define FR_H_ONZE 0, 5, 4

#define FR_H_HEURE 5, 5, 5
#define FR_H_HEURES 5, 5, 6

#define FR_H_MIDI 0, 4, 4
#define FR_H_MINUIT 5, 4, 6

#define FR_M_MOINS 0, 6, 5
#define FR_M_LE 6, 6, 2
#define FR_M_ET 0, 7, 2
// #define FR_M_TROIS 2, 7, 5 -- unused

#define FR_M_DIX 8, 6, 3
#define FR_M_VINGT 0, 8, 5
#define FR_M_VINGTCINQ 0, 8, 10
#define FR_M_CINQ 6, 8, 4
#ifdef FR_ALT
#define FR_M_ET2 0, 9, 2
#define FR_M_DEMI 3, 9, 5
#define FR_M_QUART 3, 7, 5
#else
#define FR_M_DEMI 7, 7, 4
#define FR_M_QUART 0, 9, 5
#endif
// #define FR_M_QUARTS 0, 9, 6 -- unused
#define FR_M_PILE 6, 9, 4

bool FrenchClockFace::stateForTime(int hour, int minute, int second, bool show_ampm)
{
  if (hour == _hour && minute == _minute)
  {
    return false;
  }
  _hour = hour;
  _minute = minute;

  DLOGLN("update state fr");

  clearDisplay();

  int leftover = minute % 5;
  minute = minute - leftover;

  if (minute >= 35)
    hour = (hour + 1) % 24; // Switch to "TO" minutes the next hour

  updateSegment(FR_S_IL);
  updateSegment(FR_S_EST);

  switch (hour)
  {
  case 0:
    updateSegment(FR_H_MINUIT);
    break;
  case 1:
  case 13:
    updateSegment(FR_H_UNE);
    break;
  case 2:
  case 14:
    updateSegment(FR_H_DEUX);
    break;
  case 3:
  case 15:
    updateSegment(FR_H_TROIS);
    break;
  case 4:
  case 16:
    updateSegment(FR_H_QUATRE);
    break;
  case 5:
  case 17:
    updateSegment(FR_H_CINQ);
    break;
  case 6:
  case 18:
    updateSegment(FR_H_SIX);
    break;
  case 7:
  case 19:
    updateSegment(FR_H_SEPT);
    break;
  case 8:
  case 20:
    updateSegment(FR_H_HUIT);
    break;
  case 9:
  case 21:
    updateSegment(FR_H_NEUF);
    break;
  case 10:
  case 22:
    updateSegment(FR_H_DIX);
    break;
  case 11:
  case 23:
    updateSegment(FR_H_ONZE);
    break;
  case 12:
    updateSegment(FR_H_MIDI);
    break;
  default:
    DLOG("Invalid hour ");
    DLOGLN(hour);
  }
  switch (hour)
  {
  case 0:
  case 12:
    break;
  case 1:
  case 13:
    updateSegment(FR_H_HEURE);
    break;
  default:
    updateSegment(FR_H_HEURES);
    break;
  }

  switch (minute)
  {
  case 0:
    break;
  case 5:
    updateSegment(FR_M_CINQ);
    break;
  case 10:
    updateSegment(FR_M_DIX);
    break;
  case 15:
    updateSegment(FR_M_ET);
    updateSegment(FR_M_QUART);
    break;
  case 20:
    updateSegment(FR_M_VINGT);
    break;
  case 25:
    updateSegment(FR_M_VINGTCINQ);
    break;
  case 30:
    #ifdef FR_ALT
    updateSegment(FR_M_ET2);
    #else
    updateSegment(FR_M_ET);
    #endif
    updateSegment(FR_M_DEMI);
    break;
  case 35:
    updateSegment(FR_M_MOINS);
    updateSegment(FR_M_VINGTCINQ);
    break;
  case 40:
    updateSegment(FR_M_MOINS);
    updateSegment(FR_M_VINGT);
    break;
  case 45:
    updateSegment(FR_M_MOINS);
    updateSegment(FR_M_LE);
    updateSegment(FR_M_QUART);
    break;
  case 50:
    updateSegment(FR_M_MOINS);
    updateSegment(FR_M_DIX);
    break;
  case 55:
    updateSegment(FR_M_MOINS);
    updateSegment(FR_M_CINQ);
    break;
  default:
    DLOG("Invalid minute ");
    DLOGLN(minute);
  }

  switch (leftover)
  {
  case 4:
    _state[mapMinute(TopLeft)] = true;
  case 3: // fall through
    _state[mapMinute(BottomLeft)] = true;
  case 2: // fall through
    _state[mapMinute(BottomRight)] = true;
  case 1: // fall through
    _state[mapMinute(TopRight)] = true;
  case 0: // fall through
    break;
  }
  return true;
}

// Constants to match the EnglishClockFace.
//
// Letters in lowercase below are not used by the clock.

// ITlISasAM PM
// A c QUARTER dc
// TWENTY FIVE x
// HALF s TEN u TO
// PAST bu NINE
// ONE SIX THREE
// FOUR FIVE TWO
// EIGHT ELEVEN
// SEVEN TWELVE
// TEN sz O'CLOCK

// All the segments of words on the board. The first too numbers are the
// coordinate of the first letter of the word, the last is the length. A word
// must always be on one row.
#define EN_S_IT 0, 0, 2
#define EN_S_IS 3, 0, 2

#define EN_H_ONE 0, 5, 3
#define EN_H_TWO 8, 6, 3
#define EN_H_THREE 6, 5, 5
#define EN_H_FOUR 0, 6, 4
#define EN_H_FIVE 4, 6, 4
#define EN_H_SIX 3, 5, 3
#define EN_H_SEVEN 0, 8, 5
#define EN_H_EIGHT 0, 7, 5
#define EN_H_NINE 7, 4, 4
#define EN_H_TEN 0, 9, 3
#define EN_H_ELEVEN 5, 7, 6
#define EN_H_TWELVE 5, 8, 6

#define EN_H_AM 7, 0, 2
#define EN_H_PM 9, 0, 2

#define EN_M_A 0, 1, 1
#define EN_M_PAST 0, 4, 4
#define EN_M_TO 9, 3, 2

#define EN_M_TEN 5, 3, 3
#define EN_M_QUARTER 2, 1, 7
#define EN_M_TWENTY 0, 2, 6
#define EN_M_TWENTYFIVE 0, 2, 10
#define EN_M_FIVE 6, 2, 4
#define EN_M_HALF 0, 3, 4
#define EN_M_QUART 0, 9, 5
#define EN_M_QUARTS 0, 9, 6

#define EN_M_OCLOCK 5, 9, 7

bool EnglishClockFace::stateForTime(int hour, int minute, int second, bool show_ampm)
{
  if (hour == _hour && minute == _minute && show_ampm == _show_ampm)
  {
    return false;
  }
  _hour = hour;
  _minute = minute;
  _show_ampm = show_ampm;

  DLOGLN("update state");

  clearDisplay();

  int leftover = minute % 5;
  minute = minute - leftover;

  if (minute >= 35)
    hour = (hour + 1) % 24; // Switch to "TO" minutes the next hour

  updateSegment(EN_S_IT);
  updateSegment(EN_S_IS);

  if (show_ampm)
  {
    if (hour < 12) // Midday is PM 
    {
      updateSegment(EN_H_AM);
    }
    else
    {
      updateSegment(EN_H_PM);
    }
  }

  switch (hour)
  {
  case 0:
    updateSegment(EN_H_TWELVE);
    break;
  case 1:
  case 13:
    updateSegment(EN_H_ONE);
    break;
  case 2:
  case 14:
    updateSegment(EN_H_TWO);
    break;
  case 3:
  case 15:
    updateSegment(EN_H_THREE);
    break;
  case 4:
  case 16:
    updateSegment(EN_H_FOUR);
    break;
  case 5:
  case 17:
    updateSegment(EN_H_FIVE);
    break;
  case 6:
  case 18:
    updateSegment(EN_H_SIX);
    break;
  case 7:
  case 19:
    updateSegment(EN_H_SEVEN);
    break;
  case 8:
  case 20:
    updateSegment(EN_H_EIGHT);
    break;
  case 9:
  case 21:
    updateSegment(EN_H_NINE);
    break;
  case 10:
  case 22:
    updateSegment(EN_H_TEN);
    break;
  case 11:
  case 23:
    updateSegment(EN_H_ELEVEN);
    break;
  case 12:
    updateSegment(EN_H_TWELVE);
    break;
  default:
    DLOG("Invalid hour ");
    DLOGLN(hour);
  }

  switch (minute)
  {
  case 0:
    updateSegment(EN_M_OCLOCK);
    break;
  case 5:
    updateSegment(EN_M_FIVE);
    updateSegment(EN_M_PAST);
    break;
  case 10:
    updateSegment(EN_M_TEN);
    updateSegment(EN_M_PAST);
    break;
  case 15:
    updateSegment(EN_M_A);
    updateSegment(EN_M_QUARTER);
    updateSegment(EN_M_PAST);
    break;
  case 20:
    updateSegment(EN_M_TWENTY);
    updateSegment(EN_M_PAST);
    break;
  case 25:
    updateSegment(EN_M_TWENTYFIVE);
    updateSegment(EN_M_PAST);
    break;
  case 30:
    updateSegment(EN_M_HALF);
    updateSegment(EN_M_PAST);
    break;
  case 35:
    updateSegment(EN_M_TWENTYFIVE);
    updateSegment(EN_M_TO);
    break;
  case 40:
    updateSegment(EN_M_TWENTY);
    updateSegment(EN_M_TO);
    break;
  case 45:
    updateSegment(EN_M_A);
    updateSegment(EN_M_QUARTER);
    updateSegment(EN_M_TO);
    break;
  case 50:
    updateSegment(EN_M_TEN);
    updateSegment(EN_M_TO);
    break;
  case 55:
    updateSegment(EN_M_FIVE);
    updateSegment(EN_M_TO);
    break;
  default:
    DLOG("Invalid minute ");
    DLOGLN(minute);
  }

  switch (leftover)
  {
  case 4:
    _state[mapMinute(TopLeft)] = true;
  case 3: // fall through
    _state[mapMinute(BottomLeft)] = true;
  case 2: // fall through
    _state[mapMinute(BottomRight)] = true;
  case 1: // fall through
    _state[mapMinute(TopRight)] = true;
  case 0: // fall through
    break;
  }
  return true;
}

//
// Constants to match the Dutch ClockFace.
//
// Custom Dutch face. Letters in lowercase below are not used
// by the clock.

// HETkISaVIJF
// TIENatxVOOR
// OVERmeKWART
// HALFspmOVER
// VOORthgEENs
// TWEEamcDRIE
// VIERVIJFZES
// ZEVENoNEGEN
// ACHTTIENELF
// TWAALFpmUUR
//

// All the segments of words on the board. The first too numbers are the
// coordinate of the first letter of the word, the last is the length. A word
// must always be on one row.
#define NL_S_HET 0, 0, 3
#define NL_S_IS 4, 0, 2

#define NL_H_EEN 7, 4, 3
#define NL_H_TWEE 0, 5, 4
#define NL_H_DRIE 7, 5, 4
#define NL_H_VIER 0, 6, 4
#define NL_H_VIJF 4, 6, 4
#define NL_H_ZES 8, 6, 3
#define NL_H_ZEVEN 0, 7, 5
#define NL_H_ACHT 0, 8, 4
#define NL_H_NEGEN 6, 7, 5
#define NL_H_TIEN 4, 8, 4
#define NL_H_ELF 8, 8, 3
#define NL_H_TWAALF 0, 9, 6

#define NL_H_UUR 8, 9, 3

#define NL_M_VOOR 7, 1, 4
#define NL_M_VOOR2 0, 4, 4
#define NL_M_OVER 0, 2, 4
#define NL_M_OVER2 7, 3, 4
#define NL_M_KWART 6, 2, 5
#define NL_M_TIEN 0, 1, 4
#define NL_M_VIJF 7, 0, 4
#define NL_M_HALF 0, 3, 4

bool DutchClockFace::stateForTime(int hour, int minute, int second,
                                  bool show_ampm)
{
  if (hour == _hour && minute == _minute)
  {
    return false;
  }
  _hour = hour;
  _minute = minute;

  DLOGLN("update state");

  clearDisplay();

  int leftover = minute % 5;
  minute = minute - leftover;

  if (minute > 15)
    hour = (hour + 1) % 24; // Switch to "TO" minutes the next hour

  updateSegment(NL_S_HET);
  updateSegment(NL_S_IS);

  switch (hour)
  {
  case 1:
  case 13:
    updateSegment(NL_H_EEN);
    break;
  case 2:
  case 14:
    updateSegment(NL_H_TWEE);
    break;
  case 3:
  case 15:
    updateSegment(NL_H_DRIE);
    break;
  case 4:
  case 16:
    updateSegment(NL_H_VIER);
    break;
  case 5:
  case 17:
    updateSegment(NL_H_VIJF);
    break;
  case 6:
  case 18:
    updateSegment(NL_H_ZES);
    break;
  case 7:
  case 19:
    updateSegment(NL_H_ZEVEN);
    break;
  case 8:
  case 20:
    updateSegment(NL_H_ACHT);
    break;
  case 9:
  case 21:
    updateSegment(NL_H_NEGEN);
    break;
  case 10:
  case 22:
    updateSegment(NL_H_TIEN);
    break;
  case 11:
  case 23:
    updateSegment(NL_H_ELF);
    break;
  case 0:
  case 12:
    updateSegment(NL_H_TWAALF);
    break;
  default:
    DLOG("Invalid hour ");
    DLOGLN(hour);
  }

  switch (minute)
  {
  case 0:
    updateSegment(NL_H_UUR);
    break;
  case 5:
    updateSegment(NL_M_VIJF);
    updateSegment(NL_M_OVER);
    break;
  case 10:
    updateSegment(NL_M_TIEN);
    updateSegment(NL_M_OVER);
    break;
  case 15:
    updateSegment(NL_M_KWART);
    updateSegment(NL_M_OVER2);
    break;
  case 20:
    updateSegment(NL_M_TIEN);
    updateSegment(NL_M_VOOR);
    updateSegment(NL_M_HALF);
    break;
  case 25:
    updateSegment(NL_M_VIJF);
    updateSegment(NL_M_VOOR);
    updateSegment(NL_M_HALF);
    break;
  case 30:
    updateSegment(NL_M_HALF);
    break;
  case 35:
    updateSegment(NL_M_VIJF);
    updateSegment(NL_M_OVER);
    updateSegment(NL_M_HALF);
    break;
  case 40:
    updateSegment(NL_M_TIEN);
    updateSegment(NL_M_OVER);
    updateSegment(NL_M_HALF);
    break;
  case 45:
    updateSegment(NL_M_KWART);
    updateSegment(NL_M_VOOR2);
    break;
  case 50:
    updateSegment(NL_M_VOOR);
    updateSegment(NL_M_TIEN);
    break;
  case 55:
    updateSegment(NL_M_VOOR);
    updateSegment(NL_M_VIJF);
    break;
  default:
    DLOG("Invalid minute ");
    DLOGLN(minute);
  }

  switch (leftover)
  {
  case 4:
    _state[mapMinute(TopLeft)] = true;
  case 3: // fall through
    _state[mapMinute(BottomLeft)] = true;
  case 2: // fall through
    _state[mapMinute(BottomRight)] = true;
  case 1: // fall through
    _state[mapMinute(TopRight)] = true;
  case 0: // fall through
    break;
  }
  return true;
}

// Constants to match the ItalianClockFace.
//
// Letters in lowercase below are not used by the clock.

//SONOrLEbORE
//ÈrLUNAsDUEz
//TREOTTONOVE
//DIECIUNDICI
//DODICISETTE
//QUATTROcSEI
//CINQUEaMENO
//EcUNoQUARTO
//VENTICINQUE
//DIECIpMEZZA

// All the segments of words on the board. The first too numbers are the
// coordinate of the first letter of the word, the last is the length. A word
// must always be on one row.


#define IT_S_SONO 0, 0, 4
#define IT_S_LE 5, 0, 2
#define IT_S_E 0, 1, 1
#define IT_S_ORE 8, 0, 3

#define IT_H_LUNA 2, 1, 4
#define IT_H_DUE 7, 1, 3
#define IT_H_TRE 0, 2, 3
#define IT_H_QUATTRO 0, 5, 7
#define IT_H_CINQUE 0, 6, 6
#define IT_H_SEI 8, 5, 3
#define IT_H_SETTE 6, 4, 5
#define IT_H_OTTO 3, 2, 4
#define IT_H_NOVE 7, 2, 4
#define IT_H_DIECI 0, 3, 5
#define IT_H_UNDICI 5, 3, 6
#define IT_H_DODICI 0, 4, 6

#define IT_M_E 0, 7, 1
#define IT_M_MENO 7, 6, 4
#define IT_M_DIECI 0, 9, 5
#define IT_M_UN 2, 7, 2
#define IT_M_QUARTO 5, 7, 6
#define IT_M_VENTI 0, 8, 5
#define IT_M_CINQUE 5, 8, 6
#define IT_M_MEZZA 6, 9, 5



bool ItalianClockFace::stateForTime(int hour, int minute, int second, bool show_ampm)
{
  if (hour == _hour && minute == _minute)
  {
    return false;
  }
  _hour = hour;
  _minute = minute;

  DLOGLN("update state");

  // Reset the board to all black
  clearDisplay();

  int leftover = minute % 5;
  minute = minute - leftover;

  if (minute >= 35)
    hour = (hour + 1) % 24; // Switch to "TO" minutes the next hour

  // Special case for one o'clock
  if (hour == 1 || hour == 13)
  {
    updateSegment(IT_S_E);
    updateSegment(IT_H_LUNA);
  }
  else // Normal case for other hours
  {
    updateSegment(IT_S_SONO);
    updateSegment(IT_S_LE);
    switch (hour)
    {
    case 0:
      updateSegment(IT_H_DODICI);
      break;
    case 2:
    case 14:
      updateSegment(IT_H_DUE);
      break;
    case 3:
    case 15:
      updateSegment(IT_H_TRE);
      break;
    case 4:
    case 16:
      updateSegment(IT_H_QUATTRO);
      break;
    case 5:
    case 17:
      updateSegment(IT_H_CINQUE);
      break;
    case 6:
    case 18:
      updateSegment(IT_H_SEI);
      break;
    case 7:
    case 19:
      updateSegment(IT_H_SETTE);
      break;
    case 8:
    case 20:
      updateSegment(IT_H_OTTO);
      break;
    case 9:
    case 21:
      updateSegment(IT_H_NOVE);
      break;
    case 10:
    case 22:
      updateSegment(IT_H_DIECI);
      break;
    case 11:
    case 23:
      updateSegment(IT_H_UNDICI);
      break;
    case 12:
      updateSegment(IT_H_DODICI);
      break;
    default:
      DLOG("Invalid hour ");
      DLOGLN(hour);
    }
  }

switch (minute)
{
case 0:
  // Check if the hour value is 1 or 13
  if (hour != 1 && hour != 13)
  {
    // If not, display "ORE"
    updateSegment(IT_S_ORE);
  }
  break;
// The rest of the switch statement

  case 5:
    updateSegment(IT_M_E);
    updateSegment(IT_M_CINQUE);
    break;
  case 10:
    updateSegment(IT_M_E);
    updateSegment(IT_M_DIECI);
    break;
  case 15:
    updateSegment(IT_M_E);
    updateSegment(IT_M_UN);
    updateSegment(IT_M_QUARTO);
    break;
  case 20:
    updateSegment(IT_M_E);
    updateSegment(IT_M_VENTI);
    break;
  case 25:
    updateSegment(IT_M_E);
    updateSegment(IT_M_VENTI);
    updateSegment(IT_M_CINQUE);
    break;
  case 30:
    updateSegment(IT_M_E);
    updateSegment(IT_M_MEZZA);
    break;
  case 35:
    updateSegment(IT_M_MENO);
    updateSegment(IT_M_VENTI);
    updateSegment(IT_M_CINQUE);
    break;
  case 40:
    updateSegment(IT_M_MENO);
    updateSegment(IT_M_VENTI);
    break;
  case 45:
    updateSegment(IT_M_MENO);
    updateSegment(IT_M_UN);
    updateSegment(IT_M_QUARTO);
    break;
  case 50:
    updateSegment(IT_M_MENO);
    updateSegment(IT_M_DIECI);
    break;
  case 55:
    updateSegment(IT_M_MENO);
    updateSegment(IT_M_CINQUE);
    break;
  default:
    DLOG("Invalid minute ");
    DLOGLN(minute);
  }

  switch (leftover)
  {
  case 4:
    _state[mapMinute(TopLeft)] = true;
  case 3: // fall through
    _state[mapMinute(BottomLeft)] = true;
  case 2: // fall through
    _state[mapMinute(BottomRight)] = true;
  case 1: // fall through
    _state[mapMinute(TopRight)] = true;
  case 0: // fall through
    break;
  }
  return true;
}





bool RingClockFace::stateForTime(int hour, int minute, int second, bool show_ampm)
{
  if (hour == _hour && minute == _minute)
  {
    return false;
  }
  _hour = hour;
  _minute = minute;

  DLOGLN("update state");
  clearDisplay();
  int h = (hour%12)*2+((minute >= 30)?1:0);
  updateSegment(h,0,1);
  updateSegment(24+minute,0,1);


  return true;
}