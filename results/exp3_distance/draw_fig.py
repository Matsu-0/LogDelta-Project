import csv
import os
from typing import Dict, List

import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import pandas as pd
from matplotlib.ticker import MaxNLocator
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

# 统一字体和样式设置（与microbenchmark.py保持一致）
plt.style.use('ggplot')
sns.set_theme(style="ticks", palette="pastel")
plt.rcParams['font.family'] = 'Times New Roman'


def get_file_size_bytes(file_path: str) -> int:
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'File not found: {file_path}')
    return os.path.getsize(file_path)


def get_directory_size_bytes(directory_path: str) -> int:
    """计算目录中所有文件的总大小"""
    if not os.path.isdir(directory_path):
        raise FileNotFoundError(f'Directory not found: {directory_path}')
    
    total_size = 0
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            if os.path.isfile(file_path):
                total_size += os.path.getsize(file_path)
    return total_size


def draw_distance_performance():
    """
    绘制distance性能图：横轴为距离阈值（0.0-0.98），压缩比为线图，压缩速度为线图
    支持三种方法：Cosine (红色三角), MinHash (蓝色圆形), QGram (绿色方形)
    只绘制Zookeeper数据集的结果
    """
    # 设置参数范围：从0.0到0.98，每次增加0.02
    distances = [round(i * 0.02, 2) for i in range(50)]  # 0.0, 0.02, 0.04, ..., 0.98
    
    # 文件路径
    base_dir = os.path.dirname(os.path.abspath(__file__))
    original_file = os.path.join(base_dir, 'test_dataset', 'Zookeeper_20000.txt')
    result_dir = os.path.join(base_dir, 'result_distance')
    
    # 三种方法
    methods = ['COSINE', 'MINHASH', 'QGRAM']
    method_labels = ['Cosine', 'MinHash', 'Operation']
    
    # 读取原始文件大小
    original_size = get_file_size_bytes(original_file)
    print(f"原始文件大小: {original_size} bytes")
    
    # 读取三种方法的压缩文件大小和时间
    compressed_sizes = {method: [] for method in methods}
    compression_times = {method: [] for method in methods}
    
    for method in methods:
        method_dir = os.path.join(result_dir, method, 'Zookeeper')
        time_csv_path = os.path.join(result_dir, method, 'time_cost.csv')
        
        # 读取压缩文件大小
        for distance in distances:
            compressed_file = os.path.join(method_dir, f'Zookeeper_{distance:.6f}')
            if os.path.exists(compressed_file):
                compressed_sizes[method].append(get_file_size_bytes(compressed_file))
            else:
                print(f"警告：找不到文件 {compressed_file}")
                compressed_sizes[method].append(0)
        
        # 读取压缩时间
        times = [0] * len(distances)
        if os.path.exists(time_csv_path):
            with open(time_csv_path, 'r', newline='') as f:
                reader = csv.DictReader(f)
                for row in reader:
                    if 'Distance' in row and 'Zookeeper' in row:
                        dist = float(row['Distance'])
                        if dist in distances:
                            dist_index = distances.index(dist)
                            times[dist_index] = float(row['Zookeeper'])
        compression_times[method] = times
    
    # 计算压缩比和压缩速度
    compression_ratios = {method: [] for method in methods}
    compression_speeds = {method: [] for method in methods}
    
    for method in methods:
        for i, distance in enumerate(distances):
            # 计算压缩比
            if original_size > 0 and compressed_sizes[method][i] > 0:
                ratio = original_size / compressed_sizes[method][i]
                compression_ratios[method].append(ratio)
            else:
                compression_ratios[method].append(0)
            
            # 计算压缩速度 (MB/s) = 源文件大小(MB) / 压缩时间(s)
            original_size_mb = original_size / (1024 * 1024)  # 转换为MB
            if compression_times[method][i] > 0:
                speed = original_size_mb / compression_times[method][i]
                compression_speeds[method].append(speed)
            else:
                compression_speeds[method].append(0)
    
    # 创建图形 - 两个子图
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(12, 6.5))
    
    # 配色设置 - QGram与LogDelta保持一致
    colors = ['#1f77b4', '#2ca02c', '#d62728']  # Blue, Green, Red
    markers = ['o', 's', '^']  # Circle, Square, Triangle
    
    # 绘制压缩比（ax1）和压缩速度（ax2）
    for i, method in enumerate(methods):
        # 绘制压缩比线条
        ax1.plot(distances, compression_ratios[method], color=colors[i], 
                 linestyle='-', linewidth=4, label=method_labels[i], zorder=2*i)
        
        # 绘制压缩比标记点（每4个点显示一次）
        marker_positions = list(range(0, len(distances), 4))  # 每4格显示一次
        ax1.plot([distances[j] for j in marker_positions], 
                 [compression_ratios[method][j] for j in marker_positions], 
                 color=colors[i], marker=markers[i], linestyle='', 
                 markersize=8, zorder=2*i+1)
        
        # 绘制压缩速度线条
        ax2.plot(distances, compression_speeds[method], color=colors[i], 
                 linestyle='-', linewidth=4, label=method_labels[i], zorder=10+i)
        
        # 绘制压缩速度标记点（每4个点显示一次）
        ax2.plot([distances[j] for j in marker_positions], 
                 [compression_speeds[method][j] for j in marker_positions], 
                 color=colors[i], marker=markers[i], linestyle='', 
                 markersize=8, zorder=20+i)
    
    # 设置字体大小（统一使用一个size）
    size = 29
    
    # 设置坐标轴
    ax1.set_xlabel('Distance Threshold', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax1.set_ylabel('Comp. Ratio', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax2.set_xlabel('Distance Threshold', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax2.set_ylabel('Comp. Speed (MB/s)', fontsize=size, fontfamily='Times New Roman', fontweight='bold')

    # 设置子图标题
    ax1.set_title('(a) Compression Ratio', fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
    ax2.set_title('(b) Compression Speed', fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
    
    # 设置x轴刻度 - 从0到1.0，每0.2出现一次
    x_ticks = [0.0, 0.2, 0.4, 0.6, 0.8, 1.0]
    ax1.set_xticks(x_ticks)
    ax2.set_xticks(x_ticks)
    
    # 强制设置所有y轴tick和label字体和大小（统一使用size）
    for label in ax1.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    for label in ax2.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    if ax1.yaxis.label is not None:
        ax1.yaxis.label.set_fontfamily('Times New Roman')
        ax1.yaxis.label.set_fontsize(size)
    if ax2.yaxis.label is not None:
        ax2.yaxis.label.set_fontfamily('Times New Roman')
        ax2.yaxis.label.set_fontsize(size)
    
    # 强制设置所有x轴tick字体和大小（统一使用size，不加粗）
    for label in ax1.get_xticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    for label in ax2.get_xticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    
    # 使用tick_params强制设置字体大小
    ax1.tick_params(axis='x', labelsize=size)
    ax1.tick_params(axis='y', labelsize=size)
    ax2.tick_params(axis='x', labelsize=size)
    ax2.tick_params(axis='y', labelsize=size)
    
    # 设置压缩比纵轴为线性刻度（ax1绘制压缩比）
    ax1.set_yscale('linear')
    # 设置速度纵轴为对数刻度（ax2绘制速度）
    ax2.set_yscale('log')
    
    # 设置压缩比线性刻度的范围（ax1绘制压缩比）
    all_ratios = []
    for method in methods:
        all_ratios.extend(compression_ratios[method])
    if all_ratios:
        min_ratio = min([r for r in all_ratios if r > 0])  # 找到最小正值
        max_ratio = max(all_ratios)
        # 设置范围，不从0开始，而是从最小值的90%开始
        ax1.set_ylim(min_ratio * 0.9, max_ratio * 1.1)  # 线性刻度范围
    
    # 设置速度对数刻度的范围（ax2绘制速度）
    all_speeds = []
    for method in methods:
        all_speeds.extend(compression_speeds[method])
    if all_speeds:
        min_speed = min([s for s in all_speeds if s > 0])  # 找到最小正值
        max_speed = max(all_speeds)
        ax2.set_ylim(min_speed * 0.2, max_speed * 5)  # 对数刻度范围
    
    # 不设置网格
    
    # 创建自定义图例
    custom_lines = [
        Line2D([0], [0], color=colors[0], marker=markers[0], linestyle='-', linewidth=4, markersize=8, label=method_labels[0]),
        Line2D([0], [0], color=colors[1], marker=markers[1], linestyle='-', linewidth=4, markersize=8, label=method_labels[1]),
        Line2D([0], [0], color=colors[2], marker=markers[2], linestyle='-', linewidth=4, markersize=8, label=method_labels[2]),
    ]
    
    # 添加图例，放在图的上方（仿照microbenchmark.py的设置）
    legend = fig.legend(handles=custom_lines, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=3, fontsize=size)
    # 设置图例字体
    for text in legend.get_texts():
        text.set_fontfamily('Times New Roman')
        text.set_fontsize(size)
    
    # 调整布局，为图例留出更多空间
    fig.tight_layout()
    fig.subplots_adjust(top=0.75)  # 为图例留出更多空间
    
    # 保存图形到当前目录
    output_path_png = 'distance_performance.png'
    output_path_eps = 'distance_performance.eps'
    
    fig.savefig(output_path_png, dpi=200, bbox_inches='tight')
    fig.savefig(output_path_eps, format='eps', dpi=200, bbox_inches='tight')
    
    print(f'Distance性能图已保存: {output_path_png}')
    print(f'Distance性能图已保存: {output_path_eps}')
    
    # 显示图形
    plt.show()


def main():
    # 绘制distance性能图
    print("绘制distance性能图...")
    draw_distance_performance()


if __name__ == '__main__':
    main()

