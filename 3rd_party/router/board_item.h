#pragma once

#include "core/typeinfo.h"
#include "wx_compat.h"
#include "kiid.h"

class BOARD_ITEM
{
public:
    KIID m_Uuid;
    virtual ~BOARD_ITEM() = default;
    virtual KICAD_T Type() const = 0;
    virtual int GetLayer() const { return 0; }
    virtual wxString GetItemDescription(void *, bool) const { return {}; }
};
