# NVIDIA Vera CPU — 完整技术规格报告

> 来源: GTC 2026 / SemiAnalysis / NVIDIA官方博客
> 发布日期: 2026年3月16日 (GTC 2026主题演讲)
> 报告整理日期: 2026年3月30日
> 最后更新: 2026年3月30日

---

## 一、事件概要

| 项目 | 详情 |
|------|------|
| **活动名称** | GTC 2026 (GPU Technology Conference) |
| **日期** | 2026年3月16-19日 |
| **地点** | 美国圣何塞 |
| **产品名** | NVIDIA Vera CPU |
| **平台归属** | Vera Rubin AI计算平台 |
| **角色定位** | 数据引擎/主机CPU (Data Engine / Host CPU) |
| **分类** | AI服务器配套处理器 |

---

## 二、完整技术规格

### 核心与架构

| 规格 | 详情 |
|------|------|
| **制程工艺** | 未披露 (推测台积电3nm/4nm) |
| **核心数量** | **88核** (NVIDIA自研Olympus核心) |
| **微架构** | Olympus (Arm v9.2兼容) |
| **SMT/多线程** | **支持** (空间多线程, 2线程/核, **176线程总计**) |
| **L2缓存** | **2MB/核** (总计176MB，源文件未明确标注，待确认) |
| **L3缓存** | **162 MB** 共享 |
| **基础频率** | 未披露 |
| **TDP功耗** | 未披露 |

### 内存子系统

| 规格 | 详情 |
|------|------|
| **内存类型** | **LPDDR5X** |
| **内存容量** | **1.5 TB** (是Grace的2.6倍) |
| **内存带宽** | **1.2 TB/s** (是Grace的2.4倍) |
| **内存接口** | SOCAMM标准化内存插槽 |

### I/O与互连

| 规格 | 详情 |
|------|------|
| **CPU-GPU互联** | **NVLink-C2C** |
| **互联带宽** | **1.8 TB/s** 一致性带宽 |
| **地址空间** | 统一地址空间 (CPU与GPU显存视为单一内存池) |

---

## 三、Vera Rubin平台协同设计

### 平台六芯片架构

| 芯片名称 | 功能描述 | 关键规格 |
|---------|---------|---------|
| **Rubin GPU** | AI计算核心 | 224个SM，3360亿晶体管，HBM4显存 |
| **Vera CPU** | 数据引擎/主机CPU | 88个Olympus核心，176线程，1.5TB内存 |
| **NVLink 6 Switch** | 机架内GPU互联 | 3.6 TB/s带宽/GPU，SHARP网络内计算 |
| **ConnectX-9** | 网络接口卡 | 800Gb/s，支持InfiniBand和以太网 |
| **BlueField-4 DPU** | 基础设施处理 | 64核Grace CPU，128GB LPDDR5x |
| **Spectrum-6** | 以太网交换机 | 102.4 Tb/s，CPO共封装光学 |

### 极致协同设计理念

Vera Rubin采用"Extreme Co-Design"理念，将CPU、GPU、网络、安全、软件、供电和冷却作为一个**整体系统**协同构建，而非独立芯片组合。

---

## 四、与前代Grace CPU对比

| 指标 | Grace (搭配Hopper) | Vera (搭配Rubin) | 提升 |
|------|:------------------:|:----------------:|:----:|
| **核心数** | 72核 | 88核 | 22% |
| **内存容量** | 576 GB | 1.5 TB | **2.6倍** |
| **内存带宽** | 500 GB/s | 1.2 TB/s | **2.4倍** |
| **CPU-GPU互联** | NVLink-C2C 900 GB/s | NVLink-C2C 1.8 TB/s | **2倍** |

---

## 五、在Vera Rubin平台中的角色

### 功能定位

1. **数据引擎**: 为Rubin GPU提供高速数据预处理和馈送
2. **主机CPU**: 运行操作系统、管理AI训练框架
3. **统一内存池**: 通过NVLink-C2C与GPU显存形成统一地址空间
4. **推理协调**: 在MoE模型推理中管理专家路由

### 与Rubin GPU的协同

| 协同特性 | 实现方式 |
|---------|---------|
| **统一内存** | CPU与GPU共享1.8 TB/s一致性互联 |
| **零拷贝** | 数据无需在CPU/GPU间复制 |
| **显存扩展** | CPU 1.5TB内存可作为GPU显存扩展 |

