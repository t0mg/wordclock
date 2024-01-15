#pragma once
#include "ClockFace.h"

#ifdef NODO
auto position = ClockFace::LightSensorPosition::Top;
#else
auto position = ClockFace::LightSensorPosition::Bottom;
#endif

EnglishClockFace clockFaceEN(position);
FrenchClockFace clockFaceFR(position);
DutchClockFace clockFaceNL(position);
ItalianClockFace clockFaceIT(position);