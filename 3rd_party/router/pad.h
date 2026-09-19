#pragma once

#include "board_item.h"

class PAD : public BOARD_ITEM
{
public:
    KICAD_T Type() const override { return PCB_PAD_T; }
};
