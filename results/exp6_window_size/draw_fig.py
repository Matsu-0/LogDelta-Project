import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
from matplotlib.ticker import FormatStrFormatter, MaxNLocator
import numpy as np
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

plt.style.use('ggplot')
sns.set_theme(style="ticks", palette="pastel")
plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['mathtext.fontset'] = 'stix'  # for math font to match Times New Roman


def draw_window_size_comparison():
    """绘制窗口大小对压缩比的影响对比图（无图例）"""
    size = 29
    figsize = (6, 6.3)  # 恢复图像大小
    fig, ax = plt.subplots(figsize=figsize)

    # 配色和marker
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', 
              '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']
    markers = ['o', 's', '^', 'v', 'D', 'p', '*', 'h', 'H', 'X']
    
    # 窗口大小值（按2的幂次递增）
    window_sizes = [1, 2, 4, 8, 16, 32, 64, 128, 256]
    
    dataset_list = [
        "Zookeeper", "OpenStack", "Spark", "Linux",
        "Mac", "Thunderbird", "Apache",
        "SSH", "Proxifier", "Android"
    ]
    file_name = [
        "Zookeeper", "OpenStack", "Spark", "Linux",
        "Mac", "Thunderbird", "Apache",
        "SSH", "Proxifier", "Android"
    ]
    
    for index in range(len(dataset_list)):
        original_file = f'./test_dataset/{file_name[index]}.log'
        original_size = os.path.getsize(original_file)
        
        ratios = []
        for window_size in window_sizes:
            compressed_file = f'./result_window_size/{file_name[index]}/{file_name[index]}_{window_size}.lzma'
            if os.path.exists(compressed_file):
                compressed_size = os.path.getsize(compressed_file)
                ratio = original_size / compressed_size if compressed_size > 0 else np.nan
                ratios.append(ratio)
            else:
                ratios.append(np.nan)
        
        x = np.array(window_sizes)
        y_ratio = np.array(ratios)
        
        # 绘制折线图 - 加粗线条
        # 创建marker位置：每个窗口大小都绘制一个点
        marker_positions = list(range(len(window_sizes)))
        ax.plot(x, y_ratio, color=colors[index], marker=markers[index], 
                linestyle='-', linewidth=4, markersize=8, 
                label=dataset_list[index], markevery=marker_positions, alpha=0.9)
    
    # 设置标题和坐标轴
    ax.set_title("(a) Window Size vs Comp. Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
    ax.set_xlabel("Window Size", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax.set_ylabel("Compression Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax.set_xscale('log', base=2)  # 使用对数坐标轴，因为窗口大小是2的幂次
    ax.set_xticks(window_sizes)
    # 将x轴刻度标签修改为2的n次幂数学公式形式
    power_labels = []
    for ws in window_sizes:
        power = int(np.log2(ws))
        power_labels.append(f'$2^{{{power}}}$')
    ax.set_xticklabels(power_labels)
    ax.set_ylim(10, 50)
    ax.set_yticks([10, 20, 30, 40, 50])
    
    # 移除网格线
    # ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5)
    
    # 设置字体
    for label in ax.get_xticklabels() + ax.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(size)
    
    # 不显示图例
    ax.legend().remove()
    
    # 调整布局
    fig.tight_layout()
    
    # 保存图片
    fig.savefig("./window_size_no_legend.png", dpi=400, bbox_inches='tight')
    fig.savefig("./window_size_no_legend.eps", format='eps', dpi=400, bbox_inches='tight')
    print("无图例图已保存为 window_size_no_legend.png 和 window_size_no_legend.eps")


if __name__ == "__main__":
    draw_window_size_comparison()
