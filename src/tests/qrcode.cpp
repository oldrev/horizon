#include "catch2/catch_amalgamated.hpp"
#include "common/qrcode.hpp"
#include "nlohmann/json.hpp"

using namespace horizon;

// A QR symbol has to be a valid symbol, otherwise scanners won't read it. The
// structural features checked here (finder patterns, timing pattern and the
// dark module) would all be misplaced if the row/column order was flipped, so
// they catch the most likely mistake when mapping the encoder's output to our
// bottom-up coordinate system.
namespace {

// Returns the module at the given position counted from the top left corner of
// the symbol, i.e. without the quiet zone.
bool module_from_top(const QRCode &qr, unsigned int col, unsigned int row_from_top)
{
    const auto n = qr.get_matrix_size();
    const auto border = qr.border_modules;
    return qr.get_module(border + col, border + n - 1 - row_from_top);
}

// The 7x7 finder pattern, dark modules are the border and the 3x3 centre.
bool finder_pattern_expected(int dx, int dy)
{
    if (dx == 0 || dx == 6 || dy == 0 || dy == 6)
        return true; // outer border
    if (dx == 1 || dx == 5 || dy == 1 || dy == 5)
        return false; // light ring
    return true;      // centre
}

void check_finder_pattern(const QRCode &qr, unsigned int col_offset, unsigned int row_offset)
{
    for (int dy = 0; dy < 7; dy++) {
        for (int dx = 0; dx < 7; dx++) {
            INFO("dx: " << dx << " dy: " << dy);
            CHECK(module_from_top(qr, col_offset + dx, row_offset + dy) == finder_pattern_expected(dx, dy));
        }
    }
}

} // namespace

TEST_CASE("QR code structural features")
{
    QRCode qr(UUID::random());
    qr.text = "PCB_1_2026-09-21";
    qr.ecc = QRCode::ECC::MEDIUM;

    const auto n = qr.get_matrix_size();
    REQUIRE(qr.get_error().empty());
    REQUIRE(n >= 21);
    REQUIRE((n - 21) % 4 == 0); // valid QR versions are 21, 25, 29, ...

    SECTION("finder patterns")
    {
        check_finder_pattern(qr, 0, 0);     // top left
        check_finder_pattern(qr, n - 7, 0); // top right
        check_finder_pattern(qr, 0, n - 7); // bottom left
    }

    SECTION("timing patterns")
    {
        for (unsigned int x = 8; x <= n - 9; x++) {
            INFO("x: " << x);
            CHECK(module_from_top(qr, x, 6) == (x % 2 == 0));
            CHECK(module_from_top(qr, 6, x) == (x % 2 == 0));
        }
    }

    SECTION("dark module")
    {
        // the dark module always sits at (8, 4 * version + 9), counted from the top
        const auto version = (n - 17) / 4;
        CHECK(module_from_top(qr, 8, (4 * version) + 9));
    }
}

TEST_CASE("QR code quiet zone and size")
{
    QRCode qr(UUID::random());
    qr.text = "HELLO";
    qr.module_size = 500000; // 0.5mm
    qr.border_modules = 4;

    const auto n = qr.get_matrix_size();
    REQUIRE(n > 0);
    CHECK(qr.get_total_modules() == n + 8);
    CHECK(qr.get_total_size() == static_cast<uint64_t>(n + 8) * 500000);

    // the bounding box is centered on the placement
    const auto bbox = qr.get_bbox();
    CHECK(bbox.first.x == -bbox.second.x);
    CHECK(bbox.first.y == -bbox.second.y);
    CHECK(bbox.second.x - bbox.first.x == static_cast<int64_t>(qr.get_total_size()));

    SECTION("nothing is drawn in the quiet zone")
    {
        for (unsigned int i = 0; i < qr.get_total_modules(); i++) {
            CHECK_FALSE(qr.get_module(i, 0));
            CHECK_FALSE(qr.get_module(0, i));
            CHECK_FALSE(qr.get_module(i, qr.get_total_modules() - 1));
            CHECK_FALSE(qr.get_module(qr.get_total_modules() - 1, i));
        }
    }

    SECTION("a different quiet zone changes the size, but not the symbol")
    {
        qr.border_modules = 0;
        CHECK(qr.get_matrix_size() == n);
        CHECK(qr.get_total_modules() == n);
    }

    SECTION("an absurd quiet zone is clamped")
    {
        qr.border_modules = 100000;
        CHECK(qr.get_total_modules() == n + (2 * QRCode::max_border_modules));
    }

    SECTION("no size means nothing to draw")
    {
        qr.module_size = 0;
        CHECK(qr.get_total_size() == 0);
        CHECK(qr.get_bbox().first == Coordi(0, 0));
        CHECK(qr.get_bbox().second == Coordi(0, 0));
    }
}

