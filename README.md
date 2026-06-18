# 2026 CVM 微架构性能测评 — 校企合作 Mini 项目

## 个人信息
- 姓名：ljj
- GitHub：Kujie-Hub
- 环境：VMware Ubuntu 22.04.5

## 题目完成情况

| 题目 | 状态 | 说明 |
|------|------|------|
| 任务1-①：多场景微架构指标采集 | ✅ 已完成 | tracepoint事件替代硬件事件 |
| 任务1-②：火焰图生成与热点分析 | ✅ 已完成 | 2种负载火焰图 |
| 任务1-③：Cache Line微基准测试 | ✅ 已完成 | 9种步长测试 |
| 任务2：持续CPU Profiling工具 | ⬜ 未选做 | - |

## 环境限制说明
本实验在VMware虚拟机中运行，硬件PMU事件不可用，使用tracepoint事件替代。

## 仓库结构
task1/
├── 1-perf-stat/ # perf stat采集 + 对比分析
├── 2-flamegraph/ # 火焰图生成 + 热点分析
└── 3-cache-line-test/ # Cache Line微基准测试
