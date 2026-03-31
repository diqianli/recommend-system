/// @file pipeline_tracker_viz.cpp
/// @brief Pipeline tracker visualization implementation.

#include "arm_cpu/visualization/pipeline_tracker_viz.hpp"
#include "arm_cpu/ooo/ooo_engine.hpp"

#include <algorithm>
#include <format>
#include <cstring>

namespace arm_cpu {

PipelineTrackerViz::PipelineTrackerViz(std::size_t fetch_width)
    : fetch_width_(fetch_width)
{}

void PipelineTrackerViz::record_fetch(const Instruction& instr, uint64_t cycle) {
    auto id = instr.id;
    get_or_assign_viz_id(id);

    if (fetch_count_in_cycle_ >= fetch_width_) {
        ++current_fetch_cycle_;
        fetch_count_in_cycle_ = 0;
    } else if (fetch_count_in_cycle_ == 0) {
        current_fetch_cycle_ = 0;
    }

    uint64_t adjusted_cycle = std::max(current_fetch_cycle_, cycle);
    ++fetch_count_in_cycle_;

    auto& timing = timings_[id];
    timing.fetch_start = adjusted_cycle;
    timing.fetch_end = adjusted_cycle + 1;

    bool already_ordered = false;
    for (const auto& existing : order_) {
        if (existing == id) { already_ordered = true; break; }
    }
    if (!already_ordered) {
        order_.push_back(id);
    }

    if (instr.disasm.has_value()) {
        disasm_map_[id] = *instr.disasm;
    }
    if (!instr.src_regs.empty()) {
        std::vector<uint16_t> regs;
        for (const auto& r : instr.src_regs) regs.push_back(r.value);
        src_regs_map_[id] = std::move(regs);
    }
    if (!instr.dst_regs.empty()) {
        std::vector<uint16_t> regs;
        for (const auto& r : instr.dst_regs) regs.push_back(r.value);
        dst_regs_map_[id] = std::move(regs);
    }
    if (instr.mem_access.has_value()) {
        mem_access_map_[id] = {instr.mem_access->addr, instr.mem_access->size, instr.mem_access->is_load};
    }
}

void PipelineTrackerViz::record_decode(InstructionId id, uint64_t start_cycle, uint64_t end_cycle) {
    auto& timing = timings_[id];
    timing.decode_start = start_cycle;
    timing.decode_end = end_cycle;
}

void PipelineTrackerViz::record_rename(InstructionId id, uint64_t start_cycle, uint64_t end_cycle) {
    auto& timing = timings_[id];
    timing.rename_start = start_cycle;
    timing.rename_end = end_cycle;
}

void PipelineTrackerViz::record_dispatch(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];

    uint64_t fetch_end = timing.fetch_end.value_or(cycle);
    uint64_t decode_start = timing.fetch_start.value_or(fetch_end);
    uint64_t decode_end = cycle <= fetch_end
        ? std::max(cycle, decode_start)
        : std::min(fetch_end + 1, cycle);

    if (!timing.decode_start.has_value()) {
        timing.decode_start = decode_start;
        timing.decode_end = decode_end;
    }

    uint64_t rename_start = timing.decode_end.value_or(decode_end);
    uint64_t rename_end = cycle <= rename_start
        ? std::max(cycle, rename_start)
        : std::min(rename_start + 1, cycle);

    if (!timing.rename_start.has_value()) {
        timing.rename_start = rename_start;
        timing.rename_end = rename_end;
    }

    uint64_t dispatch_start = timing.rename_end.value_or(rename_end);
    uint64_t dispatch_end = std::max(cycle, dispatch_start);
    timing.dispatch_start = dispatch_start;
    timing.dispatch_end = dispatch_end;
}

void PipelineTrackerViz::record_ready(InstructionId id, uint64_t cycle) {
    timings_[id].ready_cycle = cycle;
}