TEST_CASE("QR code error correction levels")
{
    const std::string text = "PCB_1_2026-09-21";
    QRCode low(UUID::random());
    low.text = text;
    low.ecc = QRCode::ECC::LOW;

    QRCode high(UUID::random());
    high.text = text;
    high.ecc = QRCode::ECC::HIGH;

    REQUIRE(low.get_error().empty());
    REQUIRE(high.get_error().empty());
    // more redundancy needs more space for the same data
    CHECK(high.get_matrix_size() >= low.get_matrix_size());
}

TEST_CASE("QR code inverted draws the other modules")
{
    QRCode plain(UUID::random());
    plain.text = "HELLO";
    plain.border_modules = 4;

    QRCode qr(UUID::random());
    qr.text = "HELLO";
    qr.border_modules = 4;
    qr.inverted = true;

    const auto n = qr.get_total_modules();
    REQUIRE(qr.get_matrix_size() > 0);
    CHECK(qr.get_matrix_size() == plain.get_matrix_size());

    for (unsigned int y = 0; y < n; y++) {
        for (unsigned int x = 0; x < n; x++) {
            REQUIRE(qr.get_module(x, y) != plain.get_module(x, y));
        }
    }

    SECTION("the quiet zone is drawn as well, it has to look like the light modules")
    {
        for (unsigned int i = 0; i < n; i++) {
            CHECK(qr.get_module(i, 0));
            CHECK(qr.get_module(0, i));
            CHECK(qr.get_module(i, n - 1));
            CHECK(qr.get_module(n - 1, i));
        }
    }
}

// Rebuilds the module grid from the rectangles that actually get drawn and
// compares it against what the scanner is supposed to see.
namespace {

std::vector<bool> covered_modules(const std::vector<std::pair<Coordi, Coordi>> &rects, unsigned int n,
                                  int64_t module_size)
{
    const auto half = (static_cast<int64_t>(n) * module_size) / 2;
    std::vector<bool> covered(n * n, false);
    for (const auto &[from, to] : rects) {
        REQUIRE(from.x < to.x);
        REQUIRE(from.y < to.y);
        // rectangles have to be aligned to the module grid
        REQUIRE((from.x + half) % module_size == 0);
        REQUIRE((from.y + half) % module_size == 0);
        REQUIRE((to.x + half) % module_size == 0);
        REQUIRE((to.y + half) % module_size == 0);
        for (auto y = (from.y + half) / module_size; y < (to.y + half) / module_size; y++) {
            for (auto x = (from.x + half) / module_size; x < (to.x + half) / module_size; x++) {
                REQUIRE_FALSE(covered[(y * n) + x]);
                covered[(y * n) + x] = true;
            }
        }
    }
    return covered;
}

} // namespace

