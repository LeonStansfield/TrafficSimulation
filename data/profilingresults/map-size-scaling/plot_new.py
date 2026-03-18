import pandas as pd
import matplotlib.pyplot as plt
import glob
import re
import os

def get_scaling_data(target_dir):
    files = glob.glob(f"{target_dir}/benchmark_*.csv")
    results = []
    
    for file in files:
        match = re.search(r'benchmark_(\d+)\.csv', file)
        if match:
            road_count = int(match.group(1))
            try:
                df = pd.read_csv(file)
                # Calculate averages for all columns
                cols = ['TotalTick', 'Quadtree', 'Vehicles', 'Render']
                averages = {col: df[col].mean() for col in cols if col in df.columns}
                averages['RoadCount'] = road_count
                results.append(averages)
            except Exception as e:
                print(f"Error processing {file}: {e}")
    
    # Sort by road count (map scale)
    results.sort(key=lambda x: x['RoadCount'])
    return results

def plot_comparison():
    # Directories are relative to this script's location
    batching_dir = "batching"
    no_batching_dir = "no-batching"
    
    batch_data = get_scaling_data(batching_dir)
    no_batch_data = get_scaling_data(no_batching_dir)
    
    # --- Plot 1: Simple Comparison ---
    plt.figure(figsize=(12, 7))
    
    if batch_data:
        counts = [d['RoadCount'] for d in batch_data]
        times = [d['TotalTick'] for d in batch_data]
        plt.plot(counts, times, marker='o', label='Total (With Batching)', 
                 linewidth=3, markersize=8, color='#2ecc71', zorder=10)
    
    if no_batch_data:
        counts = [d['RoadCount'] for d in no_batch_data]
        times = [d['TotalTick'] for d in no_batch_data]
        plt.plot(counts, times, marker='s', label='Total (No Batching)', 
                 linewidth=3, markersize=8, color='#e74c3c', linestyle='--', zorder=10)
    
    plt.title("Scaling Analysis: Batching vs No Batching (Total Time)", fontsize=16, pad=20)
    plt.xlabel("Number of Roads (Map Scale)", fontsize=12)
    plt.ylabel("Average Frame Time (ms)", fontsize=12)
    plt.legend(fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig("batching_comparison.png", dpi=300)
    print("Saved: batching_comparison.png")

    # --- Plot 2: Detailed Comparison ---
    plt.figure(figsize=(14, 8))
    
    # Batching results
    if batch_data:
        counts = [d['RoadCount'] for d in batch_data]
        plt.plot(counts, [d['TotalTick'] for d in batch_data], marker='o', 
                 label='Total (Batching)', linewidth=3, color='#2ecc71')
        plt.plot(counts, [d['Render'] for d in batch_data], marker='x', 
                 label='Render (Batching)', linewidth=1.5, color='#27ae60', linestyle=':')
        
        # Shared components (only plot from one dataset to avoid clutter)
        plt.plot(counts, [d['Vehicles'] for d in batch_data], marker='v', 
                 label='Vehicle Update', linewidth=1.5, color='#3498db', linestyle='-.', alpha=0.6)
        plt.plot(counts, [d['Quadtree'] for d in batch_data], marker='^', 
                 label='Quadtree Build', linewidth=1.5, color='#9b59b6', linestyle='-.', alpha=0.6)

    # No-Batching results
    if no_batch_data:
        counts = [d['RoadCount'] for d in no_batch_data]
        plt.plot(counts, [d['TotalTick'] for d in no_batch_data], marker='s', 
                 label='Total (No Batching)', linewidth=3, color='#e74c3c', linestyle='--')
        plt.plot(counts, [d['Render'] for d in no_batch_data], marker='+', 
                 label='Render (No Batching)', linewidth=1.5, color='#c0392b', linestyle=':')

    plt.title("Scaling Analysis: Component Breakdown by Map Scale", fontsize=16, pad=20)
    plt.xlabel("Number of Roads (Map Scale)", fontsize=12)
    plt.ylabel("Average Time (ms)", fontsize=12)
    
    # Legened outside the plot
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=10)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig("batching_comparison_detailed.png", dpi=300)
    print("Saved: batching_comparison_detailed.png")

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    plot_comparison()
