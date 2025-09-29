import os
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from matplotlib.ticker import MaxNLocator

# Font size parameter
FONT_SIZE = 29

# Set font size and family for all text elements
plt.rcParams.update({
    'font.size': FONT_SIZE,  # Base font size
    'axes.titlesize': FONT_SIZE,  # Title font size
    'axes.labelsize': FONT_SIZE,  # Axis label font size
    'xtick.labelsize': FONT_SIZE,  # X-axis tick label font size
    'ytick.labelsize': FONT_SIZE,  # Y-axis tick label font size
    'legend.fontsize': FONT_SIZE,  # Legend font size
    'font.family': 'serif',
    'font.serif': ['Times New Roman'],
    'mathtext.fontset': 'stix',  # for math font to match Times New Roman
})

# Set seaborn style
sns.set_style("white")  # Remove grid lines
colors = ['#1f77b4', '#2ca02c', '#ff7f0e', '#d62728']  # Blue, Green, Orange, Red
markers = ['o', 's', 'D', '^']  # Circle, Square, Diamond, Triangle

# Parameters for x-axis (all parameters for plotting)
parameters = [100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 100000]

# Selected parameters for x-axis display
selected_indices = [0, 2, 4, 6, 8, 10]  # indices for 100, 400, 1600, 6400, 25600, 100000 

# All datasets
datasets = ['Android']

# Compression methods
methods = ['LogShrink', 'Logzip', 'Lzma', 'LogDelta']

# Function to get directory size
def get_directory_size(directory_path):
    """计算目录中所有文件的总大小"""
    if not os.path.isdir(directory_path):
        return 0
    
    total_size = 0
    for root, dirs, files in os.walk(directory_path):
        for file in files:
            file_path = os.path.join(root, file)
            if os.path.isfile(file_path):
                total_size += os.path.getsize(file_path)
    return total_size

# Function to get compressed size
def get_compressed_size(method, dataset, param):
    if method == 'LogDelta':
        path = f'./result_approx/{dataset}/{dataset}_{param}.lzma'
        if os.path.exists(path):
            return os.path.getsize(path)
        else:
            return 0
    else:
        path = f'./result_{method}/{dataset}/{dataset}_{param}/'
        total_size = 0
        if os.path.exists(path):
            for root, dirs, files in os.walk(path):
                for file in files:
                    total_size += os.path.getsize(os.path.join(root, file))
        return total_size

# Function to get time cost for a specific dataset
def get_time_cost(method, dataset):
    if method == 'LogDelta':
        # Handle LogDelta's different CSV format
        df = pd.read_csv(f'./result_approx/time_cost.csv')
        dataset_row = df[df['Dataset'] == dataset]
        if not dataset_row.empty:
            # Convert to dictionary with parameter as key
            return {int(col): float(dataset_row[col].iloc[0]) for col in dataset_row.columns if col != 'Dataset'}
        else:
            return {}
    else:
        df = pd.read_csv(f'./result_{method}/time_cost.csv')
        dataset_row = df[df['Dataset'] == dataset]
        if not dataset_row.empty:
            # Convert to dictionary with parameter as key, excluding parameter 50
            return {int(col): float(dataset_row[col].iloc[0]) for col in dataset_row.columns if col != 'Dataset' and int(col) != 50}
        else:
            return {}

