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

marksize = 15

# Define datasets
dataset_list = [
    "loghub-Zookeeper", "loghub-OpenStack", "loghub-Spark", "loghub-Linux",
    "loghub-Mac", "loghub-Thunderbird", "loghub-Apache",
    "loghub-OpenSSH", "loghub-Proxifier", "loghub-Android"
]

file_name = [
    "Zookeeper", "OpenStack", "Spark", "Linux",
    "Mac", "Thunderbird", "Apache",
    "SSH", "Proxifier", "Android"
]

# Plot settings
title_i = ["a","b","c","d","e","f","g","h","i","j"]
marker_list = ['^', 'o']  # Triangle for Approx, Circle for Exact

def draw_data_size_fill():
    data_size = range(1000, 20500, 500)
    size = 50
    figsize = (50, 16)
    length_x = 2
    length_y = 5
    fig, ax_arr = plt.subplots(length_x, length_y, figsize=figsize)
    fig.subplots_adjust(hspace=0.6)
    fig.subplots_adjust(wspace=0.4)

    # 配色
    ratio_colors = ['red', 'blue']
    time_colors = ['#ff6666', '#6699ff']
    fill_alphas = [0.25, 0.25]
    markers = ['^', 'o']
    algorithm_order = ["LogDelta-Approx", "LogDelta-Exact"]
    title_i = ["a","b","c","d","e","f","g","h","i","j"]

    for index in range(len(dataset_list)):
        file_size = []
        encode_exact_file_size = []
        encode_approx_file_size = []
        # 读取压缩比数据
        for i in data_size:
            filename = './test_dataset/'+ file_name[index] + '_' + str(i) +'.txt'
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                file_size.append(len(filebytes))
            filename = './result_exact/' + file_name[index] + '/' + file_name[index] + '_' + str(i)
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                encode_exact_file_size.append(len(filebytes))
            filename = './result_approx/' + file_name[index] + '/' + file_name[index] + '_' + str(i) 
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                encode_approx_file_size.append(len(filebytes))
        # 读取时间数据
        filename = './result_approx/time_cost.csv'
        time_df = pd.read_csv(filename)
        if 'Parameter' in time_df.columns:
            time_df = time_df.drop(columns=['Parameter'])
        encode_approx_time = time_df[file_name[index]].tolist()
        filename = './result_exact/time_cost.csv'
        time_df = pd.read_csv(filename)
        if 'Parameter' in time_df.columns:
            time_df = time_df.drop(columns=['Parameter'])
        encode_exact_time = time_df[file_name[index]].tolist()
        # 采样
        x = np.array(list(data_size)[::3])
        ratio_approx = np.array([file_size[j] / encode_approx_file_size[j] for j in range(len(encode_approx_file_size))])[::3]
        ratio_exact = np.array([file_size[j] / encode_exact_file_size[j] for j in range(len(encode_exact_file_size))])[::3]
        time_approx = np.array(encode_approx_time)[::3]
        time_exact = np.array(encode_exact_time)[::3]
        dx = int(index / length_y)
        dy = int(index % length_y)
        ax1 = ax_arr[dx][dy]
        ax2 = ax1.twinx()
        # ax1: 画 time（速度，填充和虚线，左y轴，log坐标）
        ax1.fill_between(x, time_exact, color='#cce0ff', label='Time - LogDelta-Exact', zorder=1)
        ax1.plot(x, time_exact, color=time_colors[1], linestyle='--', linewidth=3, marker=None, zorder=2)
        ax1.fill_between(x, time_approx, color='#ffcccc', label='Time - LogDelta-Approx', zorder=3)
        ax1.plot(x, time_approx, color=time_colors[0], linestyle='--', linewidth=3, marker=None, zorder=4)
        ax1.set_yscale('log')
        ax1.set_ylim(0.001, 1000)
        ax1.set_yticks([0.001, 0.1, 10, 1000])
        # ax2: 画 ratio（压缩比，主线，右y轴，线性坐标）
        ax2.plot(x, ratio_approx, color=ratio_colors[0], marker=markers[0], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Approx', zorder=10)
        ax2.plot(x, ratio_exact, color=ratio_colors[1], marker=markers[1], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Exact', zorder=11)
        ax2.set_ylim(0, 60)
        ax2.yaxis.set_major_locator(MaxNLocator(nbins=4))
        # 设置y轴位置互换（ax1右侧，ax2左侧）
        ax1.yaxis.set_label_position('right')
        ax1.yaxis.tick_right()
        ax2.yaxis.set_label_position('left')
        ax2.yaxis.tick_left()
        # 只在最左侧显示ratio标签（ax2），只在最右侧显示time标签（ax1）
        if dy == 0:
            ax2.set_ylabel("Comp. Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        else:
            ax2.set_ylabel("")
        if dy == length_y - 1:
            ax1.set_ylabel("Time Cost (s)", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        else:
            ax1.set_ylabel("")
        # 强制设置所有y轴tick和label字体和大小
        for label in ax1.get_yticklabels():
            label.set_fontfamily('Times New Roman')
            label.set_fontsize(size-10)
        for label in ax2.get_yticklabels():
            label.set_fontfamily('Times New Roman')
            label.set_fontsize(size-10)
        if ax1.yaxis.label is not None:
            ax1.yaxis.label.set_fontfamily('Times New Roman')
            ax1.yaxis.label.set_fontsize(size)
        if ax2.yaxis.label is not None:
            ax2.yaxis.label.set_fontfamily('Times New Roman')
            ax2.yaxis.label.set_fontsize(size)
        ax1.set_xlabel("Data Size", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        ax1.set_title(f"({title_i[index]}) {dataset_list[index]}", fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
        # 强制设置所有x轴tick字体和大小，保证与y轴一致
        for label in ax1.get_xticklabels():
            label.set_fontfamily('Times New Roman')
            label.set_fontsize(size-10)
        # 只显示指定的x轴刻度
        ax1.set_xticks([5000, 10000, 15000])
        # 设置ratio y轴（ax2）只显示4个刻度
        ax2.set_yticks([0, 20, 40, 60])
        # 设置time y轴（ax1, log）只显示4个刻度
        ax1.set_yticks([0.001, 0.1, 10, 1000])

    # 自定义图例
    custom_lines = [
        Line2D([0], [0], color=ratio_colors[0], marker=markers[0], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Approx'),
        Line2D([0], [0], color=ratio_colors[1], marker=markers[1], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Exact'),
        Patch(facecolor='#ffcccc', edgecolor='#ffcccc', label='Time - LogDelta-Approx'),
        Patch(facecolor='#cce0ff', edgecolor='#cce0ff', label='Time - LogDelta-Exact'),
    ]
    legend = fig.legend(custom_lines, [l.get_label() for l in custom_lines],
                       bbox_to_anchor=(0.5, 1.05),
                       loc='upper center',
                       fontsize=size,
                       labelspacing=0.2,
                       handletextpad=0.2,
                       columnspacing=1.5,
                       ncol=4)
    plt.setp(legend.get_texts(), fontfamily='Times New Roman')
    fig.savefig("./data_size_performance_fill.png", dpi=400, bbox_inches='tight')
    fig.savefig("./data_size_performance_fill.eps", format='eps', dpi=400, bbox_inches='tight')


def draw_data_size_fill_two_datasets():
    """绘制Zookeeper和Android两个数据集的data_size_performance_fill图"""
    data_size = range(1000, 20500, 500)
    size = 29  # 字体大小设置为29
    figsize = (12, 6.3)  # 图像大小调整为8, 8，适合两个子图
    length_x = 1
    length_y = 2
    fig, ax_arr = plt.subplots(length_x, length_y, figsize=figsize)
    fig.subplots_adjust(hspace=0.6)
    fig.subplots_adjust(wspace=0.4)
    fig.subplots_adjust(top=0.65)  # 为图例留出更多顶部空间

    # 只选择两个数据集
    selected_datasets = ["Zookeeper", "Android"]
    selected_dataset_list = ["loghub-Zookeeper", "loghub-Android"]
    selected_indices = [0, 9]  # 对应Zookeeper, Android在原始列表中的索引

    # 配色
    ratio_colors = ['red', 'blue']
    time_colors = ['#ff6666', '#6699ff']
    fill_alphas = [0.25, 0.25]
    markers = ['^', 'o']
    algorithm_order = ["LogDelta-Approx", "LogDelta-Exact"]
    title_i = ["a", "b"]

    for plot_index, (index, dataset_name) in enumerate(zip(selected_indices, selected_datasets)):
        file_size = []
        encode_exact_file_size = []
        encode_approx_file_size = []
        # 读取压缩比数据
        for i in data_size:
            filename = './test_dataset/'+ dataset_name + '_' + str(i) +'.txt'
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                file_size.append(len(filebytes))
            filename = './result_exact/' + dataset_name + '/' + dataset_name + '_' + str(i)
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                encode_exact_file_size.append(len(filebytes))
            filename = './result_approx/' + dataset_name + '/' + dataset_name + '_' + str(i) 
            with open(filename, 'rb') as handle:
                filebytes = handle.read()
                encode_approx_file_size.append(len(filebytes))
        # 读取时间数据
        filename = './result_approx/time_cost.csv'
        time_df = pd.read_csv(filename)
        if 'Parameter' in time_df.columns:
            time_df = time_df.drop(columns=['Parameter'])
        encode_approx_time = time_df[dataset_name].tolist()
        filename = './result_exact/time_cost.csv'
        time_df = pd.read_csv(filename)
        if 'Parameter' in time_df.columns:
            time_df = time_df.drop(columns=['Parameter'])
        encode_exact_time = time_df[dataset_name].tolist()
        # 采样
        x = np.array(list(data_size)[::3])
        ratio_approx = np.array([file_size[j] / encode_approx_file_size[j] for j in range(len(encode_approx_file_size))])[::3]
        ratio_exact = np.array([file_size[j] / encode_exact_file_size[j] for j in range(len(encode_exact_file_size))])[::3]
        time_approx = np.array(encode_approx_time)[::3]
        time_exact = np.array(encode_exact_time)[::3]
        
        ax1 = ax_arr[plot_index]
        ax2 = ax1.twinx()
        # ax1: 画 time（速度，填充和虚线，左y轴，log坐标）
        ax1.fill_between(x, time_exact, color='#cce0ff', label='Time - LogDelta-Exact', zorder=1)
        ax1.plot(x, time_exact, color=time_colors[1], linestyle='--', linewidth=3, marker=None, zorder=2)
        ax1.fill_between(x, time_approx, color='#ffcccc', label='Time - LogDelta-Approx', zorder=3)
        ax1.plot(x, time_approx, color=time_colors[0], linestyle='--', linewidth=3, marker=None, zorder=4)
        ax1.set_yscale('log')
        ax1.set_ylim(0.005, 1000)
        ax1.set_yticks([0.1, 10, 1000])
        # ax2: 画 ratio（压缩比，主线，右y轴，线性坐标）
        ax2.plot(x, ratio_approx, color=ratio_colors[0], marker=markers[0], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Approx', zorder=10)
        ax2.plot(x, ratio_exact, color=ratio_colors[1], marker=markers[1], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Exact', zorder=11)
        ax2.set_ylim(0, 60)
        ax2.yaxis.set_major_locator(MaxNLocator(nbins=4))
        # 设置y轴位置互换（ax1右侧，ax2左侧）
        ax1.yaxis.set_label_position('right')
        ax1.yaxis.tick_right()
        ax2.yaxis.set_label_position('left')
        ax2.yaxis.tick_left()
        # 只在最左侧显示ratio标签（ax2），只在最右侧显示time标签（ax1）
        if plot_index == 0:
            ax2.set_ylabel("Comp. Ratio", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        else:
            ax2.set_ylabel("")
        if plot_index == 1:  # 第二个子图显示time标签
            ax1.set_ylabel("Time Cost (s)", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        else:
            ax1.set_ylabel("")
        # 强制设置所有y轴tick和label字体和大小
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
        ax1.set_xlabel("Data Size", fontsize=size, fontfamily='Times New Roman', fontweight='bold')
        ax1.set_title(f"({title_i[plot_index]}) {selected_dataset_list[plot_index]}", fontsize=size, fontfamily='Times New Roman', fontweight='bold', pad=20)
        # 强制设置所有x轴tick字体和大小，保证与y轴一致
        for label in ax1.get_xticklabels():
            label.set_fontfamily('Times New Roman')
            label.set_fontsize(size)
        # 只显示指定的x轴刻度
        ax1.set_xticks([0, 10000, 20000])
        # 设置ratio y轴（ax2）只显示4个刻度
        ax2.set_yticks([0, 20, 40, 60])
        # 设置time y轴（ax1, log）只显示4个刻度
        # ax1.set_yticks([0.001, 0.1, 10, 1000])

    # 自定义图例
    custom_lines = [
        Line2D([0], [0], color=ratio_colors[0], marker=markers[0], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Approx'),
        Line2D([0], [0], color=ratio_colors[1], marker=markers[1], linestyle='-', linewidth=4, markersize=8, label='Ratio - LogDelta-Exact'),
        Patch(facecolor='#ffcccc', edgecolor='#ffcccc', label='Time - LogDelta-Approx'),
        Patch(facecolor='#cce0ff', edgecolor='#cce0ff', label='Time - LogDelta-Exact'),
    ]
    legend = fig.legend(custom_lines, [l.get_label() for l in custom_lines],
                       bbox_to_anchor=(0.5, 1.0),
                       loc='upper center',
                       fontsize=size,
                       labelspacing=0.2,
                       handletextpad=0.2,
                       columnspacing=1.5,
                       ncol=2)
    plt.setp(legend.get_texts(), fontfamily='Times New Roman')
    fig.savefig("./data_size_performance_fill_two.png", dpi=400, bbox_inches='tight')
    fig.savefig("./data_size_performance_fill_two.eps", format='eps', dpi=400, bbox_inches='tight')

if __name__ == "__main__":
    draw_data_size_fill()
    draw_data_size_fill_two_datasets()  # 取消注释以运行新函数
