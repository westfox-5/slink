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

static std::string format(double num)
{
    if (num == INF) return "INF";
    
    std::ostringstream oss;
    oss << std::setprecision(3);
    oss << num;
    return oss.str();
}

void printVector(std::vector<double> vec)
{
    for (int i = 0; i < vec.size(); ++i)
    {
        std::cout << "[" << i << "]: " << format(vec.at(i)) << std::endl;
    }
}