# Arm AGI CPU — 完整技术规格报告

> 来源: ServeTheHome / The Register / Arm Everywhere Event
> 发布日期: 2026年3月24日
> 报告整理日期: 2026年3月30日
> 最后更新: 2026年3月30日 (补充The Register详细信息)

---

## 一、事件概要

| 项目 | 详情 |
|------|------|
| **活动名称** | Arm Everywhere |
| **日期** | 2026年3月24日 |
| **地点** | 美国旧金山 |
| **产品名** | Arm AGI CPU |
| **芯片编号** | BP113066 |
| **制造地点** | 台湾 (台积电) |
| **分类** | Server CPUs (服务器处理器) |
| **预计量产** | 2026年下半年 |

---

## 二、完整技术规格

### 核心与架构

| 规格 | 详情 |
|------|------|
| **制程工艺** | 台积电3nm (TSMC 3nm) |
| **核心数量** | 136核 (Arm Neoverse V3) |
| **微架构** | Neoverse V3 (CSS V3) |
| **SMT/多线程** | **不支持** (每核1线程, 追求确定性性能) |
| **L2缓存** | 每核2MB (总计272MB L2) |
| **系统级缓存 (SLC)** | **128 MB** 共享 |
| **基础频率** | **3.2 GHz** |
| **加速频率** | **3.7 GHz** |
| **TDP功耗** | **300W** |
| **芯片设计** | **双芯片 (Dual-chiplet)** 设计 |
| **芯片互联** | AMBA CHI extension links |
| **NUMA拓扑** | 双NUMA域 (每socket暴露为2个NUMA节点) |
| **设计理念** | Clean sheet设计，无遗留加速器，100%为目标负载服务 |

### 内存子系统

| 规格 | 详情 |
|------|------|
| **内存通道** | **12通道** (每die 6通道) |
| **内存类型** | DDR5 |
| **最高内存速率** | **DDR5-8800** |
| **每核内存带宽** | **6 GB/s** |
| **总带宽** | **825 GB/s** |
| **内存延迟** | **< 100 ns** (亚100纳秒) |
| **内存调优** | 为计算负载优化 (Memory tuned for compute) |
| **集成方式** | 内存与I/O集成在计算die内(非独立I/O die)，降低延迟 |
| **CXL支持** | **CXL 3.0** (支持内存扩展) |

### I/O与互连

| 规格 | 详情 |
|------|------|
| **PCIe通道** | **96条 PCIe Gen6** |
| **CXL版本** | CXL 3.0 |
| **芯片互连** | AMBA CHI extension links |

> 注: 两个die设计相同，PCIe和内存控制器分布在两个die上(每die 6通道DDR5)。
> 内存和I/O集成在计算die内(非独立I/O die)，每个socket暴露为2个NUMA域。
> 类似 Intel Emerald Rapids (5th Gen Xeon) 的双die集成方案。

---

## 三、机架配置方案

### 风冷方案 (Air-Cooled)

| 项目 | 详情 |
|------|------|
| **机架标准** | Open Rack V3 |
| **功率** | 36kW |
| **服务器配置** | 30台 双节点1U服务器 |
| **CPU核心总数** | **8,160核** |
| **内存总量** | 180TB+ 低延迟内存 |

### 液冷方案 (Liquid-Cooled)

| 项目 | 详情 |
|------|------|
| **机架标准** | Open Rack Wide |
| **功率** | 200kW (实际密度约100kW) |
| **服务器配置** | 42台 8节点1U服务器 |
| **CPU核心总数** | **45,696核** |

---

## 四、性能对比

### vs x86 (AMD EPYC)

基于Arm发布的Agentic AI工作负载对比数据 (相对值):

| 指标 | Arm AGI CPU | x86 (SMT关闭) | x86 (SMT开启) |
|------|:-----------:|:-------------:|:-------------:|
| **每线程持续性能** | ~1.3x | ~1.1x | ~0.9x |
| **每机架持续线程数** | ~1.9x | ~1.0x | ~1.3x |
| **每瓦性能** | ~2.0x | ~1.0x | ~1.2x |

> Arm以AMD EPYC为x86对比对象(非Intel Xeon)

### vs NVIDIA Vera CPU

| 指标 | Arm AGI CPU (液冷) | NVIDIA Vera ETL256 |
|------|:------------------:|:-------------------:|
| **每机架核心数** | **45,696核** | 22,528核 |
| **核心密度** | **~2.0x** | 1.0x |

> 注: NVIDIA Vera CPU Rack数据 (22,528核 = 88核 × 256颗CPU) 已由NVIDIA官方产品页确认:
> https://www.nvidia.com/en-us/data-center/products/vera-rack/
> 该数据与Arm发布会幻灯片引用一致 (ServeTheHome拍摄, 第15张)。

