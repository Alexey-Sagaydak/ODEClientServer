#pragma once
#include <vector>
#include <cmath>
#include <stdexcept>

std::vector<double> MultiplyScalarByVector(
    double                     scalar,
    std::vector<double> const &vec);

std::vector<double> MultiplyVectorByVector(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2);

std::vector<double> AddVectors(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2);

std::vector<double> SubtractVectors(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2);

double Norm(std::vector<double> const &vec);
