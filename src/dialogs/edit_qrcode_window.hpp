#pragma once
#include <gtkmm.h>
#include "tool_window.hpp"

namespace horizon {
class EditQRCodeWindow : public ToolWindow {
public:
    EditQRCodeWindow(Gtk::Window *parent, ImpInterface *intf, class QRCode &qr, bool use_ok);
    void focus_text();
    void focus_module_size();

private:
    QRCode &qr;

    class TextEditor *editor = nullptr;
    class SpinButtonDim *sp_module_size = nullptr;
    Gtk::ComboBoxText *combo_ecc = nullptr;
    Gtk::SpinButton *sp_border = nullptr;
    Gtk::CheckButton *cb_inverted = nullptr;
    Gtk::Label *la_info = nullptr;

    void update_info();
};
} // namespace horizon
