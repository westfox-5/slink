#include "matrix.h"

const Matrix *Matrix::create(Matrix::Type type, Matrix::InputType fileType, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn)
{
    switch (fileType)
    {
    case DIST:
        return createFromDist(type, filename);
    case CSV:
        return createFromCsv(type, filename, dist_fn);
    default:
        std::throw_with_nested(std::invalid_argument("Input Type not managed."));
    }
};

long Matrix::getDimension() const
{
    return dimension;
}

int LinearMatrix::indexOf(int i, int j) const
{
    return i * dimension + j;
}

int ColMajorMatrix::indexOf(int i, int j) const
{
    int max = i > j ? i : j;
    int min = i < j ? i : j;
    return ((max * (max - 1)) / 2) + min;
}

long Matrix::getSize() const
{
    return dimension * dimension;
}

long LinearMatrix::getSize(int N) const
{
    return N * N;
}

long ColMajorMatrix::getSize(int N) const
{
    return N * (N - 1) / 2;
}

double Matrix::valueAt(int row, int col) const
{
    if (row == col)
        return 0;
    int indexOf = this->indexOf(row, col);
    if (indexOf > -1 && indexOf < this->vec.size())
        return this->vec.at(indexOf);
    return -1;
}

bool LinearMatrix::store(int row, int col) const
{
    return true;
}

bool ColMajorMatrix::store(int row, int col) const
{
    return (row < col);
}

void Matrix::print() const
{
    std::cout << "Dimension: " << this->dimension << std::endl;
    for (int col = 0; col < this->dimension; ++col)
    {
        for (int row = 0; row < this->dimension; ++row)
        {
            std::cout << "[" << row << "," << col << "]:" << this->valueAt(row, col) << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "" << std::endl;
}

const Matrix *Matrix::createFromCsv(Matrix::Type type, std::string filename, std::function<double(const rapidcsv::Document *, int r1, int r2)> dist_fn)
{
    Matrix *matrix = nullptr;

    switch(type)
    {
    case Matrix::Type::LINEAR:
        matrix = new LinearMatrix(filename);
        break;
    case Matrix::Type::COL_MAJOR:
        matrix = new ColMajorMatrix(filename);
        break;
    default:
        std::throw_with_nested(std::invalid_argument("Matrix Type not managed."));
    }

    rapidcsv::Document doc(filename, rapidcsv::LabelParams(-1, -1));
    long N = doc.GetRowCount();
    matrix->dimension = N;
    long size = matrix->getSize(N);
    matrix->vec.reserve(size);

    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            if (i != j)
            {
                double dist = dist_fn(&doc, i, j);
                if (matrix->store(i, j))
                {
                    matrix->vec.push_back(dist);
                }
            }
        }
    }
    return matrix;
}

const Matrix *Matrix::createFromDist(Matrix::Type type, std::string filename)
{
    Matrix *matrix = nullptr;

    switch(type)
    {
    case Matrix::Type::LINEAR:
        matrix = new LinearMatrix(filename);
        break;
    case Matrix::Type::COL_MAJOR:
        matrix = new ColMajorMatrix(filename);
        break;
    default:
        std::throw_with_nested(std::invalid_argument("Matrix Type not managed."));
    }

    std::ifstream inFile(filename);
    std::string line;

    if (!inFile.is_open())
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: could not open file." << std::endl;
        exit(1);
    }

    if (inFile.eof())
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: expecting first line to contain dimension of the matrix." << std::endl;
        exit(1);
    }

    int N;
    inFile >> N;

    if (N < 0)
    {
        std::cerr << putlocation(filename, 0, 0) << ": ERROR: expecting a square matrix." << std::endl;
        exit(1);
    }

    matrix->dimension = N;
    long size = matrix->getSize(N);
    matrix->vec.reserve(size);
    std::string in;
    for (int col = 0; col < N; ++col)
    {
        for (int row = 0; row < N; ++row)
        {
            // check unexpected EOF
            if (inFile.eof())
            {
                std::cerr << putlocation(filename, row, col) << ": ERROR: unexpected EOF." << std::endl;
                exit(1);
            }

            // read next token as string
            inFile >> in;
            double value = atof(in.c_str());

            if (matrix->store(row, col))
            {
                matrix->vec.push_back(value);
            }
        }
    }

    return matrix;
}