import matplotlib.pyplot as plt
import numpy as np

strides = [1, 2, 4, 8, 16, 32, 64, 128, 256]
latency = [0.33, 0.35, 0.41, 0.61, 0.93, 1.55, 2.62, 3.90, 4.66]

plt.figure(figsize=(10, 6))

# 绘制曲线
plt.plot(strides, latency, 'bo-', linewidth=2, markersize=10, label='Measured Latency')

# 标注Cache Line边界
plt.axvline(x=64, color='red', linestyle='--', linewidth=2, label='Cache Line Boundary (64B)')

# 添加拐点箭头标注
plt.annotate('↗ Cache Line Boundary\n  Performance Drop', 
             xy=(64, 2.62), xytext=(80, 3.5),
             arrowprops=dict(arrowstyle='->', color='red', lw=1.5),
             fontsize=11, color='red')

# 添加数据标签
for i, (x, y) in enumerate(zip(strides, latency)):
    plt.annotate(f'{y:.2f}ns', (x, y), textcoords="offset points", 
                 xytext=(0, 10), ha='center', fontsize=9)

plt.xlabel('Stride (Bytes)', fontsize=13)
plt.ylabel('Average Access Latency (ns)', fontsize=13)
plt.title('Stride vs Memory Access Latency', fontsize=14, fontweight='bold')
plt.xscale('log')
plt.grid(True, alpha=0.3, linestyle='--')
plt.legend(fontsize=11, loc='upper left')
plt.tight_layout()

plt.savefig('cache_performance_optimized.png', dpi=300, bbox_inches='tight')
print("✅ 优化版曲线图已生成: cache_performance_optimized.png")
