#pragma once
#include "common/lut.hpp"
#include "nlohmann/json_fwd.hpp"
#include "util/placement.hpp"
#include "util/uuid.hpp"
#include <string>
#include <vector>

namespace horizon {
using json = nlohmann::json;

/**
 * A QR code placed on a board. Unlike most other board items it isn't tied to
 * a particular kind of layer, it may be put on silkscreen, solder mask, copper
 * or any user layer.
 *
 * The encoded data is taken from a text template which, just like board text,
 * is expanded from the board's project meta data, e.g. "PCB_${rev}_${date}".
 */
class QRCode {
public:
    /**
     * Error correction level. Higher levels make the symbol more resilient to
     * damage, at the cost of a bigger symbol for the same amount of data.
     */
    enum class ECC { LOW, MEDIUM, QUARTILE, HIGH };

    QRCode(const UUID &uu, const json &j);
    QRCode(const UUID &uu);

    UUID uuid;
    Placement placement;
    int layer = 0;

    /** Template, expanded from the project meta data by Board::expand() */
    std::string text;
    /** Result of expanding text, used if overridden is true */
    std::string text_override;
    bool overridden = false;

    /** Edge length of a single module */
    uint64_t module_size = 500000;

    /** Quiet zone surrounding the symbol, in modules */
    unsigned int border_modules = 4;

    /**
     * Upper limit for border_modules. Keeps a mistyped quiet zone from making
     * rendering iterate over an absurd number of modules.
     */
    static constexpr unsigned int max_border_modules = 16;

    /** Draw the light modules instead of the dark ones */
    bool inverted = false;

    ECC ecc = ECC::MEDIUM;

    /** @returns the text that actually gets encoded */
    const std::string &get_text() const;

    /** @returns the number of modules per side, excluding the quiet zone */
    unsigned int get_matrix_size() const;

    /** @returns the number of modules per side, including the quiet zone */
    unsigned int get_total_modules() const;

    /** @returns the edge length of the symbol including the quiet zone */
    uint64_t get_total_size() const;

    /** @returns the bounding box of the symbol, centered on the placement */
    std::pair<Coordi, Coordi> get_bbox() const;

    /**
     * Whether the module at the given position gets drawn on the layer. The
     * quiet zone around the symbol is part of the footprint and is drawn when
     * inverted is set, since it has to look like the light modules.
     * @param x column, 0 is left, less than get_total_modules()
     * @param y row, 0 is bottom, less than get_total_modules()
     */
    bool get_module(unsigned int x, unsigned int y) const;

    /**
     * Everything that has to be drawn, as rectangles in local coordinates
     * relative to the placement. Horizontally adjacent modules are merged into
     * a single rectangle.
     * @param reverse mirror the symbol horizontally, for reversed (i.e. bottom
     * side) layers, so that it reads correctly from that layer's point of view
     */
    std::vector<std::pair<Coordi, Coordi>> get_drawn_rects(bool reverse) const;

    /** @returns a message if the text couldn't be encoded, empty otherwise */
    const std::string &get_error() const;

    UUID get_uuid() const;
    json serialize() const;

private:
    void update() const;

    /** @returns the quiet zone, clamped to max_border_modules */
    unsigned int get_border() const;

    struct Cache {
        unsigned int matrix_size = 0;
        std::vector<bool> modules;
        std::string error;
        std::string text;
        ECC ecc = ECC::MEDIUM;
        bool valid = false;
    };
    mutable Cache cache;
};

extern const LutEnumStr<QRCode::ECC> qrcode_ecc_lut;
} // namespace horizon
