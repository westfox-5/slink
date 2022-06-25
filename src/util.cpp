#include "util.h"

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

bool endsWith(std::string& str, std::string suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

std::vector<std::string> split(std::string str, char delim)
{
    std::vector<std::string> v;
    std::stringstream ss(str);
 
    while (ss.good()) {
        std::string substr;
        std::getline(ss, substr, delim);
        v.push_back(substr);
    }
    return v;
}