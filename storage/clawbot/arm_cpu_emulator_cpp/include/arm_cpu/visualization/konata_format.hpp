#pragma once

/// @file konata_format.hpp
/// @brief Konata-compatible data format for pipeline visualization.
///
/// Defines data structures compatible with the Konata pipeline
/// visualization tool format, enabling detailed stage-by-stage visualization.

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace arm_cpu {

// =====================================================================
// StageId - Pipeline stage identifiers
// =====================================================================
enum class StageId : uint8_t {
    IF,  // Instruction Fetch
    DE,  // Decode
    RN,  // Rename
    DI,  // Dispatch
    IS,  // Issue
    EX,  // Execute
    ME,  // Memory
    WB,  // Writeback
    RR,  // Retire
};

inline const char* stage_id_name(StageId s) {
    switch (s) {
        case StageId::IF: return "IF";
        case StageId::DE: return "DE";
        case StageId::RN: return "RN";
        case StageId::DI: return "DI";
        case StageId::IS: return "IS";
        case StageId::EX: return "EX";
        case StageId::ME: return "ME";
        case StageId::WB: return "WB";
        case StageId::RR: return "RR";
    }
    return "?";
}

inline const char* stage_id_full_name(StageId s) {
    switch (s) {
        case StageId::IF: return "Instruction Fetch";
        case StageId::DE: return "Decode";
        case StageId::RN: return "Rename";
        case StageId::DI: return "Dispatch";
        case StageId::IS: return "Issue";
        case StageId::EX: return "Execute";
        case StageId::ME: return "Memory";
        case StageId::WB: return "Writeback";
        case StageId::RR: return "Retire";
    }
    return "Unknown";
}

// =====================================================================
// KonataStage - Single pipeline stage duration
// =====================================================================
struct KonataStage {
    std::string name;
    uint64_t start_cycle = 0;
    uint64_t end_cycle = 0;

    static KonataStage create(const std::string& n, uint64_t s, uint64_t e) {
        return KonataStage{n, s, e};
    }

    uint64_t duration() const {
        return end_cycle >= start_cycle ? end_cycle - start_cycle : 0;
    }
};

// =====================================================================
// KonataLane - Resource/execution unit lane
// =====================================================================
struct KonataLane {
    std::string name;
    std::vector<KonataStage> stages;

    KonataLane() = default;
    explicit KonataLane(std::string n) : name(std::move(n)) {}

    void add_stage(KonataStage stage) {
        stages.push_back(std::move(stage));
    }
};

// =====================================================================
// KonataDependencyType
// =====================================================================
enum class KonataDependencyType : uint8_t {
    Register,
    Memory,
};

inline const char* konata_dep_type_color(KonataDependencyType t) {
    switch (t) {
        case KonataDependencyType::Register: return "#ff6600";
        case KonataDependencyType::Memory:   return "#0066ff";
    }
    return "#ffffff";
}

// =====================================================================
// KonataDependencyRef
// =====================================================================
struct KonataDependencyRef {
    uint64_t producer_id = 0;
    KonataDependencyType dep_type = KonataDependencyType::Register;
};

// =====================================================================
// KonataOp - Instruction operation for visualization
// =====================================================================
struct KonataOp {
    uint64_t id = 0;
    uint64_t gid = 0;
    uint64_t rid = 0;
    uint64_t fetched_cycle = 0;
    std::optional<uint64_t> retired_cycle;
    std::string label_name;
    uint64_t pc = 0;
    std::unordered_map<std::string, KonataLane> lanes;
    std::vector<KonataDependencyRef> prods;
    std::vector<uint16_t> src_regs;
    std::vector<uint16_t> dst_regs;
    bool is_memory = false;
    std::optional<uint64_t> mem_addr;

    KonataOp() = default;
    KonataOp(uint64_t id_, uint64_t gid_, uint64_t pc_, std::string label)
        : id(id_), gid(gid_), pc(pc_), label_name(std::move(label)) {}

    void add_stage(StageId stage_id, uint64_t start_cycle, uint64_t end_cycle) {
        auto& lane = lanes["main"];
        lane.name = "main";
        lane.add_stage(KonataStage::create(stage_id_name(stage_id), start_cycle, end_cycle));
    }

    void add_stage_with_name(const std::string& name, uint64_t start_cycle, uint64_t end_cycle) {
        auto& lane = lanes["main"];
        lane.name = "main";
        lane.add_stage(KonataStage::create(name, start_cycle, end_cycle));
    }

    void add_dependency(uint64_t producer_id, KonataDependencyType dep_type) {
        prods.push_back(KonataDependencyRef{producer_id, dep_type});
    }

    std::optional<uint64_t> total_latency() const {
        if (!retired_cycle.has_value()) return std::nullopt;
        return *retired_cycle >= fetched_cycle ? *retired_cycle - fetched_cycle : 0;
    }
};

// =====================================================================
// KonataConfigInfo
// =====================================================================
struct KonataConfigInfo {
    std::size_t window_size = 0;
    std::size_t issue_width = 0;
    std::size_t commit_width = 0;
};

// =====================================================================
// KonataMetadata
// =====================================================================
struct KonataMetadata {
    std::optional<KonataConfigInfo> config;
    std::optional<uint64_t> timestamp;
};

// =====================================================================
// KonataSnapshot
// =====================================================================
struct KonataSnapshot {
    std::vector<KonataOp> ops;
    uint64_t cycle = 0;
    uint64_t committed_count = 0;
    KonataMetadata metadata;

    static KonataSnapshot create(uint64_t c, uint64_t cc) {
        return KonataSnapshot{{}, c, cc, KonataMetadata{}};
    }

    void add_op(KonataOp op) { ops.push_back(std::move(op)); }
    std::size_t size() const { return ops.size(); }
    bool empty() const { return ops.empty(); }
};

// =====================================================================
// CacheAccessInfoViz - Cache access information for visualization
// =====================================================================
struct CacheAccessInfoViz {
    uint64_t start_cycle = 0;
    uint64_t end_cycle = 0;
    std::string level_name;
    uint64_t latency = 0;
};

} // namespace arm_cpu