void PipelineTrackerViz::record_issue(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];
    uint64_t issue_start = timing.ready_cycle
        .value_or(timing.dispatch_end.value_or(cycle));
    uint64_t issue_end = std::max(issue_start, cycle);
    timing.issue_start = issue_start;
    timing.issue_end = issue_end;
}

void PipelineTrackerViz::record_execute_start(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];
    uint64_t start = timing.issue_start.value_or(cycle);
    timing.execute_start = start;
}

void PipelineTrackerViz::record_execute_end(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];
    uint64_t exec_start = timing.issue_end.value_or(cycle);
    uint64_t exec_end = std::max(exec_start, cycle);
    timing.execute_start = exec_start;
    timing.execute_end = exec_end;
}

void PipelineTrackerViz::record_memory(InstructionId id, uint64_t start_cycle, uint64_t end_cycle) {
    auto& timing = timings_[id];
    uint64_t mem_start = timing.issue_end.value_or(start_cycle);
    uint64_t mem_end = std::max(mem_start, end_cycle);
    timing.memory_start = mem_start;
    timing.memory_end = mem_end;
}

void PipelineTrackerViz::record_cache_access(InstructionId id, const CacheAccessInfoViz& info) {
    // Store cache info alongside timing (simplified - not storing full info)
    (void)id;
    (void)info;
}

void PipelineTrackerViz::record_complete(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];

    if (timing.memory_start.has_value() && !timing.memory_end.has_value()) {
        timing.memory_end = cycle;
    } else if (timing.execute_start.has_value() && !timing.execute_end.has_value()) {
        timing.execute_end = cycle;
    }

    if (!timing.complete_cycle.has_value()) {
        timing.complete_cycle = cycle;
    }

    bool already_completed = false;
    for (const auto& c : completed_) {
        if (c == id) { already_completed = true; break; }
    }
    if (!already_completed) {
        completed_.push_back(id);
        if (completed_.size() > max_completed_) {
            completed_.pop_front();
        }
    }
}

void PipelineTrackerViz::record_retire(InstructionId id, uint64_t cycle) {
    auto& timing = timings_[id];
    if (!timing.retire_cycle.has_value()) {
        timing.retire_cycle = cycle;
    }
    ++retire_counter_;
}

void PipelineTrackerViz::add_dependency(InstructionId consumer, InstructionId producer, bool is_memory) {
    auto viz_producer_id = get_or_assign_viz_id(producer);
    auto& deps = dependencies_[consumer];

    bool exists = false;
    for (const auto& d : deps) {
        if (d.producer_id == viz_producer_id && d.dep_type == (is_memory ? KonataDependencyType::Memory : KonataDependencyType::Register)) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        deps.push_back(TrackerDependencyInfo{
            viz_producer_id,
            is_memory ? KonataDependencyType::Memory : KonataDependencyType::Register
        });
    }
}

uint64_t PipelineTrackerViz::get_or_assign_viz_id(InstructionId id) {
    auto it = viz_id_map_.find(id);
    if (it != viz_id_map_.end()) return it->second;
    auto vid = next_viz_id_++;
    viz_id_map_[id] = vid;
    return vid;
}

