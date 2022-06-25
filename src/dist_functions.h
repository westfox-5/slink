#ifndef DISTANCE_FUNCTIONS_H
#define DISTANCE_FUNCTIONS_H

#include "../lib/rapidcsv.h"
#include "util.h"

// Definitions of distance functions
// to be defined for each dataset
namespace DistanceFunctions {

    const std::function<const double(const rapidcsv::Document *, int, int)> by_filename(std::string filename);

    namespace Functions {
        const double __imports_85(const rapidcsv::Document *doc, int row1, int row2);
        const double __eshop_clothing_2008(const rapidcsv::Document *doc, int row1, int row2);
    }
}

#endif