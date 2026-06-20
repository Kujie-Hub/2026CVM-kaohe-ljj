# 2026 CVM 微架构性能测评 — 校企合作 Mini 项目

## 个人信息
- 姓名：ljj
- GitHub：Kujie-Hub
- 环境：VMware Ubuntu 22.04.5

## 题目完成情况

| 题目 | 状态 | 说明 |
|------|------|------|
| 任务1-①：多场景微架构指标采集 | ✅ 已完成 | 使用 tracepoint 事件替代硬件 PMU 事件 |
| 任务1-②：火焰图生成与热点分析 | ✅ 已完成 | 对比矩阵乘法与随机访存火焰图 |
| 任务1-③：Cache Line 微基准测试 | ✅ 已完成 | 9 种步长测试，验证 64B Cache Line 拐点 |
| 题目2：持续 CPU Profiling 工具 | ✅ 代码存在 | 基于 Dockerfile 的容器化持续采集工具 |

## 环境限制说明
- 本实验在 VMware 虚拟机中运行，宿主机内核与虚拟机环境对 perf 访问有一定限制。
- 硬件 PMU 事件在当前虚拟机中不可用，因此 Task1 使用 tracepoint 事件替代。
- 题目2 的容器化工具需要匹配宿主机内核版本的 `linux-tools`，并使用 `--privileged` 模式运行。

## 仓库结构
.
├── task1/
│   ├── 1-perf-stat/          # perf stat 采集与微架构指标对比分析
│   ├── 2-flamegraph/         # FlameGraph 火焰图生成与热点分析
│   └── 3-cache-line-test/    # Cache Line 微基准测试
└── task2/                    # 持续 CPU Profiling 工具源码与 Docker 配置
    ├── Dockerfile
    ├── requirements.txt
    ├── src/
    └── test/

## 说明
- `task1` 为本次实验的核心作业内容，已完成 3 个子任务，结果与报告存放在各子目录下。
- `task2` 为题目2的容器化持续 CPU Profiling 工具开发，目录包含源码、`Dockerfile` 与测试脚本。

运行与注意事项：
- 若要构建并运行 `task2` 容器（需宿主机支持 `perf` 与相应 `linux-tools`）：
  ```bash
  cd task2
  docker build -t profiler:latest .
  # 运行需使用 --privileged 模式并映射数据目录以持久化采样文件
  docker run --privileged -d --name profiler -p 8080:8080 -v /data/profiler:/data profiler:latest
  ```
- 注意：容器持续采样依赖宿主机的 PMU，虚拟化环境（如 VMware）可能限制硬件事件访问，实际使用时请确保宿主机内核安装并匹配 `linux-tools`，并以 `--privileged` 启动。
