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

# 统一字体和样式设置（与minibranch.py保持一致）
plt.style.use('ggplot')
sns.set_theme(style="ticks", palette="pastel")
plt.rcParams['font.family'] = 'Times New Roman'


def get_file_size_bytes(file_path: str) -> int:
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'File not found: {file_path}')
    return os.path.getsize(file_path)


def draw_microbenchmark_performance():
    """
    绘制microbenchmark性能图：横轴为参数n（3-20），压缩比为线图，压缩时间为背景填充
    支持两种方法：LogDelta-Approx (红色三角) 和 LogDelta-Exact (蓝色圆形)
    """
    # 设置参数范围
    parameters = list(range(3, 82, 2))  # 3到20
    
    # 文件路径
    base_dir = os.path.dirname(os.path.abspath(__file__))
    datasets_dir = os.path.join(base_dir, 'test_dataset')
    result_approx_dir = os.path.join(base_dir, 'result_microbenchmark', 'result_approx')
    result_exact_dir = os.path.join(base_dir, 'result_microbenchmark', 'result_exact')
    
    # 读取原始文件大小
    original_sizes = []
    for param in parameters:
        file_path = os.path.join(datasets_dir, f'Artificial_{param}.log')
        if os.path.exists(file_path):
            original_sizes.append(get_file_size_bytes(file_path))
        else:
            print(f"警告：找不到文件 {file_path}")
            original_sizes.append(0)
    
    # 读取两种方法的压缩文件大小
    compressed_sizes_approx = []
    compressed_sizes_exact = []
    for param in parameters:
        # Approx方法
        approx_path = os.path.join(result_approx_dir, f'Artificial_{param}.zstd')
        if os.path.exists(approx_path):
            compressed_sizes_approx.append(get_file_size_bytes(approx_path))
        else:
            print(f"警告：找不到文件 {approx_path}")
            compressed_sizes_approx.append(0)
        
        # Exact方法
        exact_path = os.path.join(result_exact_dir, f'Artificial_{param}.zstd')
        if os.path.exists(exact_path):
            compressed_sizes_exact.append(get_file_size_bytes(exact_path))
        else:
            print(f"警告：找不到文件 {exact_path}")
            compressed_sizes_exact.append(0)
    
    # 读取两种方法的时间数据
    time_csv_approx_path = os.path.join(result_approx_dir, 'compression_timing.csv')
    time_csv_exact_path = os.path.join(result_exact_dir, 'compression_timing.csv')
    
    times_approx = [0] * len(parameters)
    times_exact = [0] * len(parameters)
    
    # 读取approx时间
    if os.path.exists(time_csv_approx_path):
        with open(time_csv_approx_path, 'r', newline='') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if 'n' in row and 'Compression_Time' in row:
                    param = int(row['n'])
                    if param in parameters:
                        param_index = parameters.index(param)
                        times_approx[param_index] = float(row['Compression_Time'])
    
    # 读取exact时间
    if os.path.exists(time_csv_exact_path):
        with open(time_csv_exact_path, 'r', newline='') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if 'n' in row and 'Compression_Time' in row:
                    param = int(row['n'])
                    if param in parameters:
                        param_index = parameters.index(param)
                        times_exact[param_index] = float(row['Compression_Time'])
    
    # 计算两种方法的压缩比和时间
    compression_ratios_approx = []
    compression_times_approx = []
    compression_ratios_exact = []
    compression_times_exact = []
    
    for i, param in enumerate(parameters):
        # Approx方法
        if original_sizes[i] > 0 and compressed_sizes_approx[i] > 0:
            ratio = original_sizes[i] / compressed_sizes_approx[i]
            compression_ratios_approx.append(ratio)
        else:
            compression_ratios_approx.append(0)
        
        compression_times_approx.append(times_approx[i])
        
        # Exact方法
        if original_sizes[i] > 0 and compressed_sizes_exact[i] > 0:
            ratio = original_sizes[i] / compressed_sizes_exact[i]
            compression_ratios_exact.append(ratio)
        else:
            compression_ratios_exact.append(0)
        
        compression_times_exact.append(times_exact[i])
    
    # 创建图形 - 两个子图
    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(12, 6.5))

    # colors = ['#1f77b4', '#2ca02c', '#d62728']  # Blue, Green, Red
    
    # 配色设置
    # LogDelta-Approx: 红色三角
    approx_color = '#d62728'
    approx_marker = '^'
    
    # LogDelta-Exact: 蓝色圆形
    exact_color = '#1f77b4'
    exact_marker = 'o'
    
    # 绘制压缩比（ax1）
    ax1.plot(parameters, compression_ratios_approx, color=approx_color, marker=approx_marker, 
             linestyle='-', linewidth=4, markersize=8, label='LogDelta-Approx', zorder=10)
    ax1.plot(parameters, compression_ratios_exact, color=exact_color, marker=exact_marker, 
             linestyle='-', linewidth=4, markersize=8, label='LogDelta-Exact', zorder=10)
    
    # 绘制压缩时间（ax2）
    ax2.plot(parameters, compression_times_approx, color=approx_color, marker=approx_marker, 
             linestyle='-', linewidth=4, markersize=8, label='LogDelta-Approx', zorder=10)
    ax2.plot(parameters, compression_times_exact, color=exact_color, marker=exact_marker, 
             linestyle='-', linewidth=4, markersize=8, label='LogDelta-Exact', zorder=10)
    
    # 设置字体大小（统一使用一个size）
    size = 29
    
    # 设置坐标轴
    ax1.set_xlabel('Offset Length', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax1.set_ylabel('Comp. Ratio', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax2.set_xlabel('Offset Length', fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax2.set_ylabel('Comp. Time (s)', fontsize=size, fontfamily='Times New Roman', fontweight='bold')

    # 设置子图标题
    ax1.set_title('(a) Compression Ratio', fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=10)
    ax2.set_title('(b) Compression Speed', fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=10)
    
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
    
    # 设置x轴刻度 - 显示更多刻度
    # ax1.set_xticks(parameters)  # 显示所有刻度
    # ax2.set_xticks(parameters)  # 显示所有刻度
    
    # 使用tick_params强制设置字体大小
    ax1.tick_params(axis='x', labelsize=size)
    ax1.tick_params(axis='y', labelsize=size)
    ax2.tick_params(axis='x', labelsize=size)
    ax2.tick_params(axis='y', labelsize=size)
    
    # 设置压缩比纵轴为线性刻度（ax1绘制压缩比）
    ax1.set_yscale('linear')
    # 设置时间纵轴为对数刻度（ax2绘制时间）
    ax2.set_yscale('log')
    
    # 设置压缩比线性刻度的范围（ax1绘制压缩比）
    all_ratios = compression_ratios_approx + compression_ratios_exact
    if all_ratios:
        min_ratio = min([r for r in all_ratios if r > 0])  # 找到最小正值
        max_ratio = max(all_ratios)
        ax1.set_ylim(0, max_ratio * 1.2)  # 线性刻度范围
    
    # 设置时间对数刻度的范围（ax2绘制时间）
    all_times = compression_times_approx + compression_times_exact
    if all_times:
        min_time = min([t for t in all_times if t > 0])  # 找到最小正值
        max_time = max(all_times)
        ax2.set_ylim(min_time * 0.01, max_time * 10)  # 对数刻度范围放大
    
    # 不设置网格
    
    # 创建自定义图例
    custom_lines = [
        Line2D([0], [0], color=approx_color, marker=approx_marker, linestyle='-', linewidth=4, markersize=8, label='LogDelta-Approx'),
        Line2D([0], [0], color=exact_color, marker=exact_marker, linestyle='-', linewidth=4, markersize=8, label='LogDelta-Exact'),
    ]
    
    # 添加图例，放在图的上方（仿照reorder.py的设置）
    legend = fig.legend(handles=custom_lines, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=2, fontsize=size)
    # 设置图例字体
    for text in legend.get_texts():
        text.set_fontfamily('Times New Roman')
        text.set_fontsize(size)
    
    # 调整布局，为图例留出更多空间
    fig.tight_layout()
    fig.subplots_adjust(top=0.75)  # 为图例留出适当空间
    
    # 保存图形到当前目录
    output_path_png = 'microbenchmark.png'
    output_path_eps = 'microbenchmark.eps'
    
    fig.savefig(output_path_png, dpi=200, bbox_inches='tight')
    fig.savefig(output_path_eps, format='eps', dpi=200, bbox_inches='tight')
    
    print(f'Microbenchmark性能图已保存: {output_path_png}')
    print(f'Microbenchmark性能图已保存: {output_path_eps}')
    
    # 显示图形
    plt.show()


def main():
    # 绘制microbenchmark性能图
    print("绘制microbenchmark性能图...")
    draw_microbenchmark_performance()


if __name__ == '__main__':
    main()
 