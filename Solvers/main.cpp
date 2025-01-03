#include <iostream>
#include <fstream>
#include "EulerSolver.hpp"

int main() {
    // Oscillator Van der Pol function
    auto vanderpol = [](double t, const std::vector<double>& y) {
        double mu = 6.0; // Parameter for stiffness
        double p = 1.0;  // Parameter for scaling
        return std::vector<double>{
            y[1],
                (mu* (1 - y[0] * y[0])* y[1] - y[0]) / p
        };
        };

    double t0 = 0.0;                              // Initial time
    std::vector<double> y0 = { 2.0, 0.0 };        // Initial conditions: y1 = 2, y2 = 0
    double tEnd = 20.0;                            // End time
    double initialStep = 0.1;                     // Initial step size
    double tolerance = 1e-4;                      // Tolerance for step size adjustment

    EulerSolver solver(vanderpol, initialStep);   // Solver instance
    Storage storage;                              // Storage for results

    solver.Solve(t0, y0, tEnd, storage, tolerance);

    // Output results to file (only y1 and y2)
    std::ofstream outFile("results.txt");
    if (outFile.is_open()) {
        for (size_t i = 0; i < storage.Size(); ++i) {
            outFile << storage[i][0] << "\t" << storage[i][1] << std::endl;
        }
        outFile.close();
        std::cout << "Results saved to 'results.txt'." << std::endl;
    }
    else {
        std::cerr << "Error opening file for writing!" << std::endl;
    }

    return 0;
}
