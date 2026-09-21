#pragma once
#include "tool_helper_move.hpp"

namespace horizon {

class ToolPlaceQRCode : public ToolHelperMove {
public:
    using ToolHelperMove::ToolHelperMove;

    ToolResponse begin(const ToolArgs &args) override;
    ToolResponse update(const ToolArgs &args) override;
    bool can_begin() override;

    std::set<InToolActionID> get_actions() const override
    {
        using I = InToolActionID;
        return {
                I::LMB, I::CANCEL, I::RMB, I::EDIT, I::ROTATE, I::MIRROR,
        };
    }

private:
    class QRCode *temp = nullptr;
};

} // namespace horizon
