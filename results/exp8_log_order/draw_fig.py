datasets = ['OpenStack', 'Spark', 'Zookeeper']
methods = ['restructured', 'shuffled', 'original']

import csv
import os
from typing import Dict, List

import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import pandas as pd

# Global font size variable for consistent sizing
FONT_SIZE = 29

# Set font to Times New Roman and font size to 20
plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['font.size'] = FONT_SIZE


# 统一字体和样式设置（与motivation.py保持一致）
plt.style.use('default')
sns.set_style("white")
plt.rcParams['font.family'] = 'Times New Roman'


def get_project_paths() -> Dict[str, str]:
    base_dir = os.path.dirname(os.path.abspath(__file__))
    datasets_dir = os.path.join(base_dir, 'test_dataset')
    result_dir = os.path.join(base_dir, 'result_reorder')
    return {
        'base': base_dir,
        'datasets': datasets_dir,
        'result': result_dir,
        'time_csv': os.path.join(result_dir, 'time_cost.csv'),
    }


def get_file_size_bytes(file_path: str) -> int:
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f'File not found: {file_path}')
    return os.path.getsize(file_path)


def collect_original_sizes_bytes(datasets_dir: str, dataset_names: List[str]) -> Dict[str, int]:
    original_sizes: Dict[str, int] = {}
    for dataset_name in dataset_names:
        log_path = os.path.join(datasets_dir, f'{dataset_name}.log')
        original_sizes[dataset_name] = get_file_size_bytes(log_path)
    return original_sizes


def collect_compressed_sizes_bytes(result_dir: str, dataset_names: List[str]) -> Dict[str, Dict[str, int]]:
    compressed_sizes: Dict[str, Dict[str, int]] = {d: {} for d in dataset_names}
    
    # All datasets now have all three methods: original, restructured, and shuffled
    for dataset_name in dataset_names:
        compressed_sizes[dataset_name]['original'] = get_file_size_bytes(os.path.join(result_dir, f'{dataset_name}.lzma'))
        compressed_sizes[dataset_name]['restructured'] = get_file_size_bytes(os.path.join(result_dir, f'{dataset_name}_restructured.lzma'))
        compressed_sizes[dataset_name]['shuffled'] = get_file_size_bytes(os.path.join(result_dir, f'{dataset_name}_shuffled.lzma'))
    
    return compressed_sizes


def read_time_seconds(time_csv_path: str) -> Dict[str, Dict[str, float]]:
    times: Dict[str, Dict[str, float]] = {}
    if not os.path.isfile(time_csv_path):
        raise FileNotFoundError(f'Time file not found: {time_csv_path}')
    
    with open(time_csv_path, 'r', newline='') as f:
        reader = csv.DictReader(f)
        for row in reader:
            dataset_name = row['Dataset']
            time_value = float(row['Approx'])
            
            # Parse dataset name and method
            if dataset_name == 'OpenStack':
                times['OpenStack'] = {'original': time_value}
            elif dataset_name == 'OpenStack_restructured':
                if 'OpenStack' not in times:
                    times['OpenStack'] = {}
                times['OpenStack']['restructured'] = time_value
            elif dataset_name == 'OpenStack_shuffled':
                if 'OpenStack' not in times:
                    times['OpenStack'] = {}
                times['OpenStack']['shuffled'] = time_value
            elif dataset_name == 'Spark':
                times['Spark'] = {'original': time_value}
            elif dataset_name == 'Spark_restructured':
                if 'Spark' not in times:
                    times['Spark'] = {}
                times['Spark']['restructured'] = time_value
            elif dataset_name == 'Spark_shuffled':
                if 'Spark' not in times:
                    times['Spark'] = {}
                times['Spark']['shuffled'] = time_value
            elif dataset_name == 'Zookeeper':
                times['Zookeeper'] = {'original': time_value}
            elif dataset_name == 'Zookeeper_restructured':
                if 'Zookeeper' not in times:
                    times['Zookeeper'] = {}
                times['Zookeeper']['restructured'] = time_value
            elif dataset_name == 'Zookeeper_shuffled':
                if 'Zookeeper' not in times:
                    times['Zookeeper'] = {}
                times['Zookeeper']['shuffled'] = time_value
    
    return times


def compute_compression_ratios(
    original_bytes: Dict[str, int],
    compressed_bytes: Dict[str, Dict[str, int]],
    dataset_names: List[str],
) -> Dict[str, Dict[str, float]]:
    ratios: Dict[str, Dict[str, float]] = {d: {} for d in dataset_names}
    for dataset_name in dataset_names:
        orig = float(original_bytes[dataset_name])
        for method_name, comp_size in compressed_bytes[dataset_name].items():
            ratios[dataset_name][method_name] = orig / comp_size if comp_size > 0 else float('nan')
    return ratios


