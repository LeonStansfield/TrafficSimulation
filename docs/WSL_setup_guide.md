# WSL Development Setup Guide (Windows)

This guide provides a complete walkthrough for setting up, building, and running the `TrafficSimulation` project on Windows using the Windows Subsystem for Linux (WSL).

## Why WSL?
WSL allows you to run a full Linux environment directly on Windows. This is the recommended way to run this project because:
1.  It is natively a Linux application.
2.  WSL 2 (on Windows 10/11) supports GUI applications out of the box (WSLg), meaning the traffic simulation window will appear on your Windows desktop just like a native app.

## Step 1: Install WSL (If not already installed)
1.  Open **PowerShell** as Administrator.
2.  Run the following command:
    ```powershell
    wsl --install
    ```
    *If you already have WSL but need a specific distro (Ubuntu 22.04 is recommended):*
    ```powershell
    wsl --install -d Ubuntu-22.04
    ```
3.  **Restart your computer** if prompted.
4.  After restarting, open "Ubuntu" from your Start menu to complete the initial setup (username/password).

## Step 2: Open the Project in WSL
Your Windows files are accessible inside WSL under the `/mnt/c/` path.

1.  Open your **Ubuntu** terminal (or type `wsl` in PowerShell). If you need to, use `wsl -s Ubuntu-22.04` to set it as your default.
2.  Navigate to your project directory. 
    *(Replace the filepath with the actual filepath to where the repository is located)*:
    ```bash
    cd /mnt/c/Users/YourUser/.../TrafficSimulation
    ```

---

## Step 3: Install Dependencies
You need to install the build tools and libraries. 

**This step addresses the common "CMake version too old" error.**

Run the following commands in your WSL terminal:

### 3.1 Basic Build Tools
```bash
sudo apt-get update
sudo apt-get install -y build-essential git
```

### 3.2 Upgrade CMake (CRITICAL)
The default CMake in Ubuntu 22.04 (v3.22) is too old for Raylib (v3.25+). Install the latest version from Kitware:
```bash
# Add Kitware repository and key
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

# Install CMake
sudo apt-get update
sudo apt-get install -y cmake
```
*Verify with `cmake --version` (should be 3.25 or higher).*

### 3.3 Project Libraries (Raylib, LibOsmium)
```bash
# LibOsmium & Protozero Dependencies
sudo apt-get install -y zlib1g-dev libexpat1-dev libbz2-dev

# Raylib Dependencies (Graphics/Windowing)
sudo apt-get install -y libasound2-dev libx11-dev libxrandr-dev libxi-dev \
libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
libwayland-dev libxkbcommon-dev
```

## Step 4: Build the Project
Now you are ready to compile.

1.  Create a build directory *inside* the project folder:
    ```bash
    mkdir -p build_wsl
    cd build_wsl
    ```

2.  Run CMake to configure the project:
    ```bash
    cmake ..
    ```

3.  Compile the code:
    ```bash
    make -j$(nproc)
    ```
    *(`-j$(nproc)` tells it to use all CPU cores for faster compilation)*

## Step 5: Run the Simulation
Once compilation finishes successfully:

```bash
./TrafficSimulator -ConfigFile ../data/config.txt
```

The application window should open on your Windows desktop!