---

## 五、产品路线图

| 时间 | 产品 | 架构 |
|------|------|------|
| **Now (2026)** | **Arm AGI CPU** | CSS V3 |
| **Future** | Arm AGI CPU 2 | CSS V5 |
| **Future** | Arm AGI CPU 3 | CSS V5 |

---

## 六、生态系统与合作伙伴

### 首发客户
- **Meta** — 头条客户，计划大规模部署
- **OpenAI** — 发布会站台
- **Cloudflare**
- **SAP**
- **Cerebras** — AI加速器厂商
- **F5** — 网络与安全
- **SK Telecom** — 韩国电信
- **Rebellions** — AI芯片初创

### 系统合作伙伴
- **Lenovo** (联想) — 正在开发19英寸标准机架系统
- **ASRock Rack**
- **QCT** (广达云科技)
- **Supermicro**

### 发布会嘉宾
- **NVIDIA CEO 黄仁勋** — 站台支持

### Arm关键人物
- **Mohamed Awad** — Arm EVP of Cloud AI (云AI执行副总裁)

---

## 七、战略意义

1. **ARM成立35年来首次转型** — 从纯IP授权商转为芯片成品供应商
2. **对标竞争**: 直接以AMD EPYC为竞争对象，提供单路136核方案
3. **Agentic AI定位**: 专为AI智能体工作负载设计，强调低延迟、高带宽
4. **CXL生态**: DDR5供应紧张背景下，CXL 3.0内存扩展成为重要卖点
5. **量产计划**: 2026年下半年进入生产
6. **Clean Sheet设计**: 不包含遗留加速器或兼容功能，100%为目标负载优化
7. **No SMT策略**: 每核单线程设计，追求确定性性能扩展 (vs x86 SMT)
8. **NUMA架构**: 双die设计使每个socket暴露为2个NUMA域
9. **市场定位**: 不仅限于AI Agent，还可作为自定义加速器头节点、通用网络/存储CPU
10. **竞争格局**: 填补Ampere Computing之外的独立Arm服务器CPU市场空白

---

## 八、已确认/待查规格状态

### 已确认 (2026-03-30更新)

- [x] ~~L2缓存: 每核2MB (总计272MB)~~ — 多源确认
- [x] ~~SLC (系统级缓存): 128 MB 共享~~ — The Register确认
- [x] ~~TDP功耗: 300W~~ — The Register确认
- [x] ~~基础频率: 3.2 GHz~~ — The Register确认
- [x] ~~加速频率: 3.7 GHz~~ — ServeTheHome幻灯片确认
- [x] ~~内存总带宽: 825 GB/s~~ — The Register确认 (修正原~300 GB/s估计)
- [x] ~~每核内存带宽: 6 GB/s~~ — The Register确认
- [x] ~~SMT: 不支持 (1线程/核)~~ — The Register确认
- [x] ~~NUMA: 双NUMA域/socket~~ — The Register确认
- [x] ~~PCIe: 96条 Gen6~~ — 多源确认
- [x] ~~CXL: 3.0~~ — 多源确认
- [x] ~~Die设计: 内存/I/O集成在计算die内~~ — The Register确认

### 仍未确认

- [ ] L1缓存大小 (每核指令/数据缓存)
- [ ] Die尺寸 (芯片面积)
- [ ] 具体benchmark绝对数值 (SPEC、MLPerf等)
- [ ] Arm Everywhere Event 完整演讲视频/文稿
- [ ] 技术PPT原始文件下载

---

## 九、信息来源

1. **ServeTheHome** (英文，详细技术报道):
   https://www.servethehome.com/arm-agi-cpu-launched-establishing-arm-as-a-silicon-provider/
2. **The Register** (英文，最详细补充报道, Tobias Mann):
   https://www.theregister.com/2026/03/24/arm_agi_cpu/
3. **Arm Everywhere Event 现场幻灯片** (ServeTheHome拍摄):
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-11.jpg (规格总览)
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-9.jpg (内存子系统)
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-12.jpg (机架配置)
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-15.jpg (性能对比)
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-17.jpg (路线图)
   - https://www.servethehome.com/wp-content/uploads/2026/03/Arm-Everywhere-Event-Arm-AGI-CPU-Launch-5.jpg (实物照片)
4. **中文媒体报道** (通过搜索引擎发现):
   - "Arm发布首款AGI CPU处理器:台积电3nm制程、136核设计、支持DDR5"
   - "Arm AGI CPU更多细节:台积电3nm制程、Neoverse V3微架构" (凤凰网)
   - "Arm发布136核AGI CPU追赶AI热潮" (澎湃新闻)
   - "Arm发布自研AGI CPU,136核专为AI设计,获Meta首发采用" (什么值得买)
