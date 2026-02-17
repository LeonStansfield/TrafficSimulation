#include "Benchmarker.hpp"
#include <iostream>

void Benchmarker::BeginSession(const std::string& filepath) {
    if (m_Active) {
        EndSession();
    }

    m_OutputStream.open(filepath);
    if (!m_OutputStream.is_open()) {
        std::cerr << "Benchmarker Error: Could not open file " << filepath << std::endl;
        return;
    }

    m_Active = true;
    std::cout << "Benchmarking started. Output: " << filepath << std::endl;

    // Write Header
    m_OutputStream << "Tick";
    for (const auto& name : m_ColumnOrder) {
        m_OutputStream << "," << name;
    }
    m_OutputStream << "\n";
}

void Benchmarker::EndSession() {
    if (m_Active) {
        m_OutputStream.close();
        m_Active = false;
        std::cout << "Benchmarking ended." << std::endl;
    }
}

void Benchmarker::Start(const std::string& name) {
    if (!m_Active) return;
    m_StartTimes[name] = std::chrono::high_resolution_clock::now();
}

void Benchmarker::Stop(const std::string& name) {
    if (!m_Active) return;

    auto endTime = std::chrono::high_resolution_clock::now();
    auto startTime = m_StartTimes.find(name);

    if (startTime != m_StartTimes.end()) {
        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(startTime->second).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTime).time_since_epoch().count();
        
        // Duration in milliseconds (floating point for precision)
        double duration = (end - start) * 0.001;
        m_TimerResults[name] = duration;
        
        m_StartTimes.erase(name);
    }
}

void Benchmarker::WriteFrame(int tick) {
    if (!m_Active || !m_OutputStream.is_open()) return;

    m_OutputStream << tick;
    for (const auto& name : m_ColumnOrder) {
        m_OutputStream << ",";
        if (m_TimerResults.count(name)) {
            m_OutputStream << m_TimerResults[name];
        } else {
            m_OutputStream << "0";
        }
    }
    m_OutputStream << "\n";
    m_OutputStream.flush();

    // Reset results for next frame
    m_TimerResults.clear();
}
