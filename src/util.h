#pragma once

#include <string>
#include <sstream>
#include <cmath>
#include <limits>  // for INFINITY
#include <iomanip> // for double-to-string conversion

constexpr double INF = std::numeric_limits<double>::infinity();

std::string putlocation(const std::string filename, int row, int col)
{
    std::stringstream ss;
    ss << filename << ":" << row << ":" << col;
    return ss.str();
}

void printVector(std::vector<double> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << "[" << i << "]: " << vec.at(i) << std::endl;
    }
}

void printVector(std::vector<int> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << "[" << i << "]: " << vec.at(i) << std::endl;
    }
}

void printVectorIndexHoriz(int size)
{
    for (int i = 0; i < size; ++i)
    {
        std::cout << std::setw(4) << i << " ";
    }
    std::cout << std::endl;
}
void printSep(int size) {
    for (int i = 0; i < size; ++i)
    {
        std::cout << std::setw(4) << "-----";
    }
    std::cout << std::endl;
}

void printVectorHoriz(std::vector<int> vec)
{
    std::cout << std::setprecision(2);
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << std::setw(4) << vec.at(i) << " ";
    }
    std::cout << std::endl;
}

void printVectorHoriz(std::vector<double> vec)
{
    std::cout << std::setprecision(2);
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << std::setw(4) << vec.at(i) << " ";
    }
    std::cout << std::endl;
}