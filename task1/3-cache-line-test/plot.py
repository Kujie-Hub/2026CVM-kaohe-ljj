import matplotlib.pyplot as plt

strides = [1, 2, 4, 8, 16, 32, 64, 128, 256]
latency = [0.33, 0.35, 0.41, 0.61, 0.93, 1.55, 2.62, 3.90, 4.66]

plt.figure(figsize=(10, 6))
plt.plot(strides, latency, 'bo-', linewidth=2, markersize=8)
plt.axvline(x=64, color='red', linestyle='--', linewidth=2, label='Cache Line边界 (64B)')
plt.xlabel('步长 (字节)', fontsize=12)
plt.ylabel('平均访问时间 (ns)', fontsize=12)
plt.title('步长 vs 内存访问延迟', fontsize=14)
plt.xscale('log')
plt.grid(True, alpha=0.3)
plt.legend(fontsize=12)
plt.savefig('cache_performance.png', dpi=300)
print("✅ 曲线图已生成: cache_performance.png")
