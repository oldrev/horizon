#pragma once
#include "canvas/selectables.hpp"
#include "canvas/object_ref.hpp"
#include "router/pns_router.h"
#include "util/uuid.hpp"

namespace horizon {
class Board;
class BoardPackage;
class BoardHole;
class Padstack;
class Placement;
class Pad;
class Track;
class Via;
class CanvasGL;
class BoardJunction;
class Junction;
class Net;
class BoardRules;
class Polygon;
class IPool;
class Keepout;
class KeepoutContour;
template <typename T> class Coord;
} // namespace horizon

namespace PNS {
class PNS_HORIZON_PARENT_ITEM : public BOARD_ITEM {
public:
    PNS_HORIZON_PARENT_ITEM()
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::Track *tr) : track(tr)
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::Via *v) : via(v)
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::BoardHole *h) : hole(h)
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::BoardPackage *pkg, const horizon::Pad *p) : package(pkg), pad(p)
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::Keepout *k) : keepout(k)
    {
    }
    PNS_HORIZON_PARENT_ITEM(const horizon::Keepout *k, const horizon::BoardPackage *pkg) : package(pkg), keepout(k)
    {
    }
    bool operator==(const PNS_HORIZON_PARENT_ITEM &other) const
    {
        return track == other.track && via == other.via && package == other.package && pad == other.pad
               && hole == other.hole && keepout == other.keepout;
    }

    KICAD_T Type() const override
    {
        if (via)
            return PCB_VIA_T;
        if (pad)
            return PCB_PAD_T;
        if (keepout)
            return PCB_ZONE_T;
        return PCB_TRACE_T;
    }

    const horizon::Track *track = nullptr;
    const horizon::Via *via = nullptr;
    const horizon::BoardPackage *package = nullptr;
    const horizon::Pad *pad = nullptr;
    const horizon::BoardHole *hole = nullptr;
    const horizon::Keepout *keepout = nullptr;
};

class PNS_HORIZON_IFACE : public PNS::ROUTER_IFACE {
public:
    PNS_HORIZON_IFACE();
    ~PNS_HORIZON_IFACE();

    void SetRouter(PNS::ROUTER *aRouter);
    void SetBoard(horizon::Board *brd);
    void SetCanvas(class horizon::CanvasGL *ca);
    void SetRules(const horizon::BoardRules *rules);
    void SetPool(horizon::IPool *pool);

    void SyncWorld(PNS::NODE *aWorld) override;
    void EraseView() override;
    void HideItem(PNS::ITEM *aItem) override;
    void DisplayItem(const PNS::ITEM *aItem, int aClearance = 0, bool aEdit = false, int aFlags = 0) override;
    void DisplayPathLine(const SHAPE_LINE_CHAIN &, int) override {}
    void AddItem(PNS::ITEM *aItem) override;
    void RemoveItem(PNS::ITEM *aItem) override;
    void Commit() override;

    void UpdateItem(ITEM *aItem) override;
    bool IsFlashedOnLayer(const PNS::ITEM *aItem, int aLayer) const override;
    bool ImportSizes(SIZES_SETTINGS &aSizes, ITEM *aStartItem, NET_HANDLE aNet, VECTOR2D aStartPosition) override;
    int StackupHeight(int aFirstLayer, int aSecondLayer) const override;
    void DisplayRatline(const SHAPE_LINE_CHAIN &aRatline, NET_HANDLE aNet) override;

    PNS::NODE *GetWorld() const override
    {
        return m_world;
    }

    bool IsAnyLayerVisible(const PNS_LAYER_RANGE &aLayer) const override;
    bool IsItemVisible(const PNS::ITEM *aItem) const override;
    bool IsFlashedOnLayer(const PNS::ITEM *aItem, const PNS_LAYER_RANGE &aLayer) const override;
    bool IsPNSCopperLayer(int aLayer) const override;

    void UpdateNet(NET_HANDLE aNet) override;
    int GetNetCode(NET_HANDLE aNet) const override;
    wxString GetNetName(NET_HANDLE aNet) const override;
    NET_HANDLE GetOrphanedNetHandle() override { return nullptr; }

    PNS::RULE_RESOLVER *GetRuleResolver() override;
    PNS::DEBUG_DECORATOR *GetDebugDecorator() override;

