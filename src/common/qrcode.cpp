#include "qrcode.hpp"
#include "lut.hpp"
#include "nlohmann/json.hpp"
#include "qrcodegen/qrcodegen.hpp"
#include <algorithm>
#include <stdexcept>

namespace horizon {

const LutEnumStr<QRCode::ECC> qrcode_ecc_lut = {
        {"low", QRCode::ECC::LOW},
        {"medium", QRCode::ECC::MEDIUM},
        {"quartile", QRCode::ECC::QUARTILE},
        {"high", QRCode::ECC::HIGH},
};

static qrcodegen::QrCode::Ecc ecc_to_qrcodegen(QRCode::ECC ecc)
{
    switch (ecc) {
    case QRCode::ECC::LOW:
        return qrcodegen::QrCode::Ecc::LOW;
    case QRCode::ECC::QUARTILE:
        return qrcodegen::QrCode::Ecc::QUARTILE;
    case QRCode::ECC::HIGH:
        return qrcodegen::QrCode::Ecc::HIGH;
    case QRCode::ECC::MEDIUM:
    default:
        return qrcodegen::QrCode::Ecc::MEDIUM;
    }
}

QRCode::QRCode(const UUID &uu, const json &j)
    : uuid(uu), placement(j.at("placement")), layer(j.value("layer", 0)), text(j.at("text").get<std::string>()),
      module_size(j.value("module_size", 500000)), border_modules(j.value("border_modules", 4)),
      inverted(j.value("inverted", false)), ecc(qrcode_ecc_lut.lookup(j.value("ecc", ""), ECC::MEDIUM))
{
}

QRCode::QRCode(const UUID &uu) : uuid(uu)
{
}

const std::string &QRCode::get_text() const
{
    return overridden ? text_override : text;
}

void QRCode::update() const
{
    if (cache.valid && cache.text == get_text() && cache.ecc == ecc)
        return;

    cache.valid = true;
    cache.text = get_text();
    cache.ecc = ecc;
    cache.matrix_size = 0;
    cache.modules.clear();
    cache.error.clear();

    if (cache.text.empty())
        return;

    try {
        const auto code = qrcodegen::QrCode::encodeText(cache.text.c_str(), ecc_to_qrcodegen(ecc));
        const auto n = static_cast<unsigned int>(code.getSize());
        cache.matrix_size = n;
        cache.modules.resize(n * n);
        for (unsigned int y = 0; y < n; y++) {
            for (unsigned int x = 0; x < n; x++) {
                // qrcodegen has row 0 at the top, we keep row 0 at the bottom
                cache.modules[(y * n) + x] = code.getModule(static_cast<int>(x), static_cast<int>(n - 1 - y));
            }
        }
    }
    catch (const std::exception &e) {
        cache.error = e.what();
    }
}

unsigned int QRCode::get_matrix_size() const
{
    update();
    return cache.matrix_size;
}

unsigned int QRCode::get_border() const
{
    return std::min(border_modules, max_border_modules);
}

unsigned int QRCode::get_total_modules() const
{
    update();
    return cache.matrix_size + (2 * get_border());
}

uint64_t QRCode::get_total_size() const
{
    return static_cast<uint64_t>(get_total_modules()) * module_size;
}

std::pair<Coordi, Coordi> QRCode::get_bbox() const
{
    const auto s = static_cast<int64_t>(get_total_size() / 2);
    return {{-s, -s}, {s, s}};
}

bool QRCode::get_module(unsigned int x, unsigned int y) const
{
    update();
    const auto n = cache.matrix_size;
    if (n == 0)
        return false;
    const auto border = get_border();
    // the quiet zone looks like the light modules
    if (x < border || y < border || x >= n + border || y >= n + border)
        return inverted;
    return cache.modules[((y - border) * n) + (x - border)] != inverted;
}

std::vector<std::pair<Coordi, Coordi>> QRCode::get_drawn_rects(bool reverse) const
{
    std::vector<std::pair<Coordi, Coordi>> rects;
    if (module_size == 0)
        return rects;

    // the quiet zone is part of the footprint, so iterate over all of it
    const auto n = get_total_modules();
    const auto ms = static_cast<int64_t>(module_size);
    const auto half = (static_cast<int64_t>(n) * ms) / 2;

    // local y coordinate of the bottom edge of module row y
    const auto module_y = [&](unsigned int y) { return -half + (static_cast<int64_t>(y) * ms); };

    for (unsigned int y = 0; y < n; y++) {
        unsigned int x = 0;
        while (x < n) {
            if (!get_module(x, y)) {
                x++;
                continue;
            }
            // merge horizontally adjacent modules into a single rectangle
            unsigned int x1 = x + 1;
            while (x1 < n && get_module(x1, y)) {
                x1++;
            }

            const auto xa = -half + (static_cast<int64_t>(x) * ms);
            const auto xb = -half + (static_cast<int64_t>(x1) * ms);
            const auto ya = module_y(y);
            if (reverse) // mirror horizontally
                rects.emplace_back(Coordi(-xb, ya), Coordi(-xa, ya + ms));
            else
                rects.emplace_back(Coordi(xa, ya), Coordi(xb, ya + ms));
            x = x1;
        }
    }
    return rects;
}

const std::string &QRCode::get_error() const
{
    update();
    return cache.error;
}

UUID QRCode::get_uuid() const
{
    return uuid;
}

json QRCode::serialize() const
{
    json j;
    j["placement"] = placement.serialize();
    j["layer"] = layer;
    j["text"] = text;
    j["module_size"] = module_size;
    j["border_modules"] = border_modules;
    j["inverted"] = inverted;
    j["ecc"] = qrcode_ecc_lut.lookup_reverse(ecc);
    return j;
}
} // namespace horizon
