#pragma once

#include "board_item.h"

class PCB_TRACK : public BOARD_ITEM
{
public:
    KICAD_T Type() const override { return PCB_TRACE_T; }
};

class PCB_VIA : public PCB_TRACK
{
public:
    KICAD_T Type() const override { return PCB_VIA_T; }
};

