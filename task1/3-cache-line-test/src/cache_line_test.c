/**
 * CPU Cache Line 大小对数组遍历性能的影响测试
 * 
 * 编译: gcc -O2 -D_GNU_SOURCE -o cache_line_test cache_line_test.c -lm
 * 运行: ./cache_line_test
 * 
 * 测试不同步长(stride)对内存访问性能的影响
 * 预期: stride=64 附近出现性能拐点(Cache Line边界)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#define ARRAY_SIZE (32 * 1024 * 1024)  // 32MB，大于L3 Cache
#define ITERATIONS 50                   // 每个步长迭代次数
#define WARMUP 3                        // 预热次数

// 获取高精度时间(微秒)
double get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

// 测试指定步长的访问性能
double test_stride(int stride, unsigned char *array, int array_size) {
    volatile unsigned char sum = 0;
    double start, end;
    int i, pos;
    long total_accesses = 0;
    
    // 预热
    for (i = 0; i < WARMUP; i++) {
        pos = 0;
        while (pos < array_size) {
            sum += array[pos];
            pos += stride;
            total_accesses++;
        }
    }
    
    // 正式测试
    total_accesses = 0;
    start = get_time_us();
    
    for (i = 0; i < ITERATIONS; i++) {
        pos = 0;
        while (pos < array_size) {
            sum += array[pos];
            pos += stride;
            total_accesses++;
        }
    }
    
    end = get_time_us();
    
    // 计算平均访问时间(纳秒)
    double total_us = end - start;
    double avg_ns = (total_us * 1000.0) / total_accesses;
    
    return avg_ns;
}

int main(int argc, char *argv[]) {
    unsigned char *array;
    int strides[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
    int num_strides = sizeof(strides) / sizeof(strides[0]);
    int i, j;
    double result;
    int target_stride = -1;
    
    // 如果命令行指定了步长，只测试该步长
    if (argc > 1) {
        target_stride = atoi(argv[1]);
    }
    
    // 分配大数组
    printf("分配 %d MB 内存...\n", ARRAY_SIZE / (1024 * 1024));
    array = (unsigned char *)malloc(ARRAY_SIZE);
    if (array == NULL) {
        printf("内存分配失败!\n");
        return 1;
    }
    
    // 初始化数组(防止缺页异常干扰)
    printf("初始化数组...\n");
    memset(array, 0xAA, ARRAY_SIZE);
    
    printf("\n============================================================\n");
    printf("步长(字节) | 平均访问时间(ns) | 相对性能\n");
    printf("============================================================\n");
    
    double base_time = 0;
    int first = 1;
    
    // 测试每个步长
    for (i = 0; i < num_strides; i++) {
        int stride = strides[i];
        
        // 如果指定了目标步长，跳过其他
        if (target_stride != -1 && stride != target_stride) {
            continue;
        }
        
        // 多次测试取平均
        double total_time = 0;
        int samples = 3;
        
        for (j = 0; j < samples; j++) {
            result = test_stride(stride, array, ARRAY_SIZE);
            total_time += result;
        }
        double avg_time = total_time / samples;
        
        if (first) {
            base_time = avg_time;
            first = 0;
        }
        
        printf("%4d         | %14.2f       | %.2fx\n", 
               stride, avg_time, avg_time / base_time);
        
        fflush(stdout);
    }
    
    printf("============================================================\n");
    printf("\n提示: 在 stride=64 附近应该出现性能拐点(Cache Line边界)\n");
    printf("因为典型x86_64 CPU的Cache Line大小为64字节\n");
    
    free(array);
    return 0;
}