---

## 六、Vera CPU Rack（独立CPU机架产品）

> 来源: NVIDIA官方产品页 https://www.nvidia.com/en-us/data-center/products/vera-rack/

NVIDIA提供独立的**Vera CPU Rack**产品，专为强化学习和Agentic AI工作负载设计的机架级CPU基础设施。

### Vera CPU Rack规格

| 项目 | Vera CPU Rack | 单颗Vera CPU |
|------|--------------|-------------|
| **配置** | 256颗Vera CPU | 1颗 |
| **CPU核心** | 22,528核 (Olympus) | 88核 |
| **线程数** | 45,056线程 | 176线程 |
| **内存容量** | Up to 400 TB | Up to 1.5 TB |
| **聚合带宽** | Up to 315 TB/s | Up to 1.2 TB/s |
| **基础架构** | NVIDIA MGX | — |

---

## 七、Vera Rubin NVL72系统配置

### Vera CPU在NVL72中的配置（GPU平台中的CPU部分）

| 项目 | NVL72整机架规格 |
|------|----------------|
| **Vera CPU数量** | 36颗 (LPDDR5X总容量54TB ÷ 1.5TB/颗，源文件未直接给出) |
| **CPU核心总数** | 3,168核 (88 × 36) |
| **CPU线程总数** | 6,336线程 (176 × 36) |
| **CPU内存总量** | 54 TB (源文件明确记载) |
| **Rubin GPU数量** | 72颗 |
| **CPU-GPU总带宽** | 64.8 TB/s (1.8TB/s × 36) |

### NVL72整机架性能

| 指标 | Rubin NVL72数值 |
|------|-----------------|
| **LPDDR5X总容量** | 54 TB (CPU内存) |
| **HBM4总容量** | 20.7 TB (GPU显存) |
| **纵向扩展带宽** | 260 TB/s |
| **机架TDP** | 180-220 kW |
| **冷却方式** | 100%液冷 |

---

## 八、战略意义

### 1. 双重产品战略

Vera CPU同时具备两种部署形态：
- **独立CPU机架**: Vera CPU Rack (256颗CPU)，面向强化学习和Agentic AI纯CPU工作负载
- **GPU平台配套**: Vera Rubin NVL72 (36颗CPU + 72颗GPU)，面向大规模AI训练

### 2. 平台协同设计

在Vera Rubin AI工厂平台中，Vera CPU作为核心配套组件，通过CPU+GPU+网络的**三件套协同设计**构建生态壁垒。

### 2. 客户锁定

首批客户均为云厂商与大模型公司：
- AWS
- Microsoft Azure
- Google Cloud
- Meta
- OpenAI
- CoreWeave

### 3. 技术路径

- **自研Olympus核心**: 摆脱对Arm公版核心依赖
- **Arm v9.2兼容**: 保持软件生态兼容性
- **NVLink-C2C互联**: 与GPU形成差异化竞争优势

### 4. 与Grace的演进

Grace是NVIDIA首款数据中心CPU（搭配Hopper GH100），Vera是第二代（搭配Rubin），体现出NVIDIA在CPU领域的持续投入。

---

## 九、未确认规格

### 仍未披露

- [ ] 制程工艺细节
- [ ] 基础/加速频率
- [ ] TDP功耗
- [ ] L1缓存大小
- [ ] Die尺寸
- [ ] 具体benchmark数值 (SPEC CPU等)
- [ ] Vera CPU Rack机架功耗
- [ ] 价格信息

---

## 十、信息来源

1. **SemiAnalysis** — NVIDIA Vera Rubin平台深度分析
2. **NVIDIA官方博客** — GTC 2026产品发布
3. **NVIDIA官方产品页** — Vera CPU Rack (https://www.nvidia.com/en-us/data-center/products/vera-rack/)
4. **智源社区** — GTC 2026技术解析
5. **至顶网** — Vera Rubin平台报道
6. **本地源文件**: `/Users/mac/storage/clawbot/gtc2026-leaks/AI训练芯片泄露信息汇总.md`

---

*本报告基于GTC 2026发布信息及泄露技术规格整理*
