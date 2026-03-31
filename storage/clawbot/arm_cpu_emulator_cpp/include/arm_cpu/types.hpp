#pragma once

/// @file types.hpp
/// @brief Core types for the ARM CPU emulator: Reg, VReg, OpcodeType, Instruction, etc.

#include "arm_cpu/detail/static_vector.hpp"
#include "arm_cpu/error.hpp"

#include <cstdint>
#include <cstring>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace arm_cpu {

// Forward declarations
class Reg;
class VReg;

// =====================================================================
// Reg — ARMv8-A General Purpose Register (X0-X30, SP/XZR=31)
// =====================================================================
class Reg {
public:
    uint8_t value;

    static const Reg SP;
    static const Reg XZR;

    constexpr Reg() : value(0) {}
    constexpr explicit Reg(uint8_t v) : value(v) {}

    bool is_valid() const { return value <= 31; }

    std::string_view name() const {
        static constexpr std::string_view names[31] = {
            "X0", "X1", "X2", "X3", "X4", "X5", "X6", "X7",
            "X8", "X9", "X10", "X11", "X12", "X13", "X14", "X15",
            "X16", "X17", "X18", "X19", "X20", "X21", "X22", "X23",
            "X24", "X25", "X26", "X27", "X28", "X29", "X30",
        };
        if (value == 31) return "SP/XZR";
        return names[value];
    }

    bool operator==(const Reg& o) const { return value == o.value; }
    bool operator!=(const Reg& o) const { return value != o.value; }
    bool operator<(const Reg& o) const { return value < o.value; }

    struct Hash {
        std::size_t operator()(const Reg& r) const noexcept {
            return std::hash<uint8_t>{}(r.value);
        }
    };
};

// =====================================================================
// VReg — ARMv8-A SIMD/FP Register (V0-V31)
// =====================================================================
class VReg {
public:
    uint8_t value;

    static const VReg V0;
    static const VReg V31;

    constexpr VReg() : value(0) {}
    constexpr explicit VReg(uint8_t v) : value(v) {}

    bool is_valid() const { return value <= 31; }

    bool operator==(const VReg& o) const { return value == o.value; }
    bool operator!=(const VReg& o) const { return value != o.value; }
    bool operator<(const VReg& o) const { return value < o.value; }

    struct Hash {
        std::size_t operator()(const VReg& r) const noexcept {
            return std::hash<uint8_t>{}(r.value);
        }
    };
};

// =====================================================================
// OpcodeType — Instruction classification
// =====================================================================
enum class OpcodeType : uint16_t {
    // Computational
    Add, Sub, Mul, Div, And, Orr, Eor, Lsl, Lsr, Asr,
    // Data movement
    Mov, Cmp, Shift,
    // Load/Store
    Load, Store, LoadPair, StorePair,
    // Branch
    Branch, BranchCond, BranchReg,
    // System
    Msr, Mrs, Sys, Nop,
    // SIMD/FP (basic)
    Fadd, Fsub, Fmul, Fdiv,

    // Cache Maintenance
    DcZva, DcCivac, DcCvac, DcCsw,
    IcIvau, IcIallu, IcIalluis,

    // Cryptography
    Aesd, Aese, Aesimc, Aesmc,
    Sha1H, Sha256H, Sha512H,

    // SIMD/Vector (NEON)
    Vadd, Vsub, Vmul, Vmla, Vmls,
    Vld, Vst, Vdup, Vmov,

    // Floating-point FMA
    Fmadd, Fmsub, Fnmadd, Fnmsub, Fcvt,

    // Memory Barriers
    Dmb, Dsb, Isb,

    // Other System
    Eret, Yield, Adr, Pmull,

    // Other
    Other,
};

/// Opcode classification methods (inline for performance)
inline bool is_memory_op(OpcodeType op) {
    switch (op) {
        case OpcodeType::Load: case OpcodeType::Store:
        case OpcodeType::LoadPair: case OpcodeType::StorePair:
        case OpcodeType::Vld: case OpcodeType::Vst:
            return true;
        default: return false;
    }
}

inline bool is_branch(OpcodeType op) {
    switch (op) {
        case OpcodeType::Branch: case OpcodeType::BranchCond:
        case OpcodeType::BranchReg:
            return true;
        default: return false;
    }
}

inline bool is_compute(OpcodeType op) {
    switch (op) {
        case OpcodeType::Add: case OpcodeType::Sub: case OpcodeType::Mul: case OpcodeType::Div:
        case OpcodeType::And: case OpcodeType::Orr: case OpcodeType::Eor:
        case OpcodeType::Lsl: case OpcodeType::Lsr: case OpcodeType::Asr:
        case OpcodeType::Mov: case OpcodeType::Cmp: case OpcodeType::Shift:
        case OpcodeType::Fadd: case OpcodeType::Fsub: case OpcodeType::Fmul: case OpcodeType::Fdiv:
        case OpcodeType::Vadd: case OpcodeType::Vsub: case OpcodeType::Vmul:
        case OpcodeType::Vmla: case OpcodeType::Vmls:
        case OpcodeType::Fmadd: case OpcodeType::Fmsub: case OpcodeType::Fnmadd: case OpcodeType::Fnmsub:
        case OpcodeType::Aesd: case OpcodeType::Aese: case OpcodeType::Aesimc: case OpcodeType::Aesmc:
        case OpcodeType::Sha1H: case OpcodeType::Sha256H: case OpcodeType::Sha512H:
            return true;
        default: return false;
    }
}

