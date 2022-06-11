#pragma once

#include <string>
#include <sstream>
#include <limits>  // for INFINITY
#include <iomanip> // for double-to-string conversion

constexpr double INF = std::numeric_limits<double>::infinity();

std::string putlocation(const std::string filename, int row, int col)
{
    std::stringstream ss;
    ss << filename << ":" << row << ":" << col;
    return ss.str();
}

static std::string format(double val)
{
    if (val == INF) return "INF";
    
    std::ostringstream oss;
    oss << std::setprecision(3);
    oss << val;
    return oss.str();
}

void printVector(std::vector<double> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << "[" << i << "]: " << format(vec.at(i)) << std::endl;
    }
}

void printVector(std::vector<int> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << "[" << i << "]: " << vec.at(i) << std::endl;
    }
}