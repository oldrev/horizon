#include "tool_edit_qrcode.hpp"

#include "core/tool_data_window.hpp"
#include "document/idocument.hpp"
#include "imp/imp_interface.hpp"
#include "util/selection_util.hpp"

namespace horizon {
ToolResponse ToolEditQRCode::begin(const ToolArgs &args)
{
    auto qr = doc.r->get_qrcode(sel_find_exactly_one(selection, ObjectType::QRCODE)->uuid);

    imp->dialogs.show_edit_qrcode_window(*qr, true);

    return ToolResponse();
}

ToolResponse ToolEditQRCode::update(const ToolArgs &args)
{
    if (args.type == ToolEventType::DATA) {
        if (auto data = dynamic_cast<const ToolDataWindow *>(args.data.get())) {
            switch (data->event) {
            case ToolDataWindow::Event::OK:
                return ToolResponse::commit();
            case ToolDataWindow::Event::CLOSE:
                return ToolResponse::revert();
            default:;
            }
        }
    }
    return ToolResponse();
}

bool ToolEditQRCode::can_begin()
{
    return sel_find_exactly_one(selection, ObjectType::QRCODE);
}
} // namespace horizon
