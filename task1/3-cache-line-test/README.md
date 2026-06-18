# 任务1-③：Cache Line微基准测试

## 运行说明

### 编译
cd src
gcc -O2 -o cache_line_test cache_line_test.c -lm

### 测试全部步长
./cache_line_test

### 测试指定步长用于perf采集
./cache_line_test 64

### perf stat采集所有步长
for stride in 1 2 4 8 16 32 64 128 256; do sudo perf stat -e cpu-clock,task-clock,page-faults ./src/cache_line_test $stride 2>&1 | tee results/stride_${stride}.txt; done

### 生成火焰图stride=1
sudo perf record -F 99 -g -- ./src/cache_line_test 1
sudo perf script > stride1.script
~/FlameGraph/stackcollapse-perf.pl stride1.script | ~/FlameGraph/flamegraph.pl --title="stride=1火焰图" > flamegraphs/stride1_flame.svg

### 生成火焰图stride=64
sudo perf record -F 99 -g -- ./src/cache_line_test 64
sudo perf script > stride64.script
~/FlameGraph/stackcollapse-perf.pl stride64.script | ~/FlameGraph/flamegraph.pl --title="stride=64火焰图" > flamegraphs/stride64_flame.svg

## 结果文件
src/cache_line_test.c 源代码
results/stride_*.txt 各步长perf stat输出
flamegraphs/stride1_flame.svg stride=1火焰图
flamegraphs/stride64_flame.svg stride=64火焰图
report.md 曲线图加拐点分析加AI使用记录
ai-chat-log/ai_record.md AI辅助编程记录
cache_performance_optimized.png 步长vs性能曲线图

## 测试结果
步长 平均访问时间ns 相对性能
1 0.33 1.00x
2 0.35 1.04x
4 0.41 1.24x
8 0.61 1.83x
16 0.93 2.82x
32 1.55 4.68x
64 2.62 7.90x
128 3.90 11.76x
256 4.66 14.06x

## 拐点分析
拐点位置 stride等于64字节
原因分析 x86_64 CPU的Cache Line大小为64字节 stride小于64时数据在同一个Cache Line内空间局部性好L1命中率高 stride等于64时每次访问恰好跨越Cache Line边界触发额外Cache Miss stride大于64时频繁跨越多个Cache Line Cache Miss率显著上升

## 火焰图对比
stride等于1时热点分散Cache命中率高主要在用户态执行 stride等于64时出现更多Cache Miss处理路径

## AI辅助编程记录
本次实验借助DeepSeek完成 生成C语言测试程序框架 调试编译问题 分析性能数据并生成曲线图 详细对话记录见ai-chat-log/ai_record.md

## 环境限制说明
本实验在VMware虚拟机中运行 perf硬件PMU事件不可用使用软件事件cpu-clock task-clock page-faults替代
