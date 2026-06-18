import matplotlib.pyplot as plt

strides = [1, 2, 4, 8, 16, 32, 64, 128, 256]
latency = [0.33, 0.35, 0.41, 0.61, 0.93, 1.55, 2.62, 3.90, 4.66]

plt.figure(figsize=(10, 6))
plt.plot(strides, latency, 'bo-', linewidth=2, markersize=10)

# 标注Cache Line边界
plt.axvline(x=64, color='red', linestyle='--', linewidth=2, label='Cache Line Boundary (64B)')

# 添加数据标签
for i, (x, y) in enumerate(zip(strides, latency)):
    plt.annotate(f'{y:.2f}ns', (x, y), textcoords="offset points", xytext=(0, 10), ha='center', fontsize=9)

plt.xlabel('Stride (Bytes)', fontsize=12)
plt.ylabel('Average Access Latency (ns)', fontsize=12)
plt.title('Stride vs Memory Access Latency', fontsize=14)
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.legend(fontsize=11)
plt.tight_layout()

plt.savefig('cache_performance_english.png', dpi=300, bbox_inches='tight')
print("✅ English version saved: cache_performance_english.png")
