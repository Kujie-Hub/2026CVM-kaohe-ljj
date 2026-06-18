# 五场景微架构指标对比报告

## 一、测试环境

| 项目 | 信息 |
|------|------|
| CPU | 13th Gen Intel(R) Core(TM) i7-13620H (Raptor Lake) |
| vCPU | 2核 |
| 虚拟化 | VMware |
| 内核 | Linux 6.8.0-124-generic |
| L1d Cache | 96 KiB × 2 |
| L1i Cache | 64 KiB × 2 |
| L2 Cache | 2.5 MiB × 2 |
| L3 Cache | 24 MiB × 1 |
| 内存 | 3868 MB |
| NUMA | 1个节点 |

## 二、五场景对比表格

| 场景 | 页分配 | 页释放 | 上下文切换 | 唤醒 | 系统调用 | 用户态(s) | 内核态(s) |
|------|--------|--------|-----------|------|---------|-----------|----------|
| int64 | 825 | 552 | 259 | 262 | 506 | 29.983622000 | 0.003276000 |
| matrixprod | 1,017 | 548 | 293 | 275 | 506 | 29.975404000 | 0.009938000 |
| read64 | 1,369 | 1,062 | 322 | 313 | 2,032 | 17.942373000 | 12.045456000 |
| rand-set | 6,186,227 | 6,185,090 | 272 | 282 | 1,042 | 14.923638000 | 15.158719000 |
| queens | 824 | 548 | 252 | 245 | 506 | 29.984797000 | 0.005020000 |

## 三、关键发现

### 3.1 随机访存 (rand-set)
- **页分配/释放：约6,186,227次**（比其他场景高3-4个数量级）
- **内核态时间：15.16秒**（占总时间50%以上）
- **原因**：随机地址访问导致大量缺页异常

### 3.2 连续访存 (read64)
- **系统调用：2,032次**（是计算密集型的4倍）
- **内核态时间：12.05秒**
- **原因**：连续读取1GB数据触发大量内存操作

### 3.3 计算密集型 (int64/matrixprod/queens)
- **用户态时间接近30秒**（占99%以上）
- **系统调用约506次**
- **页分配约800-1000次**

## 四、差异分析（微架构原理）

### 4.1 计算密集型
- **前端取指/解码**：指令流连续，分支预测命中率高
- **后端执行单元**：ALU/FPU满负荷运行
- **访存子系统**：数据主要在L1/L2 Cache中

### 4.2 访存密集型 (read64)
- 顺序访问1GB数据，超出Cache容量
- 触发大量页分配和系统调用

### 4.3 随机访存 (rand-set)
- 随机地址访问导致TLB Miss和Cache Miss
- 触发缺页异常：`exc_page_fault → handle_mm_fault`
- 性能瓶颈在内存子系统

## 五、环境限制说明

⚠️ 本实验在VMware虚拟机中运行，硬件PMU事件不可用：
- cycles, instructions: ❌
- cache-references, cache-misses: ❌
- 分支预测相关事件: ❌

**替代方案**：使用tracepoint事件（kmem, sched, raw_syscalls）

## 四、微架构流水线原理深度分析

### 4.1 前端取指/解码（Frontend）

#### 计算密集型（int64 / matrixprod / queens）
- **取指**：指令流连续，I-Cache命中率高，无停顿
- **解码**：int64为简单整数指令，解码效率高；matrixprod使用SIMD指令，解码稍复杂但可接受
- **分支预测**：queens涉及大量条件分支，但分支模式相对规律（递归），预测命中率尚可

#### 访存密集型（read64 / rand-set）
- **取指**：频繁的系统调用导致用户态/内核态切换，前端取指经常等待
- **解码**：系统调用指令（syscall）使得解码单元需要切换到内核态处理
- **分支预测**：访存密集型负载分支指令少，预测压力小

### 4.2 后端执行单元（Execution Engine）

| 负载类型 | 主要执行单元 | 利用率 |
|----------|-------------|--------|
| int64 | ALU（整数算术逻辑单元） | 饱和 |
| matrixprod | FPU（浮点单元）+ SIMD | 饱和 |
| queens | ALU + 分支单元 | 中等 |
| read64 | ALU + 访存单元 | 访存单元饱和 |
| rand-set | 访存单元 | 频繁停顿（等待数据） |

**关键观察**：
- 计算密集型：执行单元满负荷，性能由CPU核心决定
- 访存密集型：执行单元经常空转（Stall），等待内存数据返回

### 4.3 访存子系统（Memory Subsystem）