std::vector<KonataOp> PipelineTrackerViz::to_konata_ops(
    const std::vector<const WindowEntry*>& entries, uint64_t current_cycle) const
{
    std::vector<KonataOp> ops;

    for (const auto* entry : entries) {
        auto id = entry->instruction.id;
        auto viz_it = viz_id_map_.find(id);
        if (viz_it == viz_id_map_.end()) continue;
        uint64_t viz_id = viz_it->second;

        auto timing = timings_;
        auto timing_it = timing.find(id);
        if (timing_it == timing.end()) continue;
        auto mut_timing = timing_it->second;

        // For waiting instructions, add in-progress issue stage
        if (!mut_timing.issue_start.has_value() && mut_timing.dispatch_end.has_value()) {
            uint64_t dispatch_end = *mut_timing.dispatch_end;
            mut_timing.issue_start = dispatch_end;
            mut_timing.issue_end = current_cycle;
        }

        // For issued but not completed, add in-progress execute
        if (!mut_timing.execute_start.has_value() && mut_timing.issue_end.has_value() && !mut_timing.complete_cycle.has_value()) {
            uint64_t issue_end = *mut_timing.issue_end;
            mut_timing.execute_start = issue_end;
            mut_timing.execute_end = current_cycle;
        }

        std::string label = entry->instruction.disasm.value_or(
            std::format("Instr {}", id.value));

        KonataOp op(viz_id, id.value, entry->instruction.pc, std::move(label));
        op.fetched_cycle = mut_timing.fetch_start.value_or(0);
        op.retired_cycle = mut_timing.retire_cycle;

        // Add stages
        if (mut_timing.fetch_start && mut_timing.fetch_end)
            op.add_stage(StageId::IF, *mut_timing.fetch_start, *mut_timing.fetch_end);
        if (mut_timing.decode_start && mut_timing.decode_end)
            op.add_stage(StageId::DE, *mut_timing.decode_start, *mut_timing.decode_end);
        if (mut_timing.rename_start && mut_timing.rename_end)
            op.add_stage(StageId::RN, *mut_timing.rename_start, *mut_timing.rename_end);
        if (mut_timing.dispatch_start && mut_timing.dispatch_end)
            op.add_stage(StageId::DI, *mut_timing.dispatch_start, *mut_timing.dispatch_end);
        if (mut_timing.issue_start && mut_timing.issue_end)
            op.add_stage(StageId::IS, *mut_timing.issue_start, *mut_timing.issue_end);
        if (mut_timing.memory_start && mut_timing.memory_end)
            op.add_stage(StageId::ME, *mut_timing.memory_start, *mut_timing.memory_end);
        else if (mut_timing.execute_start && mut_timing.execute_end)
            op.add_stage(StageId::EX, *mut_timing.execute_start, *mut_timing.execute_end);

        // Writeback: from execute/memory end to complete
        uint64_t exec_mem_end = mut_timing.memory_end.value_or(
            mut_timing.execute_end.value_or(0));
        if (mut_timing.complete_cycle.has_value() && exec_mem_end > 0) {
            op.add_stage(StageId::WB, exec_mem_end, *mut_timing.complete_cycle);
        }

        // Retire
        if (mut_timing.retire_cycle.has_value()) {
            uint64_t rr_start = exec_mem_end > 0 ? exec_mem_end : 0;
            op.add_stage(StageId::RR, rr_start, *mut_timing.retire_cycle);
        }

        // Add registers
        for (const auto& r : entry->instruction.src_regs) op.src_regs.push_back(r.value);
        for (const auto& r : entry->instruction.dst_regs) op.dst_regs.push_back(r.value);

        // Add memory info
        if (entry->instruction.mem_access.has_value()) {
            op.is_memory = true;
            op.mem_addr = entry->instruction.mem_access->addr;
        }

        // Add dependencies
        auto dep_it = dependencies_.find(id);
        if (dep_it != dependencies_.end()) {
            for (const auto& dep : dep_it->second) {
                op.prods.push_back(KonataDependencyRef{dep.producer_id, dep.dep_type});
            }
        }

        ops.push_back(std::move(op));
    }

    std::sort(ops.begin(), ops.end(),
              [](const KonataOp& a, const KonataOp& b) { return a.id < b.id; });

    return ops;
}

KonataSnapshot PipelineTrackerViz::to_snapshot(
    const std::vector<const WindowEntry*>& entries,
    uint64_t cycle, uint64_t committed_count) const
{
    auto ops = to_konata_ops(entries, cycle);
    KonataSnapshot snapshot{{}, cycle, committed_count, KonataMetadata{}};
    for (auto& op : ops) {
        snapshot.ops.push_back(std::move(op));
    }
    return snapshot;
}

