# 任务1-②：火焰图生成与热点分析

## 运行说明

### 前置准备
git clone https://github.com/brendangregg/FlameGraph.git ~/FlameGraph

### 生成火焰图

#### 矩阵乘法计算密集型
sudo perf record -F 99 -g -- stress-ng --cpu 1 --cpu-method matrixprod -t 30s
sudo perf script > matrixprod.script
~/FlameGraph/stackcollapse-perf.pl matrixprod.script | ~/FlameGraph/flamegraph.pl --title="矩阵乘法火焰图" > flamegraphs/matrixprod_flame.svg

#### 随机访存访存密集型
sudo perf record -F 99 -g -- stress-ng --vm 1 --vm-bytes 512M --vm-method rand-set -t 30s
sudo perf script > randmem.script
~/FlameGraph/stackcollapse-perf.pl randmem.script | ~/FlameGraph/flamegraph.pl --title="随机访存火焰图" > flamegraphs/randmem_flame.svg

## 结果文件
flamegraphs/matrixprod_flame.svg 矩阵乘法火焰图
flamegraphs/randmem_flame.svg 随机访存火焰图
report.md 火焰图对比分析报告

## 火焰图对比分析

### 宽度分布差异
矩阵乘法计算密集型 火焰图形状扁平 宽度分布宽热点分散 主要占用CPU的函数 stress-ng主函数计算相关函数
随机访存访存密集型 火焰图形状尖塔 宽度分布窄热点集中 主要占用CPU的函数 exc_page_fault handle_mm_fault wp_page_copy

### 热点函数对比
矩阵乘法计算密集型调用栈从底部到顶部 stress-ng -> unknown -> stress-ng 热点主要集中在stress-ng主函数和计算相关函数中没有深度内核调用栈
随机访存访存密集型调用栈从底部到顶部 stress-ng -> madvise -> entry_SYSCALL_64 -> asm_exc_page_fault -> exc_page_fault -> do_user_addr_fault -> handle_mm_fault -> handle_pte_fault -> do_wp_page -> wp_page_copy 热点集中在缺页异常处理路径exc_page_fault到handle_mm_fault到wp_page_copy

### 为什么矩阵乘法是扁平状
用户态计算为主CPU时间主要在用户态执行浮点运算 很少触发缺页异常数据工作集较小能放入Cache 不进入内核没有系统调用和缺页异常 多个函数调用路径时间分散在多个用户态函数 结论计算密集型负载的CPU时间分布在多个用户态函数中火焰图呈现扁平状

### 为什么随机访存是尖塔状
频繁缺页异常随机地址访问导致大量Page Fault 深度内核调用栈exc_page_fault到handle_mm_fault到wp_page_copy 热点高度集中所有时间都花在同一条内核路径上 用户态时间短大部分时间在处理缺页异常 结论访存密集型负载的CPU时间集中在缺页异常处理路径上火焰图呈现尖塔状

## 内核态函数分析

### 出现的内核函数
entry_SYSCALL_64 系统调用入口 用户态到内核态切换
asm_exc_page_fault 缺页异常入口 汇编级异常处理
exc_page_fault 缺页异常处理 缺页异常核心处理
do_user_addr_fault 地址检查 验证用户地址合法性
handle_mm_fault 缺页处理 处理内存缺页
handle_pte_fault 页表处理 遍历页表层级
do_wp_page 写时复制 处理写时复制页面
wp_page_copy 页面复制 复制物理页面4KB

### 为什么会出现这些内核函数
随机访存负载每次访问随机地址可能导致地址未映射触发缺页异常 页表不存在handle_mm_fault分配物理页 写时复制wp_page_copy复制页面

### 对性能的影响
流水线停顿缺页异常导致CPU流水线清空 上下文切换用户态内核态切换消耗时间 内存分配分配物理页需要时间 页面复制复制4KB页面消耗内存带宽
量化影响来自perf stat数据 用户态时间14.92秒 内核态时间15.16秒 内核态占比超过百分之五十

## 总结
计算密集型负载CPU时间主要花在用户态计算上火焰图呈扁平状 访存密集型负载CPU时间集中在缺页异常处理路径火焰图呈尖塔状 火焰图形状反映瓶颈位置尖塔代表瓶颈明确扁平代表热点分散

## 环境限制说明
本实验在VMware虚拟机中运行火焰图生成功能正常perf record调用栈采样在虚拟机中正常工作
