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
            vehicle_count = int(match.group(1))
            try:
                df = pd.read_csv(file)
                # Calculate averages for all columns
                cols = ['TotalTick', 'Quadtree', 'Vehicles', 'Render']
                averages = {col: df[col].mean() for col in cols if col in df.columns}
                averages['VehicleCount'] = vehicle_count
                results.append(averages)
            except Exception as e:
                print(f"Error processing {file}: {e}")
    
    # Sort by vehicle count
    results.sort(key=lambda x: x['VehicleCount'])
    return results

def plot_comparison():
    # Directories are relative to this script's location
    quadtree_dir = "quadtree"
    no_quadtree_dir = "no-quadtree"
    
    quad_data = get_scaling_data(quadtree_dir)
    no_quad_data = get_scaling_data(no_quadtree_dir)
    
    # --- Plot 1: Simple Comparison (Total Time) ---
    plt.figure(figsize=(12, 7))
    
    if quad_data:
        counts = [d['VehicleCount'] for d in quad_data]
        times = [d['TotalTick'] for d in quad_data]
        plt.plot(counts, times, marker='o', label='Total (With Quadtree)', 
                 linewidth=3, markersize=8, color='#3498db', zorder=10)
    
    if no_quad_data:
        counts = [d['VehicleCount'] for d in no_quad_data]
        times = [d['TotalTick'] for d in no_quad_data]
        plt.plot(counts, times, marker='s', label='Total (No Quadtree)', 
                 linewidth=3, markersize=8, color='#e67e22', linestyle='--', zorder=10)
    
    plt.title("Scaling Analysis: Quadtree vs No Quadtree (Total Time)", fontsize=16, pad=20)
    plt.xlabel("Number of Vehicles", fontsize=12)
    plt.ylabel("Average Frame Time (ms)", fontsize=12)
    plt.legend(fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.tight_layout()
    plt.savefig("quadtree_comparison.png", dpi=300)
    print("Saved: quadtree_comparison.png")

    # --- Plot 2: Detailed Comparison ---
    plt.figure(figsize=(14, 8))
    
    # Quadtree Run
    if quad_data:
        counts = [d['VehicleCount'] for d in quad_data]
        plt.plot(counts, [d['TotalTick'] for d in quad_data], marker='o', 
                 label='Total (Quadtree)', linewidth=3, color='#3498db')
        plt.plot(counts, [d['Vehicles'] for d in quad_data], marker='v', 
                 label='Vehicle Update (Quadtree)', linewidth=1.5, color='#2980b9', linestyle=':')
        plt.plot(counts, [d['Quadtree'] for d in quad_data], marker='^', 
                 label='Quadtree Build', linewidth=1.5, color='#9b59b6', linestyle='-.', alpha=0.7)

    # No Quadtree Run
    if no_quad_data:
        counts = [d['VehicleCount'] for d in no_quad_data]
        plt.plot(counts, [d['TotalTick'] for d in no_quad_data], marker='s', 
                 label='Total (No Quadtree)', linewidth=3, color='#e67e22', linestyle='--')
        plt.plot(counts, [d['Vehicles'] for d in no_quad_data], marker='+', 
                 label='Vehicle Update (No Quadtree)', linewidth=1.5, color='#d35400', linestyle=':')

    plt.title("Vehicle Scaling Analysis, Quadtree vs No Quadtree", fontsize=16, pad=20)
    plt.xlabel("Number of Vehicles", fontsize=12)
    plt.ylabel("Average Time (ms)", fontsize=12)
    
    # Legend outside the plot
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=10)
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.savefig("quadtree_comparison_detailed.png", dpi=300)
    print("Saved: quadtree_comparison_detailed.png")

if __name__ == "__main__":
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    plot_comparison()
