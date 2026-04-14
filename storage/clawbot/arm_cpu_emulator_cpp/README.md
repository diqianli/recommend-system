# ARM CPU Emulator

ARMv8 时序模拟器（ESL 级别），支持乱序执行（Out-of-Order）、缓存层次建模、Konata 管线可视化。

## 特性

- **乱序执行引擎** — 可配置的指令窗口、发射/提交宽度、重排序缓冲区
- **缓存层次建模** — L1/L2/L3 缓存，支持 hit/miss 统计和 MPKI 计算
- **多格式输入** — text, binary, json, champsim, champsim_xz, **elf**（推荐）
- **Capstone v5 解码** — 支持 ARMv8 基础指令集 + SVE/SVE2/SME 扩展
- **Konata 管线可视化** — 导出 JSON 格式，配套交互式管线时间线查看器
- **多实例并行** — 支持多核仿真场景并行运行
- **零手动依赖** — 所有第三方库通过 CMake FetchContent 自动下载

## 环境要求

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| CMake | >= 3.20 | 构建系统 |
| C++ 编译器 | C++20 | GCC 11+ / Clang 14+ / Apple Clang 15+ |
| Git | 任意 | FetchContent 下载依赖 |

**可选依赖：**

| 依赖 | 说明 |
|------|------|
| aarch64-elf-gcc 或 aarch64-linux-gnu-gcc | 交叉编译测试 ELF 程序 |
| liblzma | ChampSim XZ 压缩 trace 支持（大多数系统已安装） |

## 快速开始

```bash
# 1. 克隆仓库
git clone <repo-url>
cd arm_cpu_emulator_cpp

# 2. 构建（推荐使用脚本）
./scripts/build.sh

# 或手动构建：
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# 3. 运行测试
cd build && ctest --output-on-failure

# 4. 运行仿真
./build/arm_cpu_sim -f elf tests/data/test_elf_aarch64
```

## 自动下载的依赖

所有依赖通过 CMake FetchContent 自动下载和构建，无需手动安装：

| 库 | 版本 | 用途 |
|----|------|------|
| nlohmann/json | v3.11.3 | JSON 解析/生成 |
| spdlog | v1.14.1 | 日志输出 |
| GoogleTest | v1.14.0 | 单元测试 |
| ankerl::unordered_dense | v4.4.0 | 高性能哈希表 |
| Capstone | v5 | ARM64 指令解码 |

## 仿真器使用

### 基本用法

```bash
# ELF 输入（推荐）
./build/arm_cpu_sim -f elf tests/data/test_elf_aarch64

# 指定输出路径
./build/arm_cpu_sim -f elf program.elf -o result.json

# 限制指令数 + verbose 输出
./build/arm_cpu_sim -f elf program.elf -n 100 -v
```

### CPU 配置

```bash
# 自定义窗口大小和发射宽度
./build/arm_cpu_sim -f elf program.elf --window-size 256 --issue-width 6

# 自定义缓存大小（单位 KB）
./build/arm_cpu_sim -f elf program.elf --l1-size 128 --l2-size 1024 --l3-size 16384

# 设置 CPU 频率
./build/arm_cpu_sim -f elf program.elf --frequency 3000
```

### JSON 指标输出

```bash
./build/arm_cpu_sim -f elf program.elf -j
```

输出示例：
```json
{
  "total_instructions": 1234,
  "total_cycles": 567,
  "ipc": 2.176369,
  "cpi": 0.459484,
  "l1_hit_rate": 0.950000,
  "l2_hit_rate": 0.800000,
  "instructions": { ... }
}
```

### 多实例并行

```bash
# 运行 4 个实例，使用 8 线程
./build/arm_cpu_sim -m 4 -t 8 -f elf program.elf
```

### CLI 参数完整列表

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-f, --format <fmt>` | Trace 格式：text, binary, json, champsim, champsim_xz, elf | text |
| `-n, --max-instr <n>` | 最大模拟指令数（0 = 无限制） | 0 |
| `-s, --skip <n>` | 跳过前 N 条指令 | 0 |
| `-c, --max-cycles <n>` | 最大模拟周期数 | 1000000000 |
| `-e, --engine` | 使用 SimulationEngine（替代 CPUEmulator） | off |
| `-v, --verbose` | 详细输出 | off |
| `-j, --json` | JSON 格式输出性能指标 | off |
| `--window-size <n>` | 指令窗口大小 | 128 |
| `--fetch-width <n>` | 取指宽度 | 8 |
| `--issue-width <n>` | 发射宽度 | 4 |
| `--commit-width <n>` | 提交宽度 | 4 |
| `--l1-size <kb>` | L1 缓存大小 (KB) | 64 |
| `--l2-size <kb>` | L2 缓存大小 (KB) | 512 |
| `--l3-size <kb>` | L3 缓存大小 (KB) | 8192 |
| `--frequency <mhz>` | CPU 频率 (MHz) | 2000 |
| `-m, --multi <n>` | 并行运行 N 个实例 | 1 |
| `-t, --threads <n>` | 线程数（默认硬件并发数） | auto |
| `-o, --output <file>` | Konata JSON 输出路径 | output/\<stem\>_YYYYMMDD_HHMM.json |
| `--save-trace <file>` | 保存执行 trace | - |
| `-h, --help` | 显示帮助 | - |
| `--version` | 显示版本 | - |

## 输出说明

仿真运行后会产生两类输出：

1. **终端摘要** — IPC、CPI、缓存命中率、指令分布等性能指标
2. **Konata JSON 文件** — 默认保存到 `output/<输入文件名>_<时间戳>.json`

## 可视化

使用配套的 Konata 管线可视化工具查看仿真结果：

1. 用浏览器打开 `tools/viz_server/index.html`
2. 点击 **"Load JSON File"** 上传仿真输出的 JSON 文件
3. 管线时间线会自动渲染，显示每条指令在 Fetch → Decode → Rename → Dispatch → Issue → Execute → Commit 各阶段的执行情况

## 编译测试 ELF

如果需要自行编译测试输入（用于验证仿真器）：

```bash
# 使用编译脚本（自动检测交叉编译器）
./scripts/compile_test_elf.sh

# 或手动指定编译器
./scripts/compile_test_elf.sh aarch64-linux-gnu-gcc

# 查看源码
cat scripts/test_elf_source.c
```

输出文件：`tests/data/test_elf_aarch64`

## 项目结构

```
arm_cpu_emulator_cpp/
├── CMakeLists.txt              # 构建配置
├── README.md                   # 本文件
├── scripts/
│   ├── build.sh                # 一键构建脚本
│   ├── compile_test_elf.sh     # 编译测试 ELF
│   └── test_elf_source.c       # 测试 ELF 源码
├── include/arm_cpu/
│   ├── cpu.hpp                 # CPUEmulator 主类
│   ├── config.hpp              # CPUConfig 默认配置
│   ├── types.hpp               # 核心数据类型
│   ├── decoder/                # 指令解码（Capstone 后端）
│   ├── input/                  # 输入格式（text/binary/json/champsim/elf）
│   ├── ooo/                    # 乱序执行引擎
│   ├── memory/                 # 内存子系统与缓存层次
│   ├── simulation/             # SimulationEngine
│   ├── stats/                  # 性能指标收集
│   ├── visualization/          # Konata 导出
│   └── multi_instance/         # 多实例并行管理
├── src/                        # 实现文件
├── tests/
│   ├── test_config.cpp         # 配置测试
│   ├── test_ooo.cpp            # 乱序引擎测试
│   ├── test_types.cpp          # 类型测试
│   ├── test_memory.cpp         # 内存子系统测试
│   ├── test_elf_e2e.cpp        # ELF 端到端测试
│   └── data/
│       └── test_elf_aarch64    # 预编译测试 ELF
├── tools/
│   └── viz_server/             # Konata 管线可视化
│       ├── index.html
│       ├── style.css
│       ├── pipeline_view.js
│       ├── app_static.js
│       └── konata/             # Konata 渲染器
└── output/                     # 仿真输出（.gitignore）
```

## 运行测试

```bash
cd build && ctest --output-on-failure
```

测试文件：
- `test_config.cpp` — CPU 配置默认值与自定义
- `test_ooo.cpp` — 乱序引擎依赖检测、重排序缓冲区
- `test_types.cpp` — 指令类型、操作码映射
- `test_memory.cpp` — 缓存层次模拟
- `test_elf_e2e.cpp` — ELF 文件加载 + 仿真的端到端测试

## 许可证

MIT
