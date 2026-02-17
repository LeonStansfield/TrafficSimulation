#pragma once

#include <string>
#include <fstream>
#include <chrono>
#include <map>
#include <iostream>
#include <vector>

class Benchmarker {
public:
    // Delete copy constructor and assignment operator
    Benchmarker(const Benchmarker&) = delete;
    Benchmarker(Benchmarker&&) = delete;
    Benchmarker& operator=(const Benchmarker&) = delete;
    Benchmarker& operator=(Benchmarker&&) = delete;

    static Benchmarker& Get() {
        static Benchmarker instance;
        return instance;
    }

    // Opens the CSV file and writes the header
    void BeginSession(const std::string& filepath);
    
    // Closes the file
    void EndSession();

    // Start a named timer
    void Start(const std::string& name);

    // Stop a named timer and record duration
    void Stop(const std::string& name);

    // Write the accumulated data for the current frame to the CSV
    void WriteFrame(int tick);

    bool IsActive() const { return m_Active; }

private:
    Benchmarker() : m_Active(false) {}
    ~Benchmarker() { EndSession(); }

    bool m_Active;
    std::ofstream m_OutputStream;
    std::map<std::string, double> m_TimerResults; // Stores duration in ms
    std::map<std::string, std::chrono::high_resolution_clock::time_point> m_StartTimes;
    
    // Helper to ensure CSV columns are always in the same order
    const std::vector<std::string> m_ColumnOrder = {"TotalTick", "Quadtree", "Vehicles", "Render"};
};
