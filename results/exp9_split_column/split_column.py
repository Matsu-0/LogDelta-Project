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

# 统一字体和样式设置（与reorder.py保持一致）
plt.style.use('default')
sns.set_style("white")
plt.rcParams['font.family'] = 'Times New Roman'

# 数据集列表（10个数据集）
datasets = ['Android', 'Apache', 'Linux', 'Mac', 'OpenStack', 'Proxifier', 'Spark', 'SSH', 'Thunderbird', 'Zookeeper']
methods = ['LogDelta', 'IoTDB']


def get_project_paths() -> Dict[str, str]:
    base_dir = os.path.dirname(os.path.abspath(__file__))
    return {
        'base': base_dir,
        'LogDelta': os.path.join(base_dir, 'LogDelta'),
        'IoTDB': os.path.join(base_dir, 'IoTDB'),
        'original': '/Users/matsu/Desktop/result_revision/compressor/test_dataset',
    }


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


def collect_original_sizes_bytes(original_dir: str, dataset_names: List[str]) -> Dict[str, int]:
    """收集原始数据集文件大小"""
    original_sizes: Dict[str, int] = {}
    for dataset_name in dataset_names:
        log_path = os.path.join(original_dir, f'{dataset_name}.log')
        original_sizes[dataset_name] = get_file_size_bytes(log_path)
    return original_sizes


def collect_compressed_sizes_bytes(logdelta_dir: str, iotdb_dir: str, dataset_names: List[str]) -> Dict[str, Dict[str, int]]:
    """收集压缩后文件大小"""
    compressed_sizes: Dict[str, Dict[str, int]] = {d: {} for d in dataset_names}
    
    for dataset_name in dataset_names:
        # LogDelta压缩文件大小
        logdelta_path = os.path.join(logdelta_dir, f'{dataset_name}.lzma')
        compressed_sizes[dataset_name]['LogDelta'] = get_file_size_bytes(logdelta_path)
        
        # IoTDB存储大小（整个数据集目录）
        iotdb_path = os.path.join(iotdb_dir, f'root.{dataset_name}')
        compressed_sizes[dataset_name]['IoTDB'] = get_directory_size_bytes(iotdb_path)
    
    return compressed_sizes


def compute_compression_ratios(
    original_bytes: Dict[str, int],
    compressed_bytes: Dict[str, Dict[str, int]],
    dataset_names: List[str],
    method_names: List[str],
) -> Dict[str, Dict[str, float]]:
    """计算压缩比"""
    ratios: Dict[str, Dict[str, float]] = {d: {} for d in dataset_names}
    for dataset_name in dataset_names:
        orig = float(original_bytes[dataset_name])
        for method_name in method_names:
            comp = float(compressed_bytes[dataset_name][method_name])
            ratios[dataset_name][method_name] = orig / comp if comp > 0 else float('nan')
    return ratios


def plot_compression_ratio_bars(ax, dataset_names: List[str], y_by_dataset_and_method: Dict[str, Dict[str, float]], 
                               ylabel: str, size: int = 20) -> None:
    """绘制压缩比柱状图"""
    # 使用与reorder.py相同的颜色配置
    colors = ['#1f77b4', '#d62728']  # Blue, Red
    methods = ['LogDelta', 'IoTDB']
    
    # 使用matplotlib的bar绘制，完全仿照reorder.py的方式
    x = np.arange(len(dataset_names))
    width = 0.35
    
    for i, method in enumerate(methods):
        values = []
        for dataset in dataset_names:
            if method in y_by_dataset_and_method[dataset]:
                value = y_by_dataset_and_method[dataset][method]
                values.append(value if value > 0 else 0)
            else:
                values.append(0)
        
        method_label = 'LogDelta' if method == 'LogDelta' else 'IoTDB Storage'
        ax.bar(x + i*width, values, width, label=method_label, color=colors[i])
    
    ax.set_xticks(x + width/2)
    ax.set_xticklabels(dataset_names, rotation=20, fontfamily='Times New Roman', fontsize=size)
    ax.set_ylabel(ylabel, fontweight='bold', fontfamily='Times New Roman', fontsize=size)
    ax.set_xlabel('Dataset', fontweight='bold', fontfamily='Times New Roman', fontsize=size)
    
    # 设置y轴刻度标签字体
    for label in ax.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    
    # 设置图例字体并确保一行显示
    legend = ax.get_legend()
    if legend:
        plt.setp(legend.get_texts(), fontfamily='Times New Roman', fontsize=size)
    else:
        # 如果没有图例，创建一个并设置为一行
        ax.legend(ncol=2, fontsize=size)
        legend = ax.get_legend()
        plt.setp(legend.get_texts(), fontfamily='Times New Roman', fontsize=size)


def draw_split_column_comparison():
    """绘制分列与未分列的压缩比对比图"""
    paths = get_project_paths()
    size = 29
    
    print("Collecting original file sizes...")
    original_bytes = collect_original_sizes_bytes(paths['original'], datasets)
    
    print("Collecting compressed file sizes...")
    compressed_bytes = collect_compressed_sizes_bytes(paths['LogDelta'], paths['IoTDB'], datasets)
    
    print("Computing compression ratios...")
    ratios = compute_compression_ratios(original_bytes, compressed_bytes, datasets, methods)
    
    # 打印数据用于调试
    print("\nCompression Ratios:")
    for dataset in datasets:
        print(f"{dataset}: LogDelta = {ratios[dataset]['LogDelta']:.2f}, IoTDB = {ratios[dataset]['IoTDB']:.2f}")
    
    # 创建图形
    fig, ax = plt.subplots(figsize=(12, 6))
    plot_compression_ratio_bars(ax, datasets, ratios, 
                               ylabel='Comp. Ratio', size=size)
    
    # 调整布局
    fig.tight_layout()
    
    # 保存图形
    output_path_png = 'split_column_comparison.png'
    output_path_eps = 'split_column_comparison.eps'
    
    fig.savefig(output_path_png, dpi=200, bbox_inches='tight')
    fig.savefig(output_path_eps, format='eps', dpi=200, bbox_inches='tight')
    
    print(f'\nFigure saved: {output_path_png}')
    print(f'Figure saved: {output_path_eps}')
    
    # 显示图形
    plt.show()


def main():
    print("Drawing split column comparison...")
    draw_split_column_comparison()


if __name__ == '__main__':
    main()