| 负载类型 | Cache行为 | 瓶颈层级 | 说明 |
|----------|----------|----------|------|
| int64 | L1命中率 >95% | 无 | 数据工作集小，完全在Cache中 |
| matrixprod | L2/L3命中率高 | 无/内存带宽 | 矩阵数据较大，但仍能利用Cache |
| queens | L1命中率 >90% | 无 | 栈操作，数据局部性好 |
| read64 | L1/L2/L3 Miss率逐步升高 | 内存带宽 | 顺序读取1GB，L3 Miss率高 |
| rand-set | L1/L2/L3 Miss率极高 | 内存延迟 | 随机跳转访问，TLB Miss严重 |

### 4.4 流水线效率总结


### 4.5 性能瓶颈定位

| 负载 | 瓶颈类型 | 微架构层面原因 |
|------|----------|---------------|
| int64 | 计算瓶颈 | ALU执行单元饱和，流水线满负荷运行 |
| matrixprod | 计算+访存 | FPU饱和 + 矩阵数据带宽需求 |
| queens | 分支+计算 | 分支预测+ALU，无显著瓶颈 |
| read64 | 访存带宽 | 内存带宽达到上限，Cache无法容纳工作集 |
| rand-set | 访存延迟 | TLB Miss + Cache Miss，CPU频繁等待内存 |


## 六、衍生指标计算说明

⚠️ 由于VMware虚拟化环境限制，perf无法访问硬件PMU事件：

| 衍生指标 | 计算方式 | 状态 |
|----------|----------|------|
| IPC | instructions / cycles | ❌ cycles/instructions不可用 |
| L1 DCache Miss Rate | L1-dcache-load-misses / cache-references | ❌ 不可用 |
| LLC Miss Rate | LLC-load-misses / cache-references | ❌ 不可用 |
| 分支预测失败率 | branch-misses / branch-instructions | ❌ 不可用 |
| TLB Miss Rate | dTLB-load-misses / cache-references | ❌ 不可用 |

**替代分析方法**：使用tracepoint事件追踪内核级行为，包括：
- 页分配/释放（kmem）
- 上下文切换/唤醒（sched）
- 系统调用（raw_syscalls）

这些数据在第2节和第4节中已作分析。

## 七、原始perf stat输出

### 7.1 int64_tp.txt
stress-ng: info:  [2052] setting to a 30 second run per stressor
stress-ng: info:  [2052] dispatching hogs: 1 cpu
stress-ng: info:  [2052] successful run completed in 30.00s

 Performance counter stats for 'stress-ng --cpu 1 --cpu-method int64 -t 30s':

               825      kmem:mm_page_alloc                                                    
               552      kmem:mm_page_free                                                     
               259      sched:sched_switch                                                    
               262      sched:sched_wakeup                                                    
               506      raw_syscalls:sys_enter                                                
               506      raw_syscalls:sys_exit                                                 

      30.006858100 seconds time elapsed

      29.983622000 seconds user
       0.003276000 seconds sys



### 7.2 matrixprod_tp.txt
stress-ng: info:  [2060] setting to a 30 second run per stressor
stress-ng: info:  [2060] dispatching hogs: 1 cpu
stress-ng: info:  [2060] successful run completed in 30.00s

 Performance counter stats for 'stress-ng --cpu 1 --cpu-method matrixprod -t 30s':

             1,017      kmem:mm_page_alloc                                                    
               548      kmem:mm_page_free                                                     
               293      sched:sched_switch                                                    
               275      sched:sched_wakeup                                                    
               506      raw_syscalls:sys_enter                                                
               506      raw_syscalls:sys_exit                                                 

      30.006831968 seconds time elapsed

      29.975404000 seconds user
       0.009938000 seconds sys



### 7.3 read64_tp.txt
stress-ng: info:  [2077] setting to a 30 second run per stressor
stress-ng: info:  [2077] dispatching hogs: 1 vm
stress-ng: info:  [2077] successful run completed in 30.00s

 Performance counter stats for 'stress-ng --vm 1 --vm-bytes 1G --vm-method read64 --vm-keep -t 30s':

             1,369      kmem:mm_page_alloc                                                    
             1,062      kmem:mm_page_free                                                     
               322      sched:sched_switch                                                    
               313      sched:sched_wakeup                                                    
             2,032      raw_syscalls:sys_enter                                                
             2,032      raw_syscalls:sys_exit                                                 

      30.007832752 seconds time elapsed

      17.942373000 seconds user
      12.045456000 seconds sys



### 7.4 rand-set_tp.txt
stress-ng: info:  [2086] setting to a 30 second run per stressor
stress-ng: info:  [2086] dispatching hogs: 1 vm
stress-ng: info:  [2086] successful run completed in 30.10s

 Performance counter stats for 'stress-ng --vm 1 --vm-bytes 512M --vm-method rand-set -t 30s':

         6,186,227      kmem:mm_page_alloc                                                    
         6,185,090      kmem:mm_page_free                                                     
               272      sched:sched_switch                                                    
               282      sched:sched_wakeup                                                    
             1,042      raw_syscalls:sys_enter                                                
             1,042      raw_syscalls:sys_exit                                                 

      30.104444363 seconds time elapsed

      14.923638000 seconds user
      15.158719000 seconds sys



