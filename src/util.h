#ifndef UTIL_H
#define UTIL_H

#include <iostream>  
#include <string>
#include <cstring> 
#include <sstream>
#include <cmath>
#include <vector>
#include <limits>  // for INFINITY
#include <iomanip> // for double-to-string conversion

constexpr double INF = std::numeric_limits<double>::infinity();

std::string putlocation(const std::string filename, int row, int col);
void printVector(std::vector<double> vec);
void printVector(std::vector<int> vec);
void printVectorIndexHoriz(int size);
void printSep(int size);
void printVectorHoriz(std::vector<int> vec);
void printVectorHoriz(std::vector<double> vec);
bool endsWith(std::string& str, std::string suffix);
std::vector<std::string> split(std::string str, char delim);

#endif