# RecordDelta

A C++ tool for record compression and decompression with support for multiple compression algorithms and distance calculation methods.

## Compilation

### Requirements
- C++17 compiler (g++)
- System libraries: lzma, zlib, zstd

### Build Steps

1. Navigate to the source directory:
```bash
cd c
```

2. Build all programs:
```bash
make all
```

3. Or build separately:
```bash
# Build only compression program
make comp

# Build only decompression program  
make decomp
```

4. Clean build files:
```bash
make clean
```

## Usage

### Compression Program (record_compress)

**Basic Usage:**
```bash
./record_compress <input_file> <output_file> [compressor] [window_size] [threshold] [block_size] [distance] [use_approx] [q_value]
```

**Parameters:**
- `input_file`: Path to the input file to compress
- `output_file`: Path for the compressed output file
- `compressor` (optional): Compression algorithm, supports `none`, `lzma`, `gzip`, `zstd` (default: `none`)
- `window_size` (optional): Window size (default: 8)
- `threshold` (optional): Similarity threshold (default: 0.06)
- `block_size` (optional): Block size (default: 327680000)
- `distance` (optional): Distance calculation method, supports `cosine`, `minhash`, `qgram` (default: `minhash`)
- `use_approx` (optional): Whether to use approximation algorithm, supports `true`, `false` (default: `true`)
- `q_value` (optional): Q-value for Q-gram (default: 3)

**Examples:**
```bash
# Basic compression
./record_compress input.log output.compressed

# Using LZMA compression
./record_compress input.log output.compressed lzma

# Custom parameters
./record_compress input.log output.compressed lzma 16 0.05 65536000 minhash true 4
```

#### Supported Compression Algorithms

- **none**: No additional compression
- **lzma**: LZMA compression algorithm
- **gzip**: GZIP compression algorithm  
- **zstd**: Zstandard compression algorithm

#### Supported Distance Calculation Methods

- **cosine**: Cosine Distance
- **minhash**: MinHash Distance for Jaccard Distance
- **qgram**: Operation Distance

### Decompression Program (record_decompress)

**Basic Usage:**
```bash
./record_decompress <input_file> <output_file>
```

**Parameters:**
- `input_file`: Path to the compressed file to decompress
- `output_file`: Path for the decompressed output file

**Example:**
```bash
# Decompress file
./record_decompress output.compressed decompressed.log
```

## Experimental Results and Visualization

**Note: The `datasets/` and `results/` directories currently contain only scripts and code files. Complete datasets and experimental results can be obtained from the following URL: https://cloud.tsinghua.edu.cn/d/835e8002fbd14e86ae1d/**

The `results/` directory contains visualization scripts for reproducing the figures in the paper. Each experiment directory includes a `draw_fig.py` script that generates publication-ready figures.

### Available Experiments

#### 1. Overall Performance Comparison (`exp1_whole_performance/`)
- **Data Generation Script**: `datasets/exp1_whole_performance.sh`
- **Visualization Script**: `results/exp1_whole_performance/draw_fig.py`
- **Generated Figures**: 
  - `whole_performance.png` / `whole_performance.eps`
- **Paper Section**: TABLE V and Figure 9
- **Description**: Compares compression ratios and speeds across different compression algorithms (LZMA, LZ4, Zstd, LogShrink, LogZip, MLC) on various datasets

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp1_whole_performance.sh
./exp1_whole_performance.sh

# 2. Generate visualization
cd ../results/exp1_whole_performance
python draw_fig.py
```

#### 2. Data Size Impact Analysis (`exp2_data_size/`)
- **Data Generation Script**: `datasets/exp2_data_size.sh`
- **Visualization Script**: `results/exp2_data_size/draw_fig.py`
- **Generated Figures**: 
  - `data_size_analysis.png` / `data_size_analysis.eps`
- **Paper Section**: Figure 10 and Figure 1 in Appendix
- **Description**: Analyzes how compression performance varies with different data sizes

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp2_data_size.sh
./exp2_data_size.sh

# 2. Generate visualization
cd ../results/exp2_data_size
python draw_fig.py
```

#### 3. Distance Function Comparison (`exp3_distance/`)
- **Data Generation Script**: `datasets/exp3_distance.sh`
- **Visualization Script**: `results/exp3_distance/draw_fig.py`
- **Generated Figures**: 
  - `distance_comparison.png` / `distance_comparison.eps`
