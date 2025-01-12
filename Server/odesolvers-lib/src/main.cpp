// #include <iostream>
// #include <fstream>
// #include "RK2Solver.hpp"
// #include "Storage.hpp"

// int main() {
//     auto vanderpol = [](double t, const std::vector<double>& y) {
//         double mu = 6.0;
//         double p = 1.0;
//         return std::vector<double>{
//                 y[1],
//                 (mu* (1 - y[0] * y[0])* y[1] - y[0]) / p
//             };
//         };
    
//     auto forced_oscillator = [](double t, const std::vector<double>& y) {
//         double omega = 2.0;             // ����������� �������
//         double gamma = 0.1;             // ����������� ���������
//         double F = 1.0;                 // ��������� ������� ����
//         double omega_drive = 1.5;       // ������� ������� ����
//         return std::vector<double>{
//                 y[1],
//                 -omega * omega * y[0] - gamma * y[1] + F * std::cos(omega_drive * t)
//             };
//         };

//     double t0 = 0.0;
//     double tEnd = 20.0;
//     //std::vector<double> y0 = { 1.0, 0.0 };
//     std::vector<double> y0 = { 2.0, 0.0 };
//     double initialStep = 0.001;
//     double tolerance = 0.00001;

//     RK2Solver solver(vanderpol, initialStep);
//     Storage storage;

//     solver.Solve(t0, y0, tEnd, storage, tolerance);

//     // ���������� � ����
//     std::ofstream outFile("results.txt");
//     if (!outFile) {
//         std::cerr << "Failed to open file for writing\n";
//         return 1;
//     }

//     for (size_t i = 0; i < storage.Size(); ++i) {
//         const auto& values = storage[i].second;
//         outFile << values[0] << '\t' << values[1] << std::endl;
//     }

//     outFile.close();
//     std::cout << "Results saved to results.txt\n";

//     return 0;
// }