inline bool is_cache_maintenance(OpcodeType op) {
    switch (op) {
        case OpcodeType::DcZva: case OpcodeType::DcCivac: case OpcodeType::DcCvac:
        case OpcodeType::DcCsw: case OpcodeType::IcIvau: case OpcodeType::IcIallu:
        case OpcodeType::IcIalluis:
            return true;
        default: return false;
    }
}

inline bool is_crypto(OpcodeType op) {
    switch (op) {
        case OpcodeType::Aesd: case OpcodeType::Aese: case OpcodeType::Aesimc: case OpcodeType::Aesmc:
        case OpcodeType::Sha1H: case OpcodeType::Sha256H: case OpcodeType::Sha512H:
            return true;
        default: return false;
    }
}

inline bool is_simd(OpcodeType op) {
    switch (op) {
        case OpcodeType::Vadd: case OpcodeType::Vsub: case OpcodeType::Vmul:
        case OpcodeType::Vmla: case OpcodeType::Vmls:
        case OpcodeType::Vld: case OpcodeType::Vst: case OpcodeType::Vdup: case OpcodeType::Vmov:
            return true;
        default: return false;
    }
}

inline bool is_fma(OpcodeType op) {
    switch (op) {
        case OpcodeType::Fmadd: case OpcodeType::Fmsub:
        case OpcodeType::Fnmadd: case OpcodeType::Fnmsub:
            return true;
        default: return false;
    }
}

/// Execution latency in cycles (simplified model)
inline uint64_t latency(OpcodeType op) {
    switch (op) {
        case OpcodeType::Mul: case OpcodeType::Div: return 3;
        case OpcodeType::Fadd: case OpcodeType::Fsub: return 2;
        case OpcodeType::Fmul: return 3;
        case OpcodeType::Fdiv: return 8;
        case OpcodeType::DcZva: case OpcodeType::DcCivac: case OpcodeType::DcCvac: return 20;
        case OpcodeType::DcCsw: return 30;
        case OpcodeType::IcIvau: case OpcodeType::IcIallu: case OpcodeType::IcIalluis: return 15;
        case OpcodeType::Aesd: case OpcodeType::Aese: case OpcodeType::Aesimc: case OpcodeType::Aesmc: return 4;
        case OpcodeType::Sha1H: return 10;
        case OpcodeType::Sha256H: return 12;
        case OpcodeType::Sha512H: return 16;
        case OpcodeType::Vadd: case OpcodeType::Vsub: return 2;
        case OpcodeType::Vmul: case OpcodeType::Vmla: case OpcodeType::Vmls: return 4;
        case OpcodeType::Vld: case OpcodeType::Vst: return 3;
        case OpcodeType::Vdup: case OpcodeType::Vmov: return 1;
        case OpcodeType::Fmadd: case OpcodeType::Fmsub:
        case OpcodeType::Fnmadd: case OpcodeType::Fnmsub: return 4;
        default: return 1;
    }
}

// =====================================================================
// ShiftType, ExtensionType
// =====================================================================
enum class ShiftType : uint8_t { Lsl, Lsr, Asr, Ror };

enum class ExtensionType : uint8_t {
    Uxtb, Uxth, Uxtw, Uxtx, Sxtb, Sxth, Sxtw, Sxtx,
};

// =====================================================================
// Operand
// =====================================================================
enum class OperandKind : uint8_t {
    Register, VRegister, Immediate, Memory, ShiftedReg, ExtendedReg,
};

struct ShiftedRegOperand {
    Reg reg;
    ShiftType shift_type;
    uint8_t shift_amount;
};

struct ExtendedRegOperand {
    Reg reg;
    ExtensionType ext;
};

struct MemoryOperand {
    Reg base;
    int64_t offset;
    uint8_t size; // bytes
};

using Operand = std::variant<
    Reg,                    // Register
    VReg,                   // VRegister
    int64_t,                // Immediate
    MemoryOperand,          // Memory
    ShiftedRegOperand,      // ShiftedReg
    ExtendedRegOperand      // ExtendedReg
>;

// =====================================================================
// BranchInfo, MemAccess
// =====================================================================
struct BranchInfo {
    bool is_conditional;
    uint64_t target;
    bool is_taken;
};

struct MemAccess {
    uint64_t addr;
    uint8_t size;
    bool is_load;
};

// =====================================================================
// InstructionId
// =====================================================================
struct InstructionId {
    uint64_t value;

    constexpr InstructionId() : value(0) {}
    constexpr explicit InstructionId(uint64_t v) : value(v) {}

    bool operator==(const InstructionId& o) const { return value == o.value; }
    bool operator!=(const InstructionId& o) const { return value != o.value; }
    bool operator<(const InstructionId& o) const { return value < o.value; }

    struct Hash {
        std::size_t operator()(const InstructionId& id) const noexcept {
            return std::hash<uint64_t>{}(id.value);
        }
    };
};

} // namespace arm_cpu

namespace arm_cpu {

// =====================================================================
// Instruction
// =====================================================================
struct Instruction {
    InstructionId id;
    uint64_t pc;
    uint32_t raw_opcode;
    OpcodeType opcode_type;
    StaticVector<Reg, 4> src_regs;
    StaticVector<Reg, 2> dst_regs;
    StaticVector<VReg, 4> src_vregs;
    StaticVector<VReg, 2> dst_vregs;
    std::optional<MemAccess> mem_access;
    std::optional<BranchInfo> branch_info;
    std::optional<std::string> disasm;

    Instruction() = default;

    Instruction(InstructionId id_, uint64_t pc_, uint32_t raw, OpcodeType op)
        : id(id_), pc(pc_), raw_opcode(raw), opcode_type(op) {}

    // Builder pattern
    Instruction& with_src_reg(Reg reg) {
        if (!src_regs.contains(reg)) src_regs.push_back(reg);
        return *this;
    }

    Instruction& with_dst_reg(Reg reg) {
        if (!dst_regs.contains(reg)) dst_regs.push_back(reg);
        return *this;
    }

    Instruction& with_src_vreg(VReg reg) {
        if (!src_vregs.contains(reg)) src_vregs.push_back(reg);
        return *this;
    }

    Instruction& with_dst_vreg(VReg reg) {
        if (!dst_vregs.contains(reg)) dst_vregs.push_back(reg);
        return *this;
    }

    Instruction& with_mem_access(uint64_t addr, uint8_t size, bool is_load) {
        mem_access = MemAccess{addr, size, is_load};
        return *this;
    }

    Instruction& with_branch(uint64_t target, bool is_conditional, bool is_taken) {
        branch_info = BranchInfo{is_conditional, target, is_taken};
        return *this;
    }

    Instruction& with_disasm(std::string text) {
        disasm = std::move(text);
        return *this;
    }

    bool reads_reg(Reg reg) const { return src_regs.contains(reg); }
    bool writes_reg(Reg reg) const { return dst_regs.contains(reg); }

    uint64_t instr_latency() const { return latency(opcode_type); }
};

// =====================================================================
// InstrStatus — pipeline stage status
// =====================================================================
enum class InstrStatus : uint8_t {
    Waiting,     // Waiting for operands
    Ready,       // Ready to execute
    Executing,   // Currently executing
    Completed,   // Execution complete, waiting to commit
    Committed,   // Committed to architectural state
};

// =====================================================================
// CacheLineState — MESI-like coherence states
// =====================================================================
enum class CacheLineState : uint8_t {
    Invalid,
    Shared,
    Exclusive,
    Modified,
    Unique,
};

// =====================================================================
// CHI types (defined here for cross-module use)
// =====================================================================
enum class ChiRequestType : uint8_t {
    // Read requests
    ReadNoSnoop, ReadNotSharedDirty, ReadShared, ReadMakeUnique,
    // Write requests
    WriteNoSnoop, WriteUnique, WriteUniquePtl,
    // Coherence requests
    CleanUnique, MakeUnique, Evict,
    // Data responses
    CompData, DataSepResp, NonCopyBackWrData,
    // Acknowledgments
    CompAck,
    // Snoop requests
    SnpOnce, SnpShared, SnpClean, SnpData,
};

enum class ChiResponseStatus : uint8_t {
    Pending,
    Complete,
    Error,
};

} // namespace arm_cpu

// Out-of-class definitions for static const members
inline const arm_cpu::Reg arm_cpu::Reg::SP{31};
inline const arm_cpu::Reg arm_cpu::Reg::XZR{31};
inline const arm_cpu::VReg arm_cpu::VReg::V0{0};
inline const arm_cpu::VReg arm_cpu::VReg::V31{31};

// std::hash specializations for unordered_map/unordered_set usage
template<>
struct std::hash<arm_cpu::InstructionId> {
    std::size_t operator()(const arm_cpu::InstructionId& id) const noexcept {
        return std::hash<uint64_t>{}(id.value);
    }
};

template<>
struct std::hash<arm_cpu::Reg> {
    std::size_t operator()(const arm_cpu::Reg& r) const noexcept {
        return std::hash<uint8_t>{}(r.value);
    }
};

template<>
struct std::hash<arm_cpu::VReg> {
    std::size_t operator()(const arm_cpu::VReg& r) const noexcept {
        return std::hash<uint8_t>{}(r.value);
    }
};
