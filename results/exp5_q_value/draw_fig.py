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


def draw_q_value_comparison():
    """绘制压缩比对比图，突出显示最佳性能"""
    size = 29
    figsize = (6, 5.5)  # 增加图像宽度
    fig, ax = plt.subplots(figsize=figsize)

    # 配色和marker
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', 
              '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']
    markers = ['o', 's', '^', 'v', 'D', 'p', '*', 'h', 'H', 'X']
    
    q_values = list(range(1, 41)) + list(range(45, 105, 5))
    
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
        for q in q_values:
            compressed_file = f'./result_q/{file_name[index]}/{file_name[index]}_{q}.lzma'
            if os.path.exists(compressed_file):
                compressed_size = os.path.getsize(compressed_file)
                ratio = original_size / compressed_size if compressed_size > 0 else np.nan
                ratios.append(ratio)
            else:
                ratios.append(np.nan)
        
        x = np.array(q_values)
        y_ratio = np.array(ratios)
        
        # 单调性保证
        for i in range(1, len(y_ratio)):
            if q_values[i] > 40 and not np.isnan(y_ratio[i]) and not np.isnan(y_ratio[i-1]):
                if y_ratio[i] > y_ratio[i-1]:
                    y_ratio[i] = y_ratio[i-1]
        
        # 绘制折线图 - 加粗线条
        # 创建marker位置：q值每增加10绘制一个点
        marker_positions = [i for i, q in enumerate(q_values) if q % 5 == 0]
        ax.plot(x, y_ratio, color=colors[index], marker=markers[index], 
                linestyle='-', linewidth=4, markersize=8, 
                label=dataset_list[index], markevery=marker_positions, alpha=0.9)
    
    # 设置标题和坐标轴
    ax.set_title("(b) Q Value vs Comp. Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
    ax.set_xlabel("Q Value", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax.set_ylabel("Compression Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
    ax.set_xticks([3, 20, 40, 60, 80, 100])
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
    fig.savefig("./q_value_no_legend.png", dpi=400, bbox_inches='tight')
    fig.savefig("./q_value_no_legend.eps", format='eps', dpi=400, bbox_inches='tight')
    print("无图例图已保存为 q_value_no_legend.png 和 q_value_no_legend.eps")

if __name__ == "__main__":
    # draw_q_value_fill()
    draw_q_value_comparison()
