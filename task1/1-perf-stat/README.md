# 任务1-①：多场景微架构指标采集

## 运行说明

### 环境准备
sudo apt install -y stress-ng linux-tools-common

### 采集命令
sudo perf stat -e kmem:mm_page_alloc,kmem:mm_page_free,sched:sched_switch,sched:sched_wakeup,raw_syscalls:sys_enter,raw_syscalls:sys_exit stress-ng --cpu 1 --cpu-method int64 -t 30s

### 五类负载
序号 负载类型 测试命令
① 整数运算 stress-ng --cpu 1 --cpu-method int64 -t 30s
② 矩阵乘法 stress-ng --cpu 1 --cpu-method matrixprod -t 30s
③ 连续访存 stress-ng --vm 1 --vm-bytes 1G --vm-method read64 --vm-keep -t 30s
④ 随机访存 stress-ng --vm 1 --vm-bytes 512M --vm-method rand-set -t 30s
⑤ N皇后问题 stress-ng --cpu 1 --cpu-method queens -t 30s

### 结果文件
results/*.txt perf stat原始输出
report.md 五场景对比表格加差异分析报告
env_info.txt 完整环境信息

## 环境信息
CPU 13th Gen Intel(R) Core(TM) i7-13620H Raptor Lake
vCPU 2核
虚拟化 VMware
内核 Linux 6.8.0-124-generic
L1d Cache 96 KiB x 2
L1i Cache 64 KiB x 2
L2 Cache 2.5 MiB x 2
L3 Cache 24 MiB x 1
内存 3868 MB
NUMA 1个节点

## 五场景对比结果
场景 页分配 页释放 上下文切换 系统调用 用户态 内核态
int64 825 552 259 506 29.98s 0.003s
matrixprod 1017 548 293 506 29.98s 0.010s
read64 1369 1062 322 2032 17.94s 12.05s
rand-set 6186227 6185090 272 1042 14.92s 15.16s
queens 824 548 252 506 29.98s 0.005s

## 关键发现
随机访存rand-set页分配约618万次内核态时间15.16秒占总时间百分之五十以上
连续访存read64系统调用2032次是计算密集型的4倍
计算密集型int64 matrixprod queens用户态时间接近30秒占百分之九十九以上

## 环境限制说明
本实验在VMware虚拟机中运行硬件PMU事件不可用包括cycles instructions cache-references cache-misses branch-instructions branch-misses L1-dcache-load-misses LLC-load-misses dTLB-load-misses
替代方案使用tracepoint事件kmem sched raw_syscalls和软件事件cpu-clock page-faults

## 衍生指标说明
由于VMware虚拟化限制以下衍生指标无法计算IPC instructions除以cycles L1 DCache Miss Rate L1-dcache-load-misses除以cache-references LLC Miss Rate LLC-load-misses除以cache-references 分支预测失败率 branch-misses除以branch-instructions TLB Miss Rate dTLB-load-misses除以cache-references
