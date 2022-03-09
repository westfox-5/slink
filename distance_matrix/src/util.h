#include <string>
#include <sstream>

std::string putlocation(const std::string filename, int row, int col) {
    std::stringstream ss;
    ss<<filename<<":"<<row<<":"<<col;
    return ss.str();
}