# Function to create compression comparison plot for a specific dataset
def create_compression_plot(dataset, original_file, original_dir):
    # Calculate compression ratios and speeds
    compression_ratios = {}
    compression_speeds = {}
    
    # Print header for size comparison table
    print(f"\n=== {dataset} Dataset Size Comparison ===")
    print(f"{'Param':<8} {'LogFile(MB)':<12} {'Dir(MB)':<10} {'LogDelta(MB)':<13} {'LogShrink(MB)':<14} {'Logzip(MB)':<12} {'Lzma(MB)':<10}")
    print("-" * 90)
    
    for method in methods:
        ratios = []
        speeds = []
        time_costs = get_time_cost(method, dataset)
        
        for i, param in enumerate(parameters):
            # Get original size for this specific parameter
            if method in ['LogDelta', 'LogShrink', 'Lzma']:
                # For LogDelta, LogShrink, and Lzma: use single file size
                original_size = os.path.getsize(original_file) if os.path.exists(original_file) else 0
            else:
                # For Logzip: use directory size with parameter
                original_dir_path = f'./test_dataset/{dataset}/{dataset}_{param}/'
                original_size = get_directory_size(original_dir_path)
            
            # Calculate compression ratio
            compressed_size = get_compressed_size(method, dataset, param)
            ratio = original_size / compressed_size if compressed_size > 0 else 0
            ratios.append(ratio)
            
            # Calculate compression speed (original size / compression time)
            if param in time_costs and time_costs[param] > 0:
                time = time_costs[param]
                speed = original_size / time  # bytes per second
                speeds.append(speed / (1024 * 1024))  # convert to MB/s
            else:
                speeds.append(0)
            
            # Print size information for the first method only (to avoid duplication)
            if method == methods[0]:  # Only print once per parameter
                log_file_size = os.path.getsize(original_file) if os.path.exists(original_file) else 0
                original_dir_path = f'../test_dataset/{dataset}/{dataset}_{param}/'
                dir_size = get_directory_size(original_dir_path)
                
                # Get compressed sizes for all methods
                logdelta_size = get_compressed_size('LogDelta', dataset, param)
                logshrink_size = get_compressed_size('LogShrink', dataset, param)
                logzip_size = get_compressed_size('Logzip', dataset, param)
                lzma_size = get_compressed_size('Lzma', dataset, param)
                
                # Convert to MB and format
                log_file_mb = log_file_size / (1024 * 1024)
                dir_mb = dir_size / (1024 * 1024)
                logdelta_mb = logdelta_size / (1024 * 1024)
                logshrink_mb = logshrink_size / (1024 * 1024)
                logzip_mb = logzip_size / (1024 * 1024)
                lzma_mb = lzma_size / (1024 * 1024)
                
                print(f"{param:<8} {log_file_mb:<12.2f} {dir_mb:<10.2f} {logdelta_mb:<13.2f} {logshrink_mb:<14.2f} {logzip_mb:<12.2f} {lzma_mb:<10.2f}")
        
        compression_ratios[method] = ratios
        compression_speeds[method] = speeds
    
    # Create figure with two subplots
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 6.5))
    x = np.arange(len(parameters))
    
    # Create handles and labels for legend
    handles = []
    labels = []
    
    # Prepare x-tick labels
    xtick_labels = ['$2^0$', '$2^2$', '$2^4$', '$2^6$', '$2^8$', '$10^3$']
    
    # Plot compression ratios
    for i, method in enumerate(methods):
        line = ax1.plot(x, compression_ratios[method], marker=markers[i], color=colors[i], 
                       linewidth=4, markersize=8, label=method)
        handles.append(line[0])
        labels.append(method)
    
    # 在设置xlabel、ylabel、legend时强制指定字体
    fontdict = {'family': 'Times New Roman', 'size': FONT_SIZE, 'weight': 'bold'}
    legend_fontdict = {'family': 'Times New Roman', 'size': FONT_SIZE}
    
    ax1.set_xlabel('Block Size (×100)', fontdict=fontdict)
    ax1.set_ylabel('Comp. Ratio', fontdict=fontdict)
    ax1.set_xticks([x[i] for i in selected_indices])
    ax1.set_xticklabels(xtick_labels, fontdict=fontdict)
    ax1.yaxis.set_major_locator(MaxNLocator(nbins=4))  # 控制主刻度数量
    ax1.set_title('(a) Compression Ratio', fontdict=fontdict, pad=10)
    
    # Set y-axis tick labels font
    for label in ax1.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(FONT_SIZE)
    
    
    # Plot compression speeds with log scale
    for i, method in enumerate(methods):
        ax2.plot(x, compression_speeds[method], marker=markers[i], color=colors[i], 
                linewidth=4, markersize=8, label=method)
    ax2.set_xlabel('Block Size (×100)', fontdict=fontdict)
    ax2.set_ylabel('Comp. Speed (MB/s)', fontdict=fontdict)
    ax2.set_xticks([x[i] for i in selected_indices])
    ax2.set_xticklabels(xtick_labels, fontdict=fontdict)
    ax2.set_yscale('log')  # Set y-axis to log scale
    ax2.set_title('(b) Compression Speed', fontdict=fontdict, pad=10)
    
    # Set y-axis tick labels font
    for label in ax2.get_yticklabels():
        label.set_fontfamily('Times New Roman')
        label.set_fontsize(FONT_SIZE)
    
    # Set y-axis limits for compression ratio plot
    all_ratios = [ratio for ratios in compression_ratios.values() for ratio in ratios if ratio > 0]
    if all_ratios:
        max_ratio = max(all_ratios)
        ax1.set_ylim(0, max_ratio * 1.1)  # Start from 0 with some padding
    
    # Set y-axis limits for speed plot
    all_speeds = [speed for speeds in compression_speeds.values() for speed in speeds if speed > 0]
    if all_speeds:
        min_speed = min(all_speeds)
        max_speed = max(all_speeds)
        ax2.set_ylim(min_speed * 0.5, max_speed * 5)  # Add some padding to the range
    
    # Add single legend at the top
    legend = fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(0.5, 1.0), ncol=4, fontsize=FONT_SIZE,
                       columnspacing=0.5, handletextpad=0.3, borderpad=0.3, labelspacing=0.2)
    # 设置图例字体
    for text in legend.get_texts():
        text.set_fontfamily('Times New Roman')
        text.set_fontsize(FONT_SIZE)
    
    # Adjust layout and save
    plt.tight_layout()
    plt.subplots_adjust(top=0.75)  # 为图例留出更多空间
    
    plt.savefig(f'{dataset}_Compression_Comparison.eps', format='eps', bbox_inches='tight', dpi=300)
    plt.savefig(f'{dataset}_Compression_Comparison.png', bbox_inches='tight', dpi=300)
    plt.close()
    
    return compression_ratios, compression_speeds
    
# Main execution
if __name__ == "__main__":
    # # Create output directory if it doesn't exist
    # os.makedirs( exist_ok=True)
    # os.chdir('plots')
    
    all_ratios = {}
    all_speeds = {}
    
    # Process each dataset
    for dataset in datasets:
        print(f"Processing {dataset}...")
        
        # Get original data size - different for different methods
        # For LogDelta and LogShrink: use single file
        # For Logzip and Lzma: use directory size
        original_file = f'./test_dataset/{dataset}.log'
        original_dir = f'./test_dataset/{dataset}/'
        
        if os.path.exists(original_file) and os.path.exists(original_dir):
            # Create plot for this dataset with both file and directory sizes
            ratios, speeds = create_compression_plot(dataset, original_file, original_dir)
            all_ratios[dataset] = ratios
            all_speeds[dataset] = speeds
        else:
            print(f"Warning: {original_file} or {original_dir} not found, skipping {dataset}")
    
