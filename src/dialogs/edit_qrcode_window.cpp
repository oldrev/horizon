#include "edit_qrcode_window.hpp"
#include "common/object_descr.hpp"
#include "common/qrcode.hpp"
#include "util/geom_util.hpp"
#include "util/gtk_util.hpp"
#include "widgets/spin_button_dim.hpp"
#include "widgets/text_editor.hpp"

namespace horizon {

EditQRCodeWindow::EditQRCodeWindow(Gtk::Window *parent, ImpInterface *intf, QRCode &code, bool use_ok)
    : ToolWindow(parent, intf), qr(code)
{
    set_title("Edit QR Code");
    set_use_ok(use_ok);

    auto grid = Gtk::manage(new Gtk::Grid);
    grid->property_margin() = 20;
    grid->set_row_spacing(10);
    grid->set_column_spacing(10);

    editor = Gtk::manage(new TextEditor(TextEditor::Lines::MULTI));
    editor->set_hexpand(true);
    grid->attach(*editor, 0, 0, 2, 1);
    editor->set_text(qr.text, TextEditor::Select::YES);
    editor->signal_changed().connect([this] {
        qr.text = editor->get_text();
        update_info();
        emit_event(ToolDataWindow::Event::UPDATE);
    });
    editor->signal_activate().connect([this] { emit_event(ToolDataWindow::Event::OK); });

    int top = 1;

    sp_module_size = Gtk::manage(new SpinButtonDim);
    sp_module_size->set_hexpand(true);
    sp_module_size->set_range(.1_mm, 10_mm);
    sp_module_size->set_increments(.1_mm, .05_mm);
    sp_module_size->set_value(qr.module_size);
    sp_module_size->signal_value_changed().connect([this] {
        qr.module_size = sp_module_size->get_value_as_int();
        update_info();
        emit_event(ToolDataWindow::Event::UPDATE);
    });
    spinbutton_connect_activate(sp_module_size, [this] { emit_event(ToolDataWindow::Event::OK); });
    grid_attach_label_and_widget(grid, "Module Size", sp_module_size, top);

    combo_ecc = Gtk::manage(new Gtk::ComboBoxText);
    {
        auto &items = object_descriptions.at(ObjectType::QRCODE).properties.at(ObjectProperty::ID::ECC).enum_items;
        for (const auto &[i, name] : items) {
            combo_ecc->append(std::to_string(i), name);
        }
    }
    combo_ecc->set_active_id(std::to_string(static_cast<int>(qr.ecc)));
    combo_ecc->signal_changed().connect([this] {
        qr.ecc = static_cast<QRCode::ECC>(std::stoi(combo_ecc->get_active_id()));
        update_info();
        emit_event(ToolDataWindow::Event::UPDATE);
    });
    grid_attach_label_and_widget(grid, "Error Correction", combo_ecc, top);

    sp_border = Gtk::manage(new Gtk::SpinButton);
    sp_border->set_range(0, QRCode::max_border_modules);
    sp_border->set_increments(1, 1);
    sp_border->set_value(qr.border_modules);
    sp_border->signal_value_changed().connect([this] {
        qr.border_modules = sp_border->get_value_as_int();
        update_info();
        emit_event(ToolDataWindow::Event::UPDATE);
    });
    spinbutton_connect_activate(sp_border, [this] { emit_event(ToolDataWindow::Event::OK); });
    grid_attach_label_and_widget(grid, "Quiet Zone", sp_border, top);

    cb_inverted = Gtk::manage(new Gtk::CheckButton("Draw the background instead of the modules"));
    cb_inverted->set_active(qr.inverted);
    cb_inverted->signal_toggled().connect([this] {
        qr.inverted = cb_inverted->get_active();
        emit_event(ToolDataWindow::Event::UPDATE);
    });
    grid_attach_label_and_widget(grid, "Inverted", cb_inverted, top);

    {
        // which way around it has to be depends on how the drawn material looks
        const std::string hint =
                "Enable this if the drawn material is lighter than the board, e.g. white silkscreen "
                "or exposed copper.";
        auto la = Gtk::manage(new Gtk::Label(hint));
        la->set_xalign(0);
        la->set_line_wrap(true);
        la->get_style_context()->add_class("dim-label");
        grid->attach(*la, 1, top++, 1, 1);
    }

    la_info = Gtk::manage(new Gtk::Label);
    la_info->set_xalign(0);
    la_info->set_line_wrap(true);
    la_info->get_style_context()->add_class("dim-label");
    grid_attach_label_and_widget(grid, "Result", la_info, top);

    update_info();

    grid->show_all();
    add(*grid);
}

void EditQRCodeWindow::update_info()
{
    const auto &error = qr.get_error();
    if (!error.empty()) {
        la_info->set_text(error);
        return;
    }
    const auto n = qr.get_matrix_size();
    if (n == 0) {
        la_info->set_text("No data");
        return;
    }
    la_info->set_text(std::to_string(n) + " × " + std::to_string(n) + " modules, "
                      + dim_to_string(qr.get_total_size(), false));
}

void EditQRCodeWindow::focus_text()
{
    editor->grab_focus();
}

void EditQRCodeWindow::focus_module_size()
{
    sp_module_size->grab_focus();
}

} // namespace horizon
