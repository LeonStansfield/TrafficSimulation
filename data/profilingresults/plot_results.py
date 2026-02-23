import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import pandas as pd
import glob
import os
import re

def plot_benchmarks():
    # Find all benchmark CSV files
    files = glob.glob("benchmark_*.csv")
    
    if not files:
        print("No benchmark_*.csv files found in current directory.")
        return

    data = []
    
    # Read and process files
    for file in files:
        try:
            # Extract vehicle count from filename
            match = re.search(r'benchmark_(\d+)\.csv', file)
            if match:
                vehicle_count = int(match.group(1))
            else:
                print(f"Skipping {file}: Could not parse vehicle count.")
                continue
            
            df = pd.read_csv(file)
            # Add vehicle count column for identification
            df['VehicleCount'] = vehicle_count
            data.append(df)
            
        except Exception as e:
            print(f"Error processing {file}: {e}")

    if not data:
        print("No valid data loaded.")
        return

    # sort data by vehicle count
    data.sort(key=lambda x: x['VehicleCount'].iloc[0])
    
    # ---------------------------------------------------------
    # Plot 1: Scaling Analysis (Average Time vs Vehicle Count)
    # ---------------------------------------------------------
    plt.figure(figsize=(10, 6))
    
    vehicle_counts = []
    avg_total = []
    avg_quadtree = []
    avg_vehicles = []
    avg_render = []

    for df in data:
        count = df['VehicleCount'].iloc[0]
        vehicle_counts.append(count)
        avg_total.append(df['TotalTick'].mean())
        avg_quadtree.append(df['Quadtree'].mean())
        avg_vehicles.append(df['Vehicles'].mean())
        # Check if Render exists, some old files might not have it
        if 'Render' in df.columns:
            avg_render.append(df['Render'].mean())
        else:
            avg_render.append(0)

    plt.plot(vehicle_counts, avg_total, marker='o', label='Total Tick Time', linewidth=2)
    plt.plot(vehicle_counts, avg_vehicles, marker='s', label='Vehicle Update', linestyle='--')
    plt.plot(vehicle_counts, avg_quadtree, marker='^', label='Quadtree Build', linestyle='--')
    plt.plot(vehicle_counts, avg_render, marker='x', label='Render Time', linestyle=':')

    plt.title("Scaling Analysis: Average Frame Time vs Vehicle Count")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Average Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.savefig("benchmark_scaling.png")
    print("Saved benchmark_scaling.png")
    plt.close()

    # ---------------------------------------------------------
    # Plot 2: Time Series Comparison (Total Time over Ticks)
    # ---------------------------------------------------------
    plt.figure(figsize=(12, 6))
    
    # Use a colormap to handle many distinct lines dynamically
    colors = cm.plasma(np.linspace(0, 1, len(data)))

    for i, df in enumerate(data):
        vehicle_count = df['VehicleCount'].iloc[0]
        # Smoothing
        smoothed = df['TotalTick'].rolling(window=60).mean()
        
        # Only put label for every 5th item to avoid crowding the legend
        label = f"{vehicle_count} Vehicles" if i % 1 == 0 else "_nolegend_"
        plt.plot(df['Tick'], smoothed, label=label, color=colors[i])

    plt.title("Time Series: Total Frame Time (Rolling Avg)")
    plt.xlabel("Tick")
    plt.ylabel("Time (ms)")
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.grid(True)
    plt.savefig("benchmark_timeseries.png", bbox_inches='tight')
    print("Saved benchmark_timeseries.png")
    plt.close()

    # ---------------------------------------------------------
    # Plot 3: Bar Charts (Averages)
    # ---------------------------------------------------------
    plt.figure(figsize=(12, 12))
    
    # Subplot 1: Average Total Time
    plt.subplot(2, 1, 1)
    # Use string positions for bars to ensure even spacing regardless of numerical gaps
    x_pos = [str(c) for c in vehicle_counts]
    
    bars = plt.bar(x_pos, avg_total, color='skyblue', label='Total Time')
    plt.title("Average Total Frame Time per Vehicle Count")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Time (ms)")
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    # Add value labels
    for bar in bars:
        height = bar.get_height()
        plt.text(bar.get_x() + bar.get_width()/2., height,
                 f'{height:.2f}',
                 ha='center', va='bottom')
                 
    # Add a buffer to the top so labels aren't cut off
    plt.ylim(0, max(avg_total) * 1.15)

    # Subplot 2: Component Breakdown (Stacked)
    plt.subplot(2, 1, 2)
    
    # Stack calculation
    # p1: Quadtree
    # p2: Vehicles (bottom=Quadtree)
    # p3: Render (bottom=Quadtree+Vehicles)
    
    # Ensure lists are used for bottom parameters
    bottom_vehicles = avg_quadtree
    bottom_render = [q + v for q, v in zip(avg_quadtree, avg_vehicles)]
    
    plt.bar(x_pos, avg_quadtree, label='Quadtree', color='lightgreen')
    plt.bar(x_pos, avg_vehicles, bottom=bottom_vehicles, label='Vehicles', color='salmon')
    plt.bar(x_pos, avg_render, bottom=bottom_render, label='Render', color='orange')
    
    plt.title("Average Component Times per Vehicle Count (Stacked)")
    plt.xlabel("Number of Vehicles")
    plt.ylabel("Time (ms)")
    
    # Calculate max height across all stacks and add 15% buffer
    max_height = max([q + v + r for q, v, r in zip(avg_quadtree, avg_vehicles, avg_render)])
    plt.ylim(0, max_height * 1.15)
    
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    plt.tight_layout()
    plt.savefig("benchmark_averages.png")
    print("Saved benchmark_averages.png")
    plt.close()

if __name__ == "__main__":
    plot_benchmarks()