def compute_speeds_mb_per_s(
    original_bytes: Dict[str, int],
    time_seconds: Dict[str, Dict[str, float]],
    dataset_names: List[str],
) -> Dict[str, Dict[str, float]]:
    speeds: Dict[str, Dict[str, float]] = {d: {} for d in dataset_names}
    for dataset_name in dataset_names:
        size_mb = original_bytes[dataset_name] / 1_000_000.0  # Convert to MB
        for method_name, t in time_seconds[dataset_name].items():
            speeds[dataset_name][method_name] = (size_mb / t) if t > 0 else float('nan')
    return speeds


def plot_bars(ax, dataset_names: List[str], y_by_dataset_and_method: Dict[str, Dict[str, float]], title: str, ylabel: str, logy: bool = False, show_legend: bool = True, pad: int = 10) -> None:
    # 使用与motivation.py相同的颜色配置
    colors = ['#1f77b4', '#2ca02c', '#d62728']  # Blue, Green, Red
    methods = ['shuffled', 'original', 'restructured']
    
    # 使用matplotlib的bar绘制，完全仿照motivation.py的方式
    x = np.arange(len(dataset_names))
    width = 0.25
    
    for i, method in enumerate(methods):
        values = []
        for dataset in dataset_names:
            if method in y_by_dataset_and_method[dataset]:
                value = y_by_dataset_and_method[dataset][method]
                values.append(value if value > 0 else 0)
            else:
                values.append(0)
        
        # 修改标签显示
        label = method.capitalize()
        if method == 'restructured':
            label = 'Reordered'
        ax.bar(x + i*width, values, width, label=label, color=colors[i])
    
    ax.set_xticks(x + width)
    ax.set_xticklabels(dataset_names, rotation=15, fontfamily='Times New Roman', fontsize=FONT_SIZE)
    ax.set_ylabel(ylabel, fontweight='bold', fontfamily='Times New Roman', fontsize=FONT_SIZE)
    ax.set_xlabel('Dataset', fontweight='bold', fontfamily='Times New Roman', fontsize=FONT_SIZE)
    ax.set_title(title, fontweight='bold', fontfamily='Times New Roman', fontsize=FONT_SIZE, pad=pad)
    
    # 设置y轴刻度标签字体
    for label in ax.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(FONT_SIZE)
    
    if not show_legend:
        ax.legend().remove()
    else:
        # 设置图例字体
        legend = ax.get_legend()
        if legend:
            plt.setp(legend.get_texts(), fontfamily='Times New Roman', fontsize=FONT_SIZE)
    
    if logy:
        ax.set_yscale('log')


def main():
    paths = get_project_paths()
    original_bytes = collect_original_sizes_bytes(paths['datasets'], datasets)
    compressed_bytes = collect_compressed_sizes_bytes(paths['result'], datasets)
    time_seconds = read_time_seconds(paths['time_csv'])

    ratios = compute_compression_ratios(original_bytes, compressed_bytes, datasets)
    speeds = compute_speeds_mb_per_s(original_bytes, time_seconds, datasets)

    fig, (ax1, ax2) = plt.subplots(nrows=1, ncols=2, figsize=(12, 7))
    plot_bars(ax1, datasets, ratios, title='(a) Compression Ratio', ylabel='Comp. Ratio', logy=False, show_legend=False, pad=10)
    plot_bars(ax2, datasets, speeds, title='(b) Compression Speed', ylabel='Comp. Speed (MB/s)', logy=False, show_legend=False, pad=10)
    
    # Set compression ratio y-axis ticks
    ax1.set_yticks([0, 10, 20, 30, 40])
    
    # Set speed y-axis ticks
    ax2.set_yticks([0, 1, 2, 3, 4])
    
    # Add single legend above both subplots
    handles, labels = ax1.get_legend_handles_labels()
    legend = fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=3, fontsize=FONT_SIZE)
    # Set legend font to Times New Roman
    for text in legend.get_texts():
        text.set_fontfamily('Times New Roman')
        text.set_fontsize(FONT_SIZE)
    
    # 调整布局，为图例留出更多空间
    fig.tight_layout()
    fig.subplots_adjust(top=0.78)  # 为图例留出适当空间

    # Save as PNG
    output_path_png = os.path.join(paths['base'], 'reorder_summary.png')
    fig.savefig(output_path_png, dpi=200)
    print(f'Figure saved: {output_path_png}')
    
    # Save as EPS
    output_path_eps = os.path.join(paths['base'], 'reorder_summary.eps')
    fig.savefig(output_path_eps, format='eps', dpi=200)
    print(f'Figure saved: {output_path_eps}')


if __name__ == '__main__':
    main()