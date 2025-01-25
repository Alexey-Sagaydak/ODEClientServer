#include "../include/CommonFunctions.hpp"

std::vector<double> MultiplyScalarByVector(
    double                     scalar,
    std::vector<double> const &vec)
{
    std::vector<double> result(vec.size());
    for (size_t i = 0; i < vec.size(); ++i)
    {
        result[i] = scalar * vec[i];
    }
    return result;
}

std::vector<double> MultiplyVectorByVector(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2)
{
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same size");
    }
    std::vector<double> result(vec1.size());
    for (size_t i = 0; i < vec1.size(); ++i) {
        result[i] = vec1[i] * vec2[i];
    }
    return result;
}

std::vector<double> AddVectors(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2)
{
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same size");
    }
    std::vector<double> result(vec1.size());
    for (size_t i = 0; i < vec1.size(); ++i) {
        result[i] = vec1[i] + vec2[i];
    }
    return result;
}

std::vector<double> SubtractVectors(
    std::vector<double> const &vec1,
    std::vector<double> const &vec2)
{
    if (vec1.size() != vec2.size()) {
        throw std::invalid_argument("Vectors must be of the same size");
    }
    std::vector<double> result(vec1.size());
    for (size_t i = 0; i < vec1.size(); ++i) {
        result[i] = vec1[i] - vec2[i];
    }
    return result;
}

double Norm(std::vector<double> const &vec)
{
    double sum = 0.0;
    for (double val : vec) {
        sum += val * val;
    }
    return std::sqrt(sum);
}
