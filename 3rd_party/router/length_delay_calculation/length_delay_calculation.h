#pragma once

#include <geometry/shape_line_chain.h>
#include <layer_ids.h>
#include <math/vector2d.h>

class PCB_VIA;
class PAD;

namespace LENGTH_DELAY_CALCULATION
{
inline bool IsPointInsideViaPad(const PCB_VIA *, const VECTOR2I &, PCB_LAYER_ID)
{
    return false;
}

inline void OptimiseTraceInVia(SHAPE_LINE_CHAIN &, const PCB_VIA *, PCB_LAYER_ID)
{
}

inline void OptimiseTraceInPad(SHAPE_LINE_CHAIN &, const PAD *, PCB_LAYER_ID)
{
}
}