    static int layer_to_router(int l);
    static int layer_from_router(int l);
    horizon::Net *get_net_for_code(NET_HANDLE code) const;
    NET_HANDLE get_net_code(const horizon::UUID &uu);

    horizon::UUID get_via_definition_for_code(int code);
    int get_via_definition_code(const horizon::UUID &uu);

    PNS_HORIZON_PARENT_ITEM *get_parent(const horizon::Track *track);
    PNS_HORIZON_PARENT_ITEM *get_parent(const horizon::Via *via);
    PNS_HORIZON_PARENT_ITEM *get_parent(const horizon::BoardHole *hole);
    PNS_HORIZON_PARENT_ITEM *get_parent(const horizon::BoardPackage *pkg, const horizon::Pad *pad);
    PNS_HORIZON_PARENT_ITEM *get_parent(const horizon::Keepout *keepout,
                                              const horizon::BoardPackage *pkg = nullptr);

    long long CalculateRoutedPathLength(const ITEM_SET &, const SOLID *, const SOLID *, const NETCLASS *) override { return 0; }
    int64_t CalculateRoutedPathDelay(const ITEM_SET &, const SOLID *, const SOLID *, const NETCLASS *) override { return 0; }
    int64_t CalculateLengthForDelay(int64_t value, int, bool, int, int, const NETCLASS *) override { return value; }
    int64_t CalculateDelayForShapeLineChain(const SHAPE_LINE_CHAIN &, int, bool, int, int, const NETCLASS *) override { return 0; }
    PCB_LAYER_ID GetBoardLayerFromPNSLayer(int layer) const override { return static_cast<PCB_LAYER_ID>(layer_from_router(layer)); }
    int GetPNSLayerFromBoardLayer(PCB_LAYER_ID layer) const override { return layer_to_router(static_cast<int>(layer)); }

    int64_t get_override_routing_offset() const
    {
        return override_routing_offset;
    }

    void set_override_routing_offset(int64_t o)
    {
        override_routing_offset = o;
    }

private:
    PNS_HORIZON_PARENT_ITEM *get_or_create_parent(const PNS_HORIZON_PARENT_ITEM &it);

    class PNS_HORIZON_RULE_RESOLVER *m_ruleResolver = nullptr;
    std::set<horizon::ObjectRef> m_preview_items;

    horizon::Board *board = nullptr;
    class horizon::CanvasGL *canvas = nullptr;
    const class horizon::BoardRules *rules = nullptr;
    class horizon::IPool *pool = nullptr;
    PNS::NODE *m_world = nullptr;
    PNS::ROUTER *m_router = nullptr;

    std::unique_ptr<PNS::SOLID> syncPad(const horizon::BoardPackage *pkg, const horizon::Pad *pad);
    std::unique_ptr<PNS::SOLID> syncPadstack(const horizon::Padstack *padstack, const horizon::Placement &tr);
    std::unique_ptr<PNS::SOLID> syncHole(const horizon::BoardHole *hole);
    std::unique_ptr<PNS::SEGMENT> syncTrack(const horizon::Track *track);
    std::unique_ptr<PNS::ARC> syncTrackArc(const horizon::Track *track);
    std::unique_ptr<PNS::VIA> syncVia(const horizon::Via *via);
    void syncOutline(const horizon::Polygon *poly, PNS::NODE *aWorld);
    void syncKeepout(const horizon::KeepoutContour *keepout_contour, PNS::NODE *aWorld);
    std::map<horizon::UUID, int> net_code_map;
    std::vector<horizon::Net *> net_code_map_r;

    std::map<horizon::UUID, int> via_definition_code_map;
    std::vector<horizon::UUID> via_definition_code_map_r;

    int64_t override_routing_offset = -1;

    std::list<PNS_HORIZON_PARENT_ITEM> parents;

    std::pair<horizon::BoardPackage *, horizon::Pad *> find_pad(int layer, const horizon::Coord<int64_t> &c);
    horizon::BoardJunction *find_junction(int layer, const horizon::Coord<int64_t> &c);
    std::set<horizon::BoardJunction *> find_junctions(const horizon::Via &via);
    std::set<horizon::BoardJunction *> junctions_maybe_erased;
};
} // namespace PNS