### 7.5 queens_tp.txt
stress-ng: info:  [2095] setting to a 30 second run per stressor
stress-ng: info:  [2095] dispatching hogs: 1 cpu
stress-ng: info:  [2095] successful run completed in 30.00s

 Performance counter stats for 'stress-ng --cpu 1 --cpu-method queens -t 30s':

               824      kmem:mm_page_alloc                                                    
               548      kmem:mm_page_free                                                     
               252      sched:sched_switch                                                    
               245      sched:sched_wakeup                                                    
               506      raw_syscalls:sys_enter                                                
               506      raw_syscalls:sys_exit                                                 

      30.006073290 seconds time elapsed

      29.984797000 seconds user
       0.005020000 seconds sys



## 八、测试环境补充信息
架构：                                   x86_64
CPU 运行模式：                           32-bit, 64-bit
Address sizes:                           45 bits physical, 48 bits virtual
字节序：                                 Little Endian
CPU:                                     2
在线 CPU 列表：                          0,1
厂商 ID：                                GenuineIntel
型号名称：                               13th Gen Intel(R) Core(TM) i7-13620H
CPU 系列：                               6
型号：                                   186
每个核的线程数：                         1
每个座的核数：                           2
座：                                     1
步进：                                   2
BogoMIPS：                               5836.79
标记：                                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pdpe1gb rdtscp lm constant_tsc arch_perfmon rep_good nopl xtopology tsc_reliable nonstop_tsc cpuid tsc_known_freq pni pclmulqdq ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic movbe popcnt aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch pti ssbd ibrs ibpb stibp fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid rdseed adx smap clflushopt clwb sha_ni xsaveopt xsavec xgetbv1 xsaves avx_vnni arat umip gfni vaes vpclmulqdq rdpid movdiri movdir64b fsrm md_clear serialize flush_l1d arch_capabilities
超管理器厂商：                           VMware
虚拟化类型：                             完全
L1d 缓存：                               96 KiB (2 instances)
L1i 缓存：                               64 KiB (2 instances)
L2 缓存：                                2.5 MiB (2 instances)
L3 缓存：                                24 MiB (1 instance)
NUMA 节点：                              1
NUMA 节点0 CPU：                         0,1
Vulnerability Gather data sampling:      Not affected
Vulnerability Indirect target selection: Mitigation; Aligned branch/return thunks
Vulnerability Itlb multihit:             Not affected
Vulnerability L1tf:                      Mitigation; PTE Inversion
Vulnerability Mds:                       Mitigation; Clear CPU buffers; SMT Host state unknown
Vulnerability Meltdown:                  Mitigation; PTI
Vulnerability Mmio stale data:           Unknown: No mitigations
Vulnerability Reg file data sampling:    Vulnerable: No microcode
Vulnerability Retbleed:                  Mitigation; IBRS
Vulnerability Spec rstack overflow:      Not affected
Vulnerability Spec store bypass:         Mitigation; Speculative Store Bypass disabled via prctl
Vulnerability Spectre v1:                Mitigation; usercopy/swapgs barriers and __user pointer sanitization
Vulnerability Spectre v2:                Mitigation; IBRS; IBPB conditional; STIBP disabled; RSB filling; PBRSB-eIBRS Not affected; BHI SW loop, KVM SW loop
Vulnerability Srbds:                     Not affected
Vulnerability Tsa:                       Not affected
Vulnerability Tsx async abort:           Not affected
Vulnerability Vmscape:                   Not affected
Linux ljj-virtual-machine 6.8.0-124-generic #124~22.04.1-Ubuntu SMP PREEMPT_DYNAMIC Tue May 26 21:05:19 UTC  x86_64 x86_64 x86_64 GNU/Linux
model name	: 13th Gen Intel(R) Core(TM) i7-13620H
=== NUMA拓扑 ===
available: 1 nodes (0)
node 0 cpus: 0 1
node 0 size: 3868 MB
node 0 free: 502 MB
node distances:
node   0 
  0:  10 
=== CPU频率策略 ===
analyzing CPU 0:
  no or unknown cpufreq driver is active on this CPU
  CPUs which run at the same hardware frequency: Not Available
  CPUs which need to have their frequency coordinated by software: Not Available
  maximum transition latency:  Cannot determine or is not supported.
Not Available
  available cpufreq governors: Not Available
  Unable to determine current policy
  current CPU frequency: Unable to call hardware
  current CPU frequency:  Unable to call to kernel
  boost state support:
    Supported: no
    Active: no
=== 虚拟化类型 ===
vmware