- **Paper Section**: Figure 11
- **Description**: Compares different distance calculation methods (Cosine, MinHash, Q-gram)

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp3_distance.sh
./exp3_distance.sh

# 2. Generate visualization
cd ../results/exp3_distance
python draw_fig.py
```

#### 4. Block Size Optimization (`exp4_block_size/`)
- **Data Generation Script**: `datasets/exp4_block_size.sh`
- **Visualization Script**: `results/exp4_block_size/draw_fig.py`
- **Generated Figures**: 
  - `block_size_optimization.png` / `block_size_optimization.eps`
- **Paper Section**: Figure 12
- **Description**: Evaluates the impact of different block sizes on compression performance

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp4_block_size.sh
./exp4_block_size.sh

# 2. Generate visualization
cd ../results/exp4_block_size
python draw_fig.py
```

#### 5. Q-Value Analysis (`exp5_q_value/`)
- **Data Generation Script**: `datasets/exp5_q_value.sh`
- **Visualization Script**: `results/exp5_q_value/draw_fig.py`
- **Generated Figures**: 
  - `q_value_analysis.png` / `q_value_analysis.eps`
- **Paper Section**: Figure 13 (a)
- **Description**: Analyzes the effect of different Q-values on Q-gram distance calculation

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp5_q_value.sh
./exp5_q_value.sh

# 2. Generate visualization
cd ../results/exp5_q_value
python draw_fig.py
```

#### 6. Window Size Analysis (`exp6_window_size/`)
- **Data Generation Script**: `datasets/exp6_window_size.sh`
- **Visualization Script**: `results/exp6_window_size/draw_fig.py`
- **Generated Figures**: 
  - `window_size_analysis.png` / `window_size_analysis.eps`
- **Paper Section**: Figure 13 (b)
- **Description**: Evaluates compression performance with different window sizes

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp6_window_size.sh
./exp6_window_size.sh

# 2. Generate visualization
cd ../results/exp6_window_size
python draw_fig.py
```

#### 7. Microbenchmark Analysis (`exp7_microbenchmark/`)
- **Data Generation Script**: `datasets/exp7_microbenchmark.sh`
- **Visualization Script**: `results/exp7_microbenchmark/draw_fig.py`
- **Generated Figures**: 
  - `microbenchmark_results.png` / `microbenchmark_results.eps`
- **Paper Section**: Figure 14
- **Description**: Detailed performance analysis on microbenchmark datasets

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp7_microbenchmark.sh
./exp7_microbenchmark.sh

# 2. Generate visualization
cd ../results/exp7_microbenchmark
python draw_fig.py
```

#### 8. Log Order Impact (`exp8_log_order/`)
- **Data Generation Script**: `datasets/exp8_log_order.sh`
- **Visualization Script**: `results/exp8_log_order/draw_fig.py`
- **Generated Figures**: 
  - `reorder_summary.png` / `reorder_summary.eps`
- **Paper Section**: Figure 15
- **Description**: Compares compression performance on original, restructured, and shuffled log orders

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp8_log_order.sh
./exp8_log_order.sh

# 2. Generate visualization
cd ../results/exp8_log_order
python draw_fig.py
```

#### 9. Column Splitting Analysis (`exp9_split_column/`)
- **Data Generation Script**: `datasets/exp9_split_column.sh`
- **Visualization Script**: `results/exp9_split_column/split_column.py`
- **Generated Figures**: 
  - `split_column_analysis.png` / `split_column_analysis.eps`
- **Paper Section**: Figure 16
- **Description**: Analyzes the impact of column splitting on compression performance

**Usage:**
```bash
# 1. Generate experimental data
cd datasets
chmod +x exp9_split_column.sh
./exp9_split_column.sh

# 2. Generate visualization
cd ../results/exp9_split_column
python split_column.py
```

### Figure Naming Convention

- **PNG format**: `{experiment_name}.png` - For web viewing and presentations
- **EPS format**: `{experiment_name}.eps` - For publication and high-quality printing
- **Font**: All figures use Times New Roman font for consistency with academic publications
- **Resolution**: 200 DPI for high-quality output

### Requirements

- Python 3.7+
- matplotlib
- seaborn
- pandas
- numpy
