# Cache Line 微基准测试报告

## 一、测试目的
验证CPU Cache Line大小对数组遍历性能的影响

## 二、测试环境
CPU 13th Gen Intel(R) Core(TM) i7-13620H Raptor Lake
vCPU 2核
虚拟化 VMware
内核 Linux 6.8.0-124-generic
数组大小 32MB
步长范围 1 2 4 8 16 32 64 128 256 字节

## 三、测试结果
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

## 四、拐点分析
拐点位置 stride等于64字节
原因分析 x86_64 CPU的Cache Line大小为64字节 stride小于64时数据在同一个Cache Line内空间局部性好L1命中率高 stride等于64时每次访问恰好跨越Cache Line边界触发额外Cache Miss stride大于64时频繁跨越多个Cache Line Cache Miss率显著上升

## 五、火焰图对比
stride等于1时热点分散Cache命中率高主要在用户态执行 stride等于64时出现更多Cache Miss处理路径

## 六、AI辅助编程记录
本次实验借助DeepSeek完成
1 生成C语言测试程序框架
2 调试编译问题
3 分析性能数据并生成曲线图
详细对话记录见ai-chat-log/ai_record.md

## 七、曲线图
曲线图文件 cache_performance_optimized.png

## 八、环境限制说明
本实验在VMware虚拟机中运行 perf硬件PMU事件不可用使用软件事件cpu-clock task-clock page-faults替代

## 九、结论
Cache Line大小为64字节 stride等于64时出现性能拐点 步长超过Cache Line大小时性能显著下降
