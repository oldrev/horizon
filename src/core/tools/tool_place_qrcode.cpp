#include "tool_place_qrcode.hpp"
#include "common/qrcode.hpp"
#include "core/tool_id.hpp"
#include "dialogs/edit_qrcode_window.hpp"
#include "document/idocument.hpp"
#include "imp/imp_interface.hpp"

namespace horizon {

ToolResponse ToolPlaceQRCode::begin(const ToolArgs &args)
{
    temp = doc.r->insert_qrcode(UUID::random());
    imp->set_snap_filter({{ObjectType::QRCODE, temp->uuid}});
    temp->layer = args.work_layer;
    temp->placement.shift = args.coords;
    selection = {{temp->uuid, ObjectType::QRCODE}};

    imp->tool_bar_set_actions({
            {InToolActionID::LMB},
            {InToolActionID::RMB},
            {InToolActionID::ROTATE},
            {InToolActionID::MIRROR},
            {InToolActionID::EDIT, "change QR code"},
    });

    auto dia = imp->dialogs.show_edit_qrcode_window(*temp, false);
    dia->focus_text();

    return {};
}

ToolResponse ToolPlaceQRCode::update(const ToolArgs &args)
{
    if (args.type == ToolEventType::MOVE) {
        if (imp->dialogs.get_nonmodal() == nullptr)
            temp->placement.shift = args.coords;
    }
    else if (args.type == ToolEventType::ACTION) {
        switch (args.action) {
        case InToolActionID::LMB:
            if (imp->dialogs.get_nonmodal() == nullptr)
                return ToolResponse::commit();
            break;

        case InToolActionID::RMB:
        case InToolActionID::CANCEL:
            return ToolResponse::revert();

        case InToolActionID::EDIT: {
            auto dia = imp->dialogs.show_edit_qrcode_window(*temp, false);
            dia->focus_text();
        } break;

        case InToolActionID::ROTATE:
        case InToolActionID::MIRROR:
            move_mirror_or_rotate(temp->placement.shift, args.action == InToolActionID::ROTATE);
            break;

        default:;
        }
    }
    else if (args.type == ToolEventType::LAYER_CHANGE) {
        temp->layer = args.work_layer;
    }
    else if (args.type == ToolEventType::DATA) {
        if (auto data = dynamic_cast<const ToolDataWindow *>(args.data.get())) {
            if (data->event == ToolDataWindow::Event::OK || data->event == ToolDataWindow::Event::CLOSE) {
                if (data->event == ToolDataWindow::Event::OK)
                    imp->dialogs.close_nonmodal();
            }
        }
    }
    return {};
}

bool ToolPlaceQRCode::can_begin()
{
    return doc.r->has_object_type(ObjectType::QRCODE);
}

} // namespace horizon
