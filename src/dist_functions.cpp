#include "dist_functions.h"

const std::function<const double(const rapidcsv::Document *, int, int)> DistanceFunctions::by_filename(std::string filename)
{
    if (endsWith(filename, "imports-85.data"))
        return DistanceFunctions::Functions::__imports_85;

    if (endsWith(filename, "e-shop-clothing-2008.csv") || endsWith(filename, "e-shop-clothing-2008__less.csv"))
        return DistanceFunctions::Functions::__eshop_clothing_2008;

    std::throw_with_nested("No distance function found for the file '" + filename + "'");
}

const double DistanceFunctions::Functions::__imports_85(const rapidcsv::Document *doc, int row1, int row2)
{
    double l1 = doc->GetCell<double>(10, row1);
    double w1 = doc->GetCell<double>(11, row1);
    double h1 = doc->GetCell<double>(12, row1);

    double l2 = doc->GetCell<double>(10, row2);
    double w2 = doc->GetCell<double>(11, row2);
    double h2 = doc->GetCell<double>(12, row2);

    return std::sqrt((l1 - l2) * (l1 - l2) + (w1 - w2) * (w1 - w2) + (h1 - h2) * (h1 - h2));
};

const double DistanceFunctions::Functions::__eshop_clothing_2008(const rapidcsv::Document *doc, int row1, int row2)
{
    double price1 = doc->GetCell<double>(11, row1);
    double price2 = doc->GetCell<double>(11, row2);

    return std::sqrt((price1 - price2) * (price1 - price2));
};