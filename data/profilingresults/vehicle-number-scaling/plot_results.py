import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import pandas as pd
import glob
import os
import re

def plot_dir_benchmarks(target_dir):
    files = glob.glob(f"{target_dir}/benchmark_*.csv")
    
    if not files:
        print(f"No benchmark_*.csv files found in {target_dir}.")
        return

    data = []
    
    # Read and process files
    for file in files:
        try:
            match = re.search(r'benchmark_(\d+)\.csv', file)
            if match:
                vehicle_count = int(match.group(1))
            else:
                print(f"Skipping {file}: Could not parse vehicle count.")
                continue
            
            df = pd.read_csv(file)
            df['VehicleCount'] = vehicle_count
            data.append(df)
            
        except Exception as e:
            print(f"Error processing {file}: {e}")

    if not data:
        print(f"No valid data loaded for {target_dir}.")
        return

    data.sort(key=lambda x: x['VehicleCount'].iloc[0])
    
    # ---------------------------------------------------------
    # Plot 1: Scaling Analysis
    # ---------------------------------------------------------
    plt.figure(figsize=(10, 6))
    
    vehicle_counts = []
    avg_total = []
    avg_quadtree = []
    avg_vehicles = []
    for df in data:
        count = df['VehicleCount'].iloc[0]
        vehicle_counts.append(count)
        avg_total.append(df['TotalTick'].mean())
        avg_quadtree.append(df['Quadtree'].mean())
        avg_vehicles.append(df['Vehicles'].mean())

    plt.plot(vehicle_counts, avg_total, marker='o', label='Total Tick Time', linewidth=2)
    plt.plot(vehicle_counts, avg_vehicles, marker='s', label='Vehicle Update', linestyle='--')
    plt.plot(vehicle_counts, avg_quadtree, marker='^', label='Quadtree Build', linestyle='--')

    plt.title(f"Scaling Analysis ({target_dir})")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Average Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"{target_dir}/benchmark_scaling.png")
    print(f"Saved {target_dir}/benchmark_scaling.png")
    plt.close()

    # ---------------------------------------------------------
    # Plot 2: Time Series
    # ---------------------------------------------------------
    plt.figure(figsize=(12, 6))
    
    colors = cm.plasma(np.linspace(0, 1, len(data)))

    for i, df in enumerate(data):
        vehicle_count = df['VehicleCount'].iloc[0]
        smoothed = df['TotalTick'].rolling(window=60).mean()
        label = f"{vehicle_count} Vehicles" if i % 1 == 0 else "_nolegend_"
        plt.plot(df['Tick'], smoothed, label=label, color=colors[i])

    plt.title(f"Time Series: Total Frame Time ({target_dir})")
    plt.xlabel("Tick")
    plt.ylabel("Time (ms)")
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.grid(True)
    plt.savefig(f"{target_dir}/benchmark_timeseries.png", bbox_inches='tight')
    print(f"Saved {target_dir}/benchmark_timeseries.png")
    plt.close()

    # ---------------------------------------------------------
    # Plot 3: Bar Charts
    # ---------------------------------------------------------
    plt.figure(figsize=(12, 12))
    
    plt.subplot(2, 1, 1)
    x_pos = [str(c) for c in vehicle_counts]
    
    bars = plt.bar(x_pos, avg_total, color='skyblue', label='Total Time')
    plt.title(f"Average Total Frame Time ({target_dir})")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Time (ms)")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    for bar in bars:
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height,
                 f'{height:.2f}',
                 ha='center', va='bottom')
                 
    plt.ylim(0, max(avg_total) * 1.15)

    plt.subplot(2, 1, 2)
    bottom_vehicles = avg_quadtree
    
    plt.bar(x_pos, avg_quadtree, label='Quadtree', color='lightgreen')
    plt.bar(x_pos, avg_vehicles, bottom=bottom_vehicles, label='Vehicles', color='salmon')
    
    plt.title(f"Average Component Times Stacked ({target_dir})")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Time (ms)")
    
    max_height = max([q + v for q, v in zip(avg_quadtree, avg_vehicles)])
    plt.ylim(0, max_height * 1.15)
    
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    plt.savefig(f"{target_dir}/benchmark_averages.png")
    print(f"Saved {target_dir}/benchmark_averages.png")
    plt.close()

def plot_benchmarks():
    plot_dir_benchmarks("quadtree")
    plot_dir_benchmarks("no-quadtree")

if __name__ == "__main__":
    plot_benchmarks()