std::vector<KonataOp> PipelineTrackerViz::export_all_konata_ops() const {
    std::vector<KonataOp> ops;

    for (const auto& [id, viz_id] : viz_id_map_) {
        auto timing_it = timings_.find(id);
        if (timing_it == timings_.end()) continue;
        const auto& timing = timing_it->second;

        std::string disasm = "Instr " + std::to_string(id.value);
        auto disasm_it = disasm_map_.find(id);
        if (disasm_it != disasm_map_.end()) disasm = disasm_it->second;

        KonataOp op(viz_id, id.value, 0, disasm);
        op.fetched_cycle = timing.fetch_start.value_or(0);
        op.retired_cycle = timing.retire_cycle;

        if (timing.fetch_start && timing.fetch_end)
            op.add_stage(StageId::IF, *timing.fetch_start, *timing.fetch_end);
        if (timing.decode_start && timing.decode_end)
            op.add_stage(StageId::DE, *timing.decode_start, *timing.decode_end);
        if (timing.rename_start && timing.rename_end)
            op.add_stage(StageId::RN, *timing.rename_start, *timing.rename_end);
        if (timing.dispatch_start && timing.dispatch_end)
            op.add_stage(StageId::DI, *timing.dispatch_start, *timing.dispatch_end);
        if (timing.issue_start && timing.issue_end)
            op.add_stage(StageId::IS, *timing.issue_start, *timing.issue_end);
        if (timing.memory_start && timing.memory_end)
            op.add_stage(StageId::ME, *timing.memory_start, *timing.memory_end);
        else if (timing.execute_start && timing.execute_end)
            op.add_stage(StageId::EX, *timing.execute_start, *timing.execute_end);

        uint64_t exec_mem_end = timing.memory_end.value_or(timing.execute_end.value_or(0));
        if (timing.complete_cycle && exec_mem_end > 0)
            op.add_stage(StageId::WB, exec_mem_end, *timing.complete_cycle);
        if (timing.retire_cycle)
            op.add_stage(StageId::RR, exec_mem_end > 0 ? exec_mem_end : 0, *timing.retire_cycle);

        auto src_it = src_regs_map_.find(id);
        if (src_it != src_regs_map_.end()) op.src_regs = src_it->second;
        auto dst_it = dst_regs_map_.find(id);
        if (dst_it != dst_regs_map_.end()) op.dst_regs = dst_it->second;

        auto mem_it = mem_access_map_.find(id);
        if (mem_it != mem_access_map_.end()) {
            op.is_memory = true;
            op.mem_addr = std::get<0>(mem_it->second);
        }

        auto dep_it = dependencies_.find(id);
        if (dep_it != dependencies_.end()) {
            for (const auto& dep : dep_it->second) {
                op.prods.push_back(KonataDependencyRef{dep.producer_id, dep.dep_type});
            }
        }

        ops.push_back(std::move(op));
    }

    std::sort(ops.begin(), ops.end(),
              [](const KonataOp& a, const KonataOp& b) { return a.id < b.id; });

    return ops;
}

void PipelineTrackerViz::clear() {
    timings_.clear();
    order_.clear();
    viz_id_map_.clear();
    next_viz_id_ = 0;
    retire_counter_ = 0;
    completed_.clear();
    dependencies_.clear();
    disasm_map_.clear();
    src_regs_map_.clear();
    dst_regs_map_.clear();
    mem_access_map_.clear();
}

const StageTimingViz* PipelineTrackerViz::get_timing(InstructionId id) const {
    auto it = timings_.find(id);
    return it != timings_.end() ? &it->second : nullptr;
}

const std::vector<TrackerDependencyInfo>* PipelineTrackerViz::get_dependencies(InstructionId id) const {
    auto it = dependencies_.find(id);
    return it != dependencies_.end() ? &it->second : nullptr;
}

} // namespace arm_cpu
