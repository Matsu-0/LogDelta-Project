import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

file_name = [
    "Zookeeper", "OpenStack", "Spark", "Linux",
    "Mac", "Thunderbird", "Apache",
    "SSH", "Proxifier", "Android"
]

# zstd级别参数
zstd_level = 17

def get_file_size(file_path):
    """获取文件大小（字节）"""
    try:
        return os.path.getsize(file_path)
    except:
        return 0

def calculate_compression_ratio(original_size, compressed_size):
    """计算压缩比"""
    if compressed_size == 0:
        return 0
    return original_size / compressed_size

def calculate_compression_speed(original_size, compression_time):
    """计算压缩速度（MB/s）"""
    if compression_time == 0:
        return 0
    return (original_size / (1024 * 1024)) / compression_time

def calculate_decompression_speed(original_size, decompression_time):
    """计算解压缩速度（MB/s）"""
    if decompression_time == 0:
        return 0
    return (original_size / (1024 * 1024)) / decompression_time

def analyze_compression_results():
    """分析压缩结果 - 保留8种方法的数据"""
    results = []
    
    for filename in file_name:
        # 原始文件路径
        original_file = f"./test_dataset/{filename}.log"
        original_size = get_file_size(original_file)
        
        # 只保留需要的压缩结果
        # LZMA
        lzma_file = f"./compress/result_lzma/{filename}.7z"
        lzma_size = get_file_size(lzma_file)
        
        # zstd (level zstd_level)
        zstd_17_file = f"./compress/result_zstd/level_{zstd_level}/{filename}.zst"
        zstd_17_size = get_file_size(zstd_17_file)
        
        # zstd (level zstd_level+1)
        zstd_18_file = f"./compress/result_zstd/level_{zstd_level+1}/{filename}.zst"
        zstd_18_size = get_file_size(zstd_18_file)
        
        # LZ4 (level 12)
        lz4_ratio_file = f"./compress/result_lz4_hc/level_12/{filename}.lz4"
        lz4_ratio_size = get_file_size(lz4_ratio_file)
        
        # LogZip-LZMA
        logzip_lzma_file = f"./compress/result_logzip/lzma/{filename}.logzip.tar.lzma"
        logzip_lzma_size = get_file_size(logzip_lzma_file)
        
        # LogDelta-LZMA
        logdelta_lzma_file = f"./compress/result_approx/lzma/{filename}.lzma"
        logdelta_lzma_size = get_file_size(logdelta_lzma_file)
        
        # LogShrink-LZMA（文件夹大小）
        logshrink_lzma_dir = f"./compress/result_logshrink/{filename}"
        logshrink_lzma_size = 0
        if os.path.exists(logshrink_lzma_dir):
            for root, dirs, files in os.walk(logshrink_lzma_dir):
                for file in files:
                    file_path = os.path.join(root, file)
                    logshrink_lzma_size += get_file_size(file_path)
        
        # MLC-LZMA
        mlc_lzma_file = f"./compress/result_mlc/{filename}"
        mlc_lzma_size = get_file_size(mlc_lzma_file)
        mlc_missing = filename in ["Mac", "Thunderbird"]
        
        # 读取压缩时间
        lzma_time = 0
        zstd_17_time = 0
        zstd_18_time = 0
        lz4_ratio_time = 0
        logzip_lzma_time = 0
        logdelta_lzma_time = 0
        logshrink_lzma_time = 0
        mlc_lzma_time = 0
        
        # 读取解压缩时间
        lzma_decomp_time = 0
        zstd_17_decomp_time = 0
        zstd_18_decomp_time = 0
        lz4_ratio_decomp_time = 0
        logdelta_lzma_decomp_time = 0
        logshrink_lzma_decomp_time = 0
        mlc_lzma_decomp_time = 0
        
        # 读取LZMA时间
        try:
            lzma_time_df = pd.read_csv("./compress/result_lzma/time_cost.csv")
            lzma_time = lzma_time_df[lzma_time_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取zstd时间
        try:
            zstd_17_df = pd.read_csv(f"./compress/result_zstd/level_{zstd_level}/time_cost.csv")
            zstd_17_time = zstd_17_df[zstd_17_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取zstd (zstd_level+1)时间
        try:
            zstd_18_df = pd.read_csv(f"./compress/result_zstd/level_{zstd_level+1}/time_cost.csv")
            zstd_18_time = zstd_18_df[zstd_18_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取LZ4时间
        try:
            lz4_ratio_df = pd.read_csv("./compress/result_lz4_hc/level_12/time_cost.csv")
            lz4_ratio_time = lz4_ratio_df[lz4_ratio_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取LogZip时间
        try:
            logzip_lzma_df = pd.read_csv("./compress/result_logzip/lzma/time_cost.csv")
            logzip_lzma_time = logzip_lzma_df[logzip_lzma_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取LogDelta时间
        try:
            logdelta_lzma_df = pd.read_csv("./compress/result_approx/lzma/time_cost.csv")
            logdelta_lzma_time = logdelta_lzma_df[logdelta_lzma_df['Metric'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取LogShrink和MLC时间
        try:
            logshrink_df = pd.read_csv("./compress/result_logshrink/time_cost.csv")
            logshrink_lzma_time = logshrink_df[logshrink_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
            
        try:
            mlc_df = pd.read_csv("./compress/result_mlc/time_cost.csv")
            mlc_lzma_time = mlc_df[mlc_df['Dataset'] == filename]['Compression_Time'].iloc[0]
        except:
            pass
        
        # 读取解压缩时间
        # 读取LZMA解压缩时间
        try:
            lzma_decomp_df = pd.read_csv("./decompress/result_lzma/decompression_time.csv")
            lzma_decomp_time = lzma_decomp_df[lzma_decomp_df['Dataset'] == filename]['Time'].iloc[0]
        except:
            pass
        
        # 读取zstd解压缩时间
        try:
            zstd_17_decomp_df = pd.read_csv(f"./decompress/result_zstd/level_{zstd_level}/decompression_time.csv")
            zstd_17_decomp_time = zstd_17_decomp_df[zstd_17_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 读取zstd (zstd_level+1)解压缩时间
        try:
            zstd_18_decomp_df = pd.read_csv(f"./decompress/result_zstd/level_{zstd_level+1}/decompression_time.csv")
            zstd_18_decomp_time = zstd_18_decomp_df[zstd_18_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 读取LZ4解压缩时间
        try:
            lz4_ratio_decomp_df = pd.read_csv("./decompress/result_lz4/level_12/decompression_time.csv")
            lz4_ratio_decomp_time = lz4_ratio_decomp_df[lz4_ratio_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 读取LogDelta解压缩时间
        try:
            logdelta_lzma_decomp_df = pd.read_csv("./decompress/result_approx/decompression_time.csv")
            logdelta_lzma_decomp_time = logdelta_lzma_decomp_df[logdelta_lzma_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 读取LogShrink解压缩时间
        try:
            logshrink_lzma_decomp_df = pd.read_csv("./decompress/result_logshrink/decompression_time.csv")
            logshrink_lzma_decomp_time = logshrink_lzma_decomp_df[logshrink_lzma_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 读取MLC解压缩时间
        try:
            mlc_lzma_decomp_df = pd.read_csv("./decompress/result_mlc/decompression_time.csv")
            mlc_lzma_decomp_time = mlc_lzma_decomp_df[mlc_lzma_decomp_df['Dataset'] == filename]['Decompression_Time'].iloc[0]
        except:
            pass
        
        # 计算压缩比
        lzma_ratio = calculate_compression_ratio(original_size, lzma_size)
        zstd_17_ratio = calculate_compression_ratio(original_size, zstd_17_size)
        zstd_18_ratio = calculate_compression_ratio(original_size, zstd_18_size)
        lz4_ratio_ratio = calculate_compression_ratio(original_size, lz4_ratio_size)
        logzip_lzma_ratio = calculate_compression_ratio(original_size, logzip_lzma_size)
        logdelta_lzma_ratio = calculate_compression_ratio(original_size, logdelta_lzma_size)
        logshrink_lzma_ratio = calculate_compression_ratio(original_size, logshrink_lzma_size)
        mlc_lzma_ratio = calculate_compression_ratio(original_size, mlc_lzma_size)
        
        # 计算压缩速度
        lzma_speed = calculate_compression_speed(original_size, lzma_time)
        zstd_17_speed = calculate_compression_speed(original_size, zstd_17_time)
        zstd_18_speed = calculate_compression_speed(original_size, zstd_18_time)
        lz4_ratio_speed = calculate_compression_speed(original_size, lz4_ratio_time)
        logzip_lzma_speed = calculate_compression_speed(original_size, logzip_lzma_time)
        logdelta_lzma_speed = calculate_compression_speed(original_size, logdelta_lzma_time)
        logshrink_lzma_speed = calculate_compression_speed(original_size, logshrink_lzma_time)
        mlc_lzma_speed = calculate_compression_speed(original_size, mlc_lzma_time)
        
        # 计算解压缩速度
        lzma_decomp_speed = calculate_decompression_speed(original_size, lzma_decomp_time)
        zstd_17_decomp_speed = calculate_decompression_speed(original_size, zstd_17_decomp_time)
        zstd_18_decomp_speed = calculate_decompression_speed(original_size, zstd_18_decomp_time)
        lz4_ratio_decomp_speed = calculate_decompression_speed(original_size, lz4_ratio_decomp_time)
        logdelta_lzma_decomp_speed = calculate_decompression_speed(original_size, logdelta_lzma_decomp_time)
        logshrink_lzma_decomp_speed = calculate_decompression_speed(original_size, logshrink_lzma_decomp_time)
        mlc_lzma_decomp_speed = calculate_decompression_speed(original_size, mlc_lzma_decomp_time)
        
        results.append({
            'filename': filename,
            'original_size_mb': round(original_size / (1024 * 1024), 2),
            'lzma_ratio': round(lzma_ratio, 2),
            'lzma_speed': round(lzma_speed, 2),
            'lzma_decomp_speed': round(lzma_decomp_speed, 2),
            'zstd_17_ratio': round(zstd_17_ratio, 2),
            'zstd_17_speed': round(zstd_17_speed, 2),
            'zstd_17_decomp_speed': round(zstd_17_decomp_speed, 2),
            'zstd_18_ratio': round(zstd_18_ratio, 2),
            'zstd_18_speed': round(zstd_18_speed, 2),
            'zstd_18_decomp_speed': round(zstd_18_decomp_speed, 2),
            'lz4_ratio_ratio': round(lz4_ratio_ratio, 2),
            'lz4_ratio_speed': round(lz4_ratio_speed, 2),
            'lz4_ratio_decomp_speed': round(lz4_ratio_decomp_speed, 2),
            'logzip_lzma_ratio': round(logzip_lzma_ratio, 2),
            'logzip_lzma_speed': round(logzip_lzma_speed, 2),
            'logdelta_lzma_ratio': round(logdelta_lzma_ratio, 2),
            'logdelta_lzma_speed': round(logdelta_lzma_speed, 2),
            'logdelta_lzma_decomp_speed': round(logdelta_lzma_decomp_speed, 2),
            'logshrink_lzma_ratio': round(logshrink_lzma_ratio, 2),
            'logshrink_lzma_speed': round(logshrink_lzma_speed, 2),
            'logshrink_lzma_decomp_speed': round(logshrink_lzma_decomp_speed, 2),
            'mlc_lzma_ratio': round(mlc_lzma_ratio, 2),
            'mlc_lzma_speed': round(mlc_lzma_speed, 2),
            'mlc_lzma_decomp_speed': round(mlc_lzma_decomp_speed, 2),
            'mlc_missing': mlc_missing
        })
    
    return pd.DataFrame(results)

def print_summary_statistics(df):
    """打印汇总统计信息"""
    print("=== 压缩算法性能汇总 ===")
    print()
    
    # 平均压缩比
    print("平均压缩比:")
    print(f"LZMA: {df['lzma_ratio'].mean():.2f}")
    print(f"zstd ({zstd_level}): {df['zstd_17_ratio'].mean():.2f}")
    print(f"zstd ({zstd_level+1}): {df['zstd_18_ratio'].mean():.2f}")
    print(f"LZ4: {df['lz4_ratio_ratio'].mean():.2f}")
    print(f"LogZip: {df['logzip_lzma_ratio'].mean():.2f}")
    print(f"LogDelta: {df['logdelta_lzma_ratio'].mean():.2f}")
    print(f"LogShrink: {df['logshrink_lzma_ratio'].mean():.2f}")
    mlc_ratio_mean = df[~df['mlc_missing']]['mlc_lzma_ratio'].mean()
    print(f"MLC: {mlc_ratio_mean:.2f} (跳过Mac, Thunderbird)")
    print()
    
    # 平均压缩速度
    print("平均压缩速度 (MB/s):")
    print(f"LZMA: {df['lzma_speed'].mean():.2f}")
    print(f"zstd ({zstd_level}): {df['zstd_17_speed'].mean():.2f}")
    print(f"zstd ({zstd_level+1}): {df['zstd_18_speed'].mean():.2f}")
    print(f"LZ4: {df['lz4_ratio_speed'].mean():.2f}")
    print(f"LogZip: {df['logzip_lzma_speed'].mean():.2f}")
    print(f"LogDelta: {df['logdelta_lzma_speed'].mean():.2f}")
    print(f"LogShrink: {df['logshrink_lzma_speed'].mean():.2f}")
    mlc_speed_mean = df[~df['mlc_missing']]['mlc_lzma_speed'].mean()
    print(f"MLC: {mlc_speed_mean:.2f} MB/s (跳过Mac, Thunderbird)")
    print()
    
    # 最佳压缩比
    best_ratio = df[['lzma_ratio', 'zstd_17_ratio', 'zstd_18_ratio', 'lz4_ratio_ratio', 
                     'logzip_lzma_ratio', 'logdelta_lzma_ratio', 'logshrink_lzma_ratio', 'mlc_lzma_ratio']].max().max()
    print(f"最佳压缩比: {best_ratio:.2f}")
    
    # 最快压缩速度
    best_speed = df[['lzma_speed', 'zstd_17_speed', 'zstd_18_speed', 'lz4_ratio_speed', 
                     'logzip_lzma_speed', 'logdelta_lzma_speed', 'logshrink_lzma_speed', 'mlc_lzma_speed']].max().max()
    print(f"最快压缩速度: {best_speed:.2f} MB/s")

def draw_compression_performance_plot(df):
    """绘制压缩和解压缩性能点图"""
    # 字体大小控制变量
    font_size = 29
    
    # 设置全局字体
    plt.rcParams['font.family'] = 'Times New Roman'
    plt.rcParams['axes.unicode_minus'] = False
    plt.rcParams['font.size'] = font_size  # 设置全局字体大小
    
    # 创建图形 - 两个子图
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 7))
    
    # 定义8种方法，每种方法使用不同的颜色和符号
    methods = {
        'LZMA': {'color': '#1f77b4', 'marker': '*', 'size': 200},           # 蓝色星星
        'LogDelta': {'color': '#ff7f0e', 'marker': 'D', 'size': 100},       # 橙色菱形
        'LogShrink': {'color': '#2ca02c', 'marker': 's', 'size': 100},      # 绿色正方形
        'LogZip': {'color': '#d62728', 'marker': '^', 'size': 100},         # 红色三角形
        'MLC': {'color': '#9467bd', 'marker': 'o', 'size': 100},            # 紫色圆形
        f'zstd ({zstd_level})': {'color': '#8c564b', 'marker': 'P', 'size': 150},           # 棕色五边形
        'LZ4': {'color': '#e377c2', 'marker': 'X', 'size': 150},             # 粉色叉号
        f'zstd ({zstd_level+1})': {'color': '#7f7f7f', 'marker': 'd', 'size': 150} # 灰色菱形
    }
    
    # 定义压缩数据点 - 8种独立方法
    compression_data_points = [
        {'method': 'LZMA', 'ratio': df['lzma_ratio'].mean(), 'speed': df['lzma_speed'].mean()},
        {'method': 'LogDelta', 'ratio': df['logdelta_lzma_ratio'].mean(), 'speed': df['logdelta_lzma_speed'].mean()},
        {'method': 'LogShrink', 'ratio': df['logshrink_lzma_ratio'].mean(), 'speed': df['logshrink_lzma_speed'].mean()},
        {'method': 'LogZip', 'ratio': df['logzip_lzma_ratio'].mean(), 'speed': df['logzip_lzma_speed'].mean()},
        {'method': 'MLC', 'ratio': df[~df['mlc_missing']]['mlc_lzma_ratio'].mean(), 'speed': df[~df['mlc_missing']]['mlc_lzma_speed'].mean()},
        {'method': f'zstd ({zstd_level})', 'ratio': df['zstd_17_ratio'].mean(), 'speed': df['zstd_17_speed'].mean()},
        {'method': 'LZ4', 'ratio': df['lz4_ratio_ratio'].mean(), 'speed': df['lz4_ratio_speed'].mean()},
        {'method': f'zstd ({zstd_level+1})', 'ratio': df['zstd_18_ratio'].mean(), 'speed': df['zstd_18_speed'].mean()}
    ]
    
    # 定义解压缩数据点 - 7种方法（排除LogZip）
    decompression_data_points = [
        {'method': 'LZMA', 'ratio': df['lzma_ratio'].mean(), 'speed': df['lzma_decomp_speed'].mean()},
        {'method': 'LogDelta', 'ratio': df['logdelta_lzma_ratio'].mean(), 'speed': df['logdelta_lzma_decomp_speed'].mean()},
        {'method': 'LogShrink', 'ratio': df['logshrink_lzma_ratio'].mean(), 'speed': df['logshrink_lzma_decomp_speed'].mean()},
        {'method': 'MLC', 'ratio': df[~df['mlc_missing']]['mlc_lzma_ratio'].mean(), 'speed': df[~df['mlc_missing']]['mlc_lzma_decomp_speed'].mean()},
        {'method': f'zstd ({zstd_level})', 'ratio': df['zstd_17_ratio'].mean(), 'speed': df['zstd_17_decomp_speed'].mean()},
        {'method': 'LZ4', 'ratio': df['lz4_ratio_ratio'].mean(), 'speed': df['lz4_ratio_decomp_speed'].mean()},
        {'method': f'zstd ({zstd_level+1})', 'ratio': df['zstd_18_ratio'].mean(), 'speed': df['zstd_18_decomp_speed'].mean()}
    ]
    
    # 绘制压缩数据点（左子图）
    for point in compression_data_points:
        method = point['method']
        ratio = point['ratio']
        speed = point['speed']
        
        # 获取该方法的颜色、形状和大小
        method_config = methods[method]
        color = method_config['color']
        marker = method_config['marker']
        size = method_config['size']
        
        # 绘制点
        ax1.scatter(ratio, speed, c=color, marker=marker, s=size, alpha=0.8, edgecolors='black', linewidth=1)
    
    # 设置左子图坐标轴
    ax1.set_xlabel('Comp. Ratio', fontweight='bold', fontsize=font_size)
    ax1.set_ylabel('Comp. Speed (MB/s)', fontweight='bold', fontsize=font_size)
    ax1.set_title('(a) Compression', fontweight='bold', fontsize=font_size, pad=10)
    
    # 设置y轴为对数刻度
    ax1.set_yscale('log')
    
    # 绘制解压缩数据点（右子图）
    for point in decompression_data_points:
        method = point['method']
        ratio = point['ratio']
        speed = point['speed']
        
        # 获取该方法的颜色、形状和大小
        method_config = methods[method]
        color = method_config['color']
        marker = method_config['marker']
        size = method_config['size']
        
        # 绘制点
        ax2.scatter(ratio, speed, c=color, marker=marker, s=size, alpha=0.8, edgecolors='black', linewidth=1)
    
    # 设置右子图坐标轴
    ax2.set_xlabel('Comp. Ratio', fontweight='bold', fontsize=font_size)
    ax2.set_ylabel('Decomp. Speed (MB/s)', fontweight='bold', fontsize=font_size)
    ax2.set_title('(b) Decompression', fontweight='bold', fontsize=font_size, pad=10)
    
    # 设置y轴为对数刻度
    ax2.set_yscale('log')
    
    # 统一两个子图的y轴范围
    ax1.set_ylim(0.01, 2000)
    ax2.set_ylim(0.01, 2000)
    
    # 统一两个子图的横轴刻度
    # 计算所有压缩比的范围
    all_ratios = []
    for point in compression_data_points:
        all_ratios.append(point['ratio'])
    for point in decompression_data_points:
        all_ratios.append(point['ratio'])
    
    min_ratio = min(all_ratios)
    max_ratio = max(all_ratios)
    
    # 设置相同的横轴范围，并添加一些边距
    margin = (max_ratio - min_ratio) * 0.1
    x_min = max(0, min_ratio - margin)
    x_max = max_ratio + margin
    
    # 设置两个子图的横轴范围
    ax1.set_xlim(x_min, x_max)
    ax2.set_xlim(x_min, x_max)
    
    # 设置相同的横轴刻度数量
    ax1.locator_params(axis='x', nbins=6)
    ax2.locator_params(axis='x', nbins=6)
    
    # 创建图例 - 为每种方法创建图例项
    legend_elements = []
    
    for method, config in methods.items():
        legend_elements.append(plt.Line2D([0], [0], marker=config['marker'], color=config['color'], 
                                        markersize=8, label=method, linestyle=''))
    
    # 将图例放在两个子图的最上方
    fig.legend(handles=legend_elements, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, 
              columnspacing=0.6, handletextpad=0.2, borderpad=0.3)
    
    # 调整布局，为图例留出空间
    plt.tight_layout()
    plt.subplots_adjust(top=0.73)  # 为图例留出顶部空间
    
    # 保存图像为PNG和EPS两种格式
    plt.savefig('compression_performance_plot.png', dpi=300, bbox_inches='tight')
    plt.savefig('compression_performance_plot.eps', bbox_inches='tight')
    print("性能对比图已保存为 compression_performance_plot.png 和 compression_performance_plot.eps")
    
    # 显示图像
    plt.show()

if __name__ == "__main__":
    # 分析压缩结果
    results_df = analyze_compression_results()
    
    # 只显示LogDelta的压缩结果
    print("=== LogDelta压缩结果 ===")
    logdelta_results = results_df[['filename', 'original_size_mb', 'logdelta_lzma_ratio', 'logdelta_lzma_speed']].copy()
    logdelta_results.columns = ['Dataset', 'Original_Size_MB', 'Compression_Ratio', 'Compression_Speed_MB_s']
    print(logdelta_results.to_string(index=False))
    print()
    
    # 显示LogDelta汇总统计
    print("=== LogDelta性能汇总 ===")
    print(f"平均压缩比: {results_df['logdelta_lzma_ratio'].mean():.2f}")
    print(f"平均压缩速度: {results_df['logdelta_lzma_speed'].mean():.2f} MB/s")
    print(f"最佳压缩比: {results_df['logdelta_lzma_ratio'].max():.2f}")
    print(f"最快压缩速度: {results_df['logdelta_lzma_speed'].max():.2f} MB/s")
    print()
    
    # 显示LZ4的压缩结果
    print("=== LZ4压缩结果 ===")
    lz4_results = results_df[['filename', 'original_size_mb', 'lz4_ratio_ratio', 'lz4_ratio_speed']].copy()
    lz4_results.columns = ['Dataset', 'Original_Size_MB', 'Compression_Ratio', 'Compression_Speed_MB_s']
    print(lz4_results.to_string(index=False))
    print()
    
    # 显示LZ4汇总统计
    print("=== LZ4性能汇总 ===")
    print(f"平均压缩比: {results_df['lz4_ratio_ratio'].mean():.2f}")
    print(f"平均压缩速度: {results_df['lz4_ratio_speed'].mean():.2f} MB/s")
    print(f"最佳压缩比: {results_df['lz4_ratio_ratio'].max():.2f}")
    print(f"最快压缩速度: {results_df['lz4_ratio_speed'].max():.2f} MB/s")
    print()
    
    # 显示zstd(17)的压缩结果
    print("=== zstd(17)压缩结果 ===")
    zstd_17_results = results_df[['filename', 'original_size_mb', 'zstd_17_ratio', 'zstd_17_speed']].copy()
    zstd_17_results.columns = ['Dataset', 'Original_Size_MB', 'Compression_Ratio', 'Compression_Speed_MB_s']
    print(zstd_17_results.to_string(index=False))
    print()
    
    # 显示zstd(17)汇总统计
    print("=== zstd(17)性能汇总 ===")
    print(f"平均压缩比: {results_df['zstd_17_ratio'].mean():.2f}")
    print(f"平均压缩速度: {results_df['zstd_17_speed'].mean():.2f} MB/s")
    print(f"最佳压缩比: {results_df['zstd_17_ratio'].max():.2f}")
    print(f"最快压缩速度: {results_df['zstd_17_speed'].max():.2f} MB/s")
    print()
    
    # 绘制性能对比图
    print("正在绘制性能对比图...")
    draw_compression_performance_plot(results_df)
    
    # 保存结果到CSV文件
    results_df.to_csv("compression_analysis_results.csv", index=False)
    print("\n结果已保存到 compression_analysis_results.csv")