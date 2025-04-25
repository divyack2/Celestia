// orbit_profiler.cpp
//
// A simple profiling tool for the orbit.cpp code

#include "orbit.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <cmath>

// Simple timer class for profiling
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string function_name;
    std::map<std::string, std::vector<double>>& results;

public:
    Timer(const std::string& name, std::map<std::string, std::vector<double>>& results_map)
        : function_name(name), results(results_map) {
        start_time = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        auto end_time = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / 1000.0;
        results[function_name].push_back(duration);
    }
};

// Define a macro for easy timing of function calls
#define TIME_FUNCTION(name, results) Timer timer##__LINE__(name, results)

// Sample orbit data collection class
class OrbitSampleCollector : public celestia::ephem::OrbitSampleProc {
public:
    std::vector<Eigen::Vector3d> positions;
    std::vector<Eigen::Vector3d> velocities;
    std::vector<double> times;

    void sample(double t, const Eigen::Vector3d& position, const Eigen::Vector3d& velocity) override {
        positions.push_back(position);
        velocities.push_back(velocity);
        times.push_back(t);
    }
};

// Function to print profile results
void printResults(const std::map<std::string, std::vector<double>>& results) {
    std::cout << "\n=== PROFILING RESULTS ===\n";
    std::cout << std::left << std::setw(40) << "FUNCTION" 
            << std::setw(15) << "CALLS" 
            << std::setw(20) << "AVG TIME (µs)" 
            << std::setw(20) << "MIN TIME (µs)" 
            << std::setw(20) << "MAX TIME (µs)" 
            << std::setw(20) << "TOTAL TIME (µs)" << std::endl;
    std::cout << std::string(135, '-') << std::endl;

    std::vector<std::pair<std::string, double>> sortedTotals;
    
    for (const auto& [function, times] : results) {
        if (times.empty()) continue;
        
        double total = 0.0;
        double min_time = times[0];
        double max_time = times[0];
        
        for (double time : times) {
            total += time;
            min_time = std::min(min_time, time);
            max_time = std::max(max_time, time);
        }
        
        double avg = total / times.size();
        sortedTotals.push_back({function, total});
        
        std::cout << std::left << std::setw(40) << function 
            << std::setw(15) << times.size() 
            << std::fixed << std::setprecision(8)
            << std::setw(20) << avg 
            << std::setw(20) << min_time 
            << std::setw(20) << max_time 
            << std::setw(20) << total << std::endl;

    }
    
    // Sort by total time
    std::sort(sortedTotals.begin(), sortedTotals.end(), 
    [](const auto& a, const auto& b) { return a.second > b.second; });

    std::cout << "\n=== FUNCTIONS SORTED BY TOTAL TIME ===\n";
    std::cout << std::left << std::setw(40) << "FUNCTION" 
    << std::setw(20) << "TOTAL TIME (µs)" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto& [function, total] : sortedTotals) {
    std::cout << std::left << std::setw(40) << function 
        << std::fixed << std::setprecision(8) 
        << std::setw(20) << total << std::endl;
    }
}

// Test function to create and use different orbit types
void runOrbitTests(std::map<std::string, std::vector<double>>& results) {
    const double epoch = 2451545.0; // J2000.0
    
    // Setup elliptical orbit
    celestia::astro::KeplerElements elements;
    elements.semimajorAxis = 149.6e6; // Earth orbit in km
    elements.eccentricity = 0.0167;  // Earth eccentricity
    elements.inclination = 0.0;
    elements.longAscendingNode = 0.0;
    elements.argPericenter = 102.9 * M_PI / 180.0;
    elements.meanAnomaly = 0.0;
    elements.period = 365.25 * 86400.0;  // Earth orbital period in seconds
    
    {
        TIME_FUNCTION("EllipticalOrbit constructor", results);
        auto orbit = std::make_unique<celestia::ephem::EllipticalOrbit>(elements, epoch);
    }
    
    auto orbit = std::make_unique<celestia::ephem::EllipticalOrbit>(elements, epoch);
    
    // Test positionAtTime at different orbital positions
    {
        TIME_FUNCTION("positionAtTime (perihelion)", results);
        orbit->positionAtTime(epoch);
    }
    
    {
        TIME_FUNCTION("positionAtTime (90 degrees)", results);
        orbit->positionAtTime(epoch + elements.period / 4.0);
    }
    
    {
        TIME_FUNCTION("positionAtTime (aphelion)", results);
        orbit->positionAtTime(epoch + elements.period / 2.0);
    }
    
    // Test velocityAtTime
    {
        TIME_FUNCTION("velocityAtTime", results);
        orbit->velocityAtTime(epoch);
    }
    
    // Test orbit sampling
    OrbitSampleCollector collector;
    {
        TIME_FUNCTION("sample (full orbit)", results);
        orbit->sample(epoch, epoch + elements.period, collector);
    }
    
    // Print sample count
    std::cout << "Generated " << collector.positions.size() << " orbit samples" << std::endl;
    
    // Setup hyperbolic orbit
    elements.semimajorAxis = -50.0e6;  // negative for hyperbolic
    elements.eccentricity = 1.5;
    
    {
        TIME_FUNCTION("HyperbolicOrbit constructor", results);
        auto hypOrbit = std::make_unique<celestia::ephem::HyperbolicOrbit>(elements, epoch);
    }
    
    auto hypOrbit = std::make_unique<celestia::ephem::HyperbolicOrbit>(elements, epoch);
    
    double startTime, endTime;
    hypOrbit->getValidRange(startTime, endTime);
    std::cout << "Hyperbolic orbit valid range: " << (endTime - startTime) << " days" << std::endl;
    
    {
        TIME_FUNCTION("hyperbolicOrbit->positionAtTime", results);
        hypOrbit->positionAtTime(epoch);
    }
    
    {
        TIME_FUNCTION("hyperbolicOrbit->velocityAtTime", results);
        hypOrbit->velocityAtTime(epoch);
    }
    
    // Test getters and utility functions
    {
        TIME_FUNCTION("getPeriod", results);
        for (int i = 0; i < 1000; i++) {
            orbit->getPeriod();
        }
    }
    
    {
        TIME_FUNCTION("getBoundingRadius", results);
        for (int i = 0; i < 1000; i++) {
            orbit->getBoundingRadius();
        }
    }
    
    // Test fixed orbit
    {
        TIME_FUNCTION("FixedOrbit constructor", results);
        auto fixedOrbit = std::make_unique<celestia::ephem::FixedOrbit>(Eigen::Vector3d(149.6e6, 0, 0));
    }
    
    auto fixedOrbit = std::make_unique<celestia::ephem::FixedOrbit>(Eigen::Vector3d(149.6e6, 0, 0));
    
    {
        TIME_FUNCTION("fixedOrbit->positionAtTime", results);
        for (int i = 0; i < 1000; i++) {
            fixedOrbit->positionAtTime(epoch);
        }
    }
    
    // Test the public interface with many iterations for detailed timing
    std::cout << "Running position calculations 10,000 times..." << std::endl;
    {
        TIME_FUNCTION("10k positionAtTime calls (elliptical)", results);
        for (int i = 0; i < 10000; i++) {
            double t = epoch + (i / 10000.0) * elements.period;
            orbit->positionAtTime(t);
        }
    }
    
    {
        TIME_FUNCTION("10k velocityAtTime calls (elliptical)", results);
        for (int i = 0; i < 10000; i++) {
            double t = epoch + (i / 10000.0) * elements.period;
            orbit->velocityAtTime(t);
        }
    }
    
    // Test performance with different orbital eccentricities
    std::vector<double> eccentricities = {0.01, 0.1, 0.3, 0.5, 0.7, 0.9, 0.99};
    elements.semimajorAxis = 149.6e6; // Reset to elliptical
    
    for (double ecc : eccentricities) {
        elements.eccentricity = ecc;
        auto eccOrbit = std::make_unique<celestia::ephem::EllipticalOrbit>(elements, epoch);
        
        std::string funcName = "positionAtTime (e=" + std::to_string(ecc) + ")";
        {
            TIME_FUNCTION(funcName, results);
            for (int i = 0; i < 1000; i++) {
                double t = epoch + (i / 1000.0) * elements.period;
                eccOrbit->positionAtTime(t);
            }
        }
    }
}

int main() {
    std::map<std::string, std::vector<double>> profileResults;
    
    std::cout << "Running orbit profiling tests...\n";
    
    // Run multiple iterations for more accurate timing
    const int iterations = 5;
    for (int i = 0; i < iterations; i++) {
        std::cout << "Iteration " << (i+1) << " of " << iterations << std::endl;
        runOrbitTests(profileResults);
    }
    
    printResults(profileResults);
    
    return 0;
}