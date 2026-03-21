# WSL Development Setup Guide (Windows)

### Install WSL

This guide provides a complete walkthrough for setting up, building, and running the `TrafficSimulation` project on Windows using the Windows Subsystem for Linux (WSL).


## Install WSL
Open PowerShell as Administrator and run:
`wsl --install -d Ubuntu-22.04`
Restart your computer if prompted.

### Open Project in WSL
Open the Ubuntu terminal and navigate to the project:
`cd /mnt/c/Users/YourUser/path/to/TrafficSimulation`

### Install Dependencies
Install build tools, CMake, and project libraries:
```bash
sudo apt-get update
sudo apt-get install -y build-essential git
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null
echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null
sudo apt-get update
sudo apt-get install -y cmake zlib1g-dev libexpat1-dev libbz2-dev libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
```

### Build the Project
`mkdir -p build_wsl`
`cd build_wsl`
`cmake ..`
`make -j$(nproc)`

### Run the Simulation
`./TrafficSimulator -ConfigFile ../data/config.txt`
