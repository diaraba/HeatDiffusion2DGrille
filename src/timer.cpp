#include "timer.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

Timer::Timer(const std::string& timer_name) : 
    elapsed_seconds(0.0), name(timer_name) {}

void Timer::start() {
    start_time = std::chrono::high_resolution_clock::now();
}

void Timer::stop() {
    end_time = std::chrono::high_resolution_clock::now();
    elapsed_seconds = std::chrono::duration<double>(end_time - start_time).count();
}

void Timer::reset() {
    elapsed_seconds = 0.0;
}

double Timer::elapsed() const {
    return elapsed_seconds;
}

void Timer::record() {
    // TODO: Record current measurement
    // stop() should be called before record()
    measurements.push_back(elapsed_seconds);
}

void Timer::clear_measurements() {
    measurements.clear();
}

double Timer::average() const {
    if (measurements.empty()) return 0.0;
    double sum = std::accumulate(measurements.begin(), measurements.end(), 0.0);
    return sum / measurements.size();
}

double Timer::min() const {
    if (measurements.empty()) return 0.0;
    return *std::min_element(measurements.begin(), measurements.end());
}

double Timer::max() const {
    if (measurements.empty()) return 0.0;
    return *std::max_element(measurements.begin(), measurements.end());
}

void Timer::report() const {
    std::cout << name << ": " << std::fixed << std::setprecision(6) 
              << elapsed_seconds << " seconds";
    
    if (!measurements.empty()) {
        std::cout << " (avg: " << average() 
                  << ", min: " << min() 
                  << ", max: " << max() 
                  << ", count: " << measurements.size() << ")";
    }
    std::cout << std::endl;
}