TEST_CASE("QR code drawn rectangles match the modules")
{
    QRCode qr(UUID::random());
    qr.text = "PCB_1_2026-09-21";
    qr.module_size = 1000000; // 1mm, so that one module is one grid step
    qr.border_modules = 4;

    const auto n = qr.get_total_modules();
    const auto ms = static_cast<int64_t>(qr.module_size);
    REQUIRE(n > 0);
    REQUIRE((static_cast<int64_t>(n) * ms) % 2 == 0); // otherwise the grid math below is off

    SECTION("nothing is drawn outside the symbol and the quiet zone")
    {
        const auto bbox = qr.get_bbox();
        for (const auto &[from, to] : qr.get_drawn_rects(false)) {
            CHECK(from.x >= bbox.first.x);
            CHECK(from.y >= bbox.first.y);
            CHECK(to.x <= bbox.second.x);
            CHECK(to.y <= bbox.second.y);
        }
    }

    SECTION("the rectangles cover exactly the drawn modules")
    {
        const auto covered = covered_modules(qr.get_drawn_rects(false), n, ms);
        for (unsigned int y = 0; y < n; y++) {
            for (unsigned int x = 0; x < n; x++) {
                INFO("x: " << x << " y: " << y);
                CHECK(covered[(y * n) + x] == qr.get_module(x, y));
            }
        }
    }

    SECTION("mirroring flips the modules horizontally")
    {
        const auto covered = covered_modules(qr.get_drawn_rects(true), n, ms);
        for (unsigned int y = 0; y < n; y++) {
            for (unsigned int x = 0; x < n; x++) {
                INFO("x: " << x << " y: " << y);
                // what is drawn at x in the mirrored symbol is the module from n-1-x
                CHECK(covered[(y * n) + x] == qr.get_module(n - 1 - x, y));
            }
        }
    }

    SECTION("a reversed symbol is not the same as an unreversed one")
    {
        // guards against the mirroring silently doing nothing
        CHECK(qr.get_drawn_rects(true) != qr.get_drawn_rects(false));
    }

    SECTION("nothing is drawn without a size")
    {
        qr.module_size = 0;
        CHECK(qr.get_drawn_rects(false).empty());
    }
}

TEST_CASE("QR code empty and oversized text")
{
    QRCode empty(UUID::random());
    CHECK(empty.get_matrix_size() == 0);
    CHECK(empty.get_error().empty());
    // nothing may be drawn for an empty symbol, not even the quiet zone
    for (unsigned int i = 0; i < empty.get_total_modules(); i++) {
        CHECK_FALSE(empty.get_module(i, 0));
    }

    QRCode too_long(UUID::random());
    too_long.text = std::string(3000, 'X');
    too_long.ecc = QRCode::ECC::HIGH;
    // has to report an error instead of throwing or crashing
    CHECK(too_long.get_matrix_size() == 0);
    CHECK(!too_long.get_error().empty());
    CHECK_FALSE(too_long.get_module(0, 0));
}

TEST_CASE("QR code serialization round trip")
{
    QRCode qr(UUID::random());
    qr.text = "PCB_${rev}_${date}";
    qr.layer = -120;
    qr.module_size = 750000;
    qr.border_modules = 2;
    qr.inverted = true;
    qr.ecc = QRCode::ECC::QUARTILE;
    qr.placement.shift = {1000000, -2000000};
    qr.placement.set_angle_deg(90);

    const auto j = qr.serialize();
    QRCode restored(qr.uuid, j);

    CHECK(restored.text == qr.text);
    CHECK(restored.layer == qr.layer);
    CHECK(restored.module_size == qr.module_size);
    CHECK(restored.border_modules == qr.border_modules);
    CHECK(restored.inverted == qr.inverted);
    CHECK(restored.ecc == qr.ecc);
    CHECK(restored.placement.shift == qr.placement.shift);
    CHECK(restored.placement.get_angle() == qr.placement.get_angle());
    CHECK(restored.get_matrix_size() == qr.get_matrix_size());
}

TEST_CASE("QR code uses the expanded text override")
{
    QRCode qr(UUID::random());
    qr.text = "PCB_${rev}_${date}";
    qr.text_override = "PCB_1_2026-09-21";
    qr.overridden = true;

    CHECK(qr.get_text() == "PCB_1_2026-09-21");

    QRCode plain(UUID::random());
    plain.text = "PCB_1_2026-09-21";
    plain.ecc = qr.ecc;
    CHECK(qr.get_matrix_size() == plain.get_matrix_size());
}
