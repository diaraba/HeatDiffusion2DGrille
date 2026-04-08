#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <string>
#include <vector>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    double elapsed_seconds;
    std::string name;
    
    // For multiple measurements
    std::vector<double> measurements;
    
public:
    Timer(const std::string& timer_name = "Timer");
    
    void start();
    void stop();
    void reset();
    double elapsed() const;
    
    // For multiple measurements
    void record();
    double average() const;
    double min() const;
    double max() const;
    void clear_measurements();
    
    // Print results
    void report() const;
};

#endif // TIMER_H
