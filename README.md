# High Performance Computing Project Report

**date**: *27/06/2022*

**course code**: *CM0227*

**reference professor**: *Claudio Lucchese*

**authors**: *Volpe Davide (862989), Sello Stefano (864851)*

## Introduction

[Slink](https://doi.org/10.1093/comjnl/16.1.30) is a single linkage clustering algorithm developed by [R. Sibson](https://academic.oup.com/comjnl/search-results?f_Authors=R.+Sibson) in 1973 in order to provide an efficient algorithm to cluster elements coming from huge datasets with the "single linkage" method. The algorithm is based on the so-called "pointer representation" of a [*dendrogram*](https://en.wikipedia.org/wiki/Dendrogram), which consists of two vectors $\pi$ and $\lambda$ that represent, respectively, the $id$ of the higher element of the cluster they are part of and the $distance$ from the closest element of such cluster (as a matter of fact in the [single linkage method](https://en.wikipedia.org/wiki/Single-linkage_clustering) the distance of an element *x* and a cluster *Y* is the smallest distance between *x* and any element *y* in *Y*).

The aim of this project is to investigate the possibility to parallelize the execution of such algorithm in order to optimize its execution times for large amounts of data. The kernel algorithm is developed using the `C++` language, but a convenience script written in `Python` is provided.

The whole source code can be found in the related [GitHub repository](https://github.com/westfox-5/slink).

## Code structure

The structure of the codebase is quite simple: at the top-level directory of the project there are:
* the `bootstrap.py` script, used to automate the compile/execute tasks;
* the `makefile`, which automates the compilation process;
* the `datasets` directory containing the `csv` files used to test the algorithm
* the `lib` directory to store external libraries
* the `src` directory containing all the source and heading files for the project. In particular, inside the latter there is the `slink_executors` directory which contains the core code responsible for the different execution modes of the algorithm.

### External Dependencies

The only library we rely on is [RapidCSV](https://github.com/d99kris/rapidcsv), a single-header c++ library that helps with parsing and accessing a csv file.
We did not investigate the perfomances of this library since it is only used to load data into our data structure. This fraction of execution must be executed sequentially, so we can consider it constant with respect to the input size.

## Bootstrap synopsis

The convenience script has the meaning of providing a clear and simple user-experience over the program.

### Environment setup

We used [Conda](https://docs.conda.io/en/latest/) to easily setup a portable execution environment.

``` shell
$> conda env create -f environment.yml
$> conda activate slink
```

Then you can run the script with:

```shell
(slink)$> python bootstrap.py

Usage: bootstrap.py [MODE] [MODE ARGS] [OPTIONS] [ARGS]
  MODE:
   -r --random <matrix dimension>            generate a distance matrix with random numbers
   -i --input <input file>                   compute the distance matrix for the specified file
  OPTIONS:
   -h --help                                 print this helper
   -v --verbose                              enable verbose mode
   -f --force                                force recreation of the distance matrix
   -n --num-threads <N>                      specify number of threads for the parallel execution
   -m --matrix-type <linear, col_major>      specify type of matrix to store data
   -t --file-type <dist, csv>                forces the execution with the specified file type
   -e --execution-type <0,..,3>                executes a specific execution policy (0=sequential, 1=parallel OMP, 2=parallel split, 3=parallel split OMP)
   -p --perf                                 enables execution with 'perf' tool
```

Some examples on how to use the script:
* `python bootstrap.py -v -r 500 -n 4`
creates a 500x500 distance matrix filled with random numbers and executes the algorithm on this matrix with 4 threads.
* `python bootstrap.py -v -i datasets/imports-85.data -n 4`
runs the algorithm on the provided csv file with 4 threads.

## Implementations

In this section we focus on the various implementations that we developed,  the al

### Serial

We started this project by developing the serial implementation of the algorithm, based on the pseudo-code provided. This version will be the basis for our further analysis and parallel implementations, since it provides a base execution time and $\pi$ and $\lambda$ correct vectors for reference.
A `check-value` function will be computed on the results of execution in order to provide a simple way of proving correctness of the various implementations. We made the assumption that the value provided by the serial implementation is correct.

#### Performance analysis

From now on, all performance analysis will be provided thanks to the `perf` command line tool, and more specificly wrapping the execution of our algorithms with the `perf -d -r <x>` command, which executes the selected algorithm $x$ times.
The perf mode can be activated through the bootstrap script with the flags `-p` or `--perf`.
Here there are some measurements for a random distance matrix of 5000 elements:
<font size="1">
    
```
 Performance counter stats for './slink dist ./5000.in linear 12 0' (30 runs):

          2,795.98 msec task-clock                #    0.999 CPUs utilized            ( +-  0.62% )
                61      context-switches          #    0.022 K/sec                    ( +- 30.58% )
                 1      cpu-migrations            #    0.000 K/sec                    ( +- 16.61% )
               801      page-faults               #    0.287 K/sec                    ( +-  5.86% )
    12,006,954,321      cycles                    #    4.294 GHz                      ( +-  0.61% )  (49.91%)
    27,274,298,965      instructions              #    2.27  insn per cycle           ( +-  0.02% )  (62.47%)
     5,803,899,146      branches                  # 2075.803 M/sec                    ( +-  0.02% )  (62.51%)
        21,746,477      branch-misses             #    0.37% of all branches          ( +-  4.74% )  (62.55%)
     7,020,332,896      L1-dcache-loads           # 2510.868 M/sec                    ( +-  0.02% )  (62.60%)
        32,957,969      L1-dcache-load-misses     #    0.47% of all L1-dcache accesses  ( +-  0.70% )  (62.58%)
        10,688,240      LLC-loads                 #    3.823 M/sec                    ( +-  0.81% )  (49.96%)
         1,612,549      LLC-load-misses           #   15.09% of all LL-cache accesses  ( +-  0.91% )  (49.90%)

            2.7978 +- 0.0174 seconds time elapsed  ( +-  0.62% )
```
</font>

### OpenMP

The next step we took was to include the [OpenMP](https://www.openmp.org/) API in the serial version of the algorithm. 
We immediately realized how thightly coupled were the iterations of the loop and how difficult it was to parallelize it.

The best configuration for the OMP directive is the one used in [src/slink_executors/parallel_omp.cpp](./src/slink_executors/parallel_omp.cpp). The outer loop could not be parallelized so we focused on the inner loops, specifically on the _update_ and _finalize_ loops. This configuration led to the best results in terms of execution times with respect to the serial implementation. Also, from a cache point of view, parallelyzing the first inner loop will lead to less cache misses, since threads will access the same elements of the distance matrix in a short time range.

#### Performance analysis

The following graph compares the execution times between the serial and parallel versions of the algorithm on a input matrix of size 5000x5000. 

The tests are executed on a Intel(R) Core(TM) i7-8700 CPU @ 3.20GHz with 6 physical cores and 12 threads. The compiler is gcc version 10.2.1.

![](https://i.imgur.com/ING77n4.png)


The obtained speedup is:

|             |                        | SpeedUp |
| ----------- | ---------------------- | ------- |
| Sequential  | Parallel (2 Threads)   | 1,587   |
| Sequential  | Parallel (4 Threads)   | 2,151   |
| Sequential  | Parallel (6 Threads)   | 2,293   |
| Sequential  | Parallel (8 Threads)   | 1,859   |
| Sequential  | Parallel (12 Threads)  | 0,740   |

The parallel version runned with 12 threads clearly deteriorates the mean performances as we can notice from the 3 peaks and the speedup less than 1.

The best speedup obtained is related to the execution with 6 threads. This is as expected since the CPU under test has 6 cores.

<font size="1">
    
```
 Performance counter stats for './slink dist ./5000.in linear 6 1' (30 runs):

          2,972.96 msec task-clock                #    1.089 CPUs utilized            ( +-  0.59% )
                71      context-switches          #    0.024 K/sec                    ( +- 27.39% )
                 1      cpu-migrations            #    0.000 K/sec                    ( +- 13.84% )
               768      page-faults               #    0.258 K/sec                    ( +-  6.13% )
    12,768,922,178      cycles                    #    4.295 GHz                      ( +-  0.59% )  (50.22%)
    27,256,065,262      instructions              #    2.13  insn per cycle           ( +-  0.17% )  (62.75%)
     5,794,185,375      branches                  # 1948.962 M/sec                    ( +-  0.18% )  (62.64%)
        19,264,696      branch-misses             #    0.33% of all branches          ( +-  3.86% )  (62.46%)
     7,065,096,274      L1-dcache-loads           # 2376.452 M/sec                    ( +-  0.13% )  (62.36%)
        35,425,338      L1-dcache-load-misses     #    0.50% of all L1-dcache accesses  ( +-  1.08% )  (62.28%)
         4,539,504      LLC-loads                 #    1.527 M/sec                    ( +-  2.03% )  (49.95%)
         1,579,310      LLC-load-misses           #   34.79% of all LL-cache accesses  ( +-  1.03% )  (50.09%)

            2.7306 +- 0.0172 seconds time elapsed  ( +-  0.63% )
```
</font>

### Divide and Conquer

Next we focused on how to apply a _divide and conquer_ approach to this algorithm. The main idea is to partition the input and apply _Slink_ to each portion but we faced serious issues at the moment of the merging of two partial results.
We tried various different approaches on how to correctly merge two clusters represented in the form of two $\pi$ and $\lambda$ vectors. 

Starting from the first (or left-most) cluster, we tried to adjust each node in this cluster considering distances with the nodes in the second (or right-most) cluster.
But after that we noticed that nodes in the second cluster needed to be finalized too, in particular the $\lambda$ vector of distances.
We developed an algorithm that, given a node _n_ and all nodes _i_ such that _$\pi$[i]=n_ tries to find the minimum distance of node _i_ with any node _j_ such that _$\pi$[j]=$\pi$[n]_. This seems complicated but essentialy is the linkage of two clusters (n and $\pi$[n]) with the minimum distance.
The distance at $\lambda$[n] is updated with such a minimum distance, if found.

We struggled to make it work, but without success, so we decided to keep this version as is and analyze the performances of it, knowing that the correctness is wrong.

#### Performance analysis

The merge process is extremely costly because it loops over all elements of the two input sets twice (in order to adjust distances and links). So it is inconvenient to use it, as also measurements show. In fact the mean execution times for such algorithm in the same conditions as in the previous case are shown in the table below and are always way worst then the sequential algorithm.

|                        | Mean execution time (ms) |
| ---------------------- | ------------------------ |
| Parallel (2 Threads)   | 202                      |
| Parallel (4 Threads)   | 256                      |
| Parallel (6 Threads)   | 356                      |
| Parallel (8 Threads)   | 440                      |

As it is shown by the table above, the mean execution time decreases while the number of concurrent threads increases. This behaviour is due to the fact that more concurrent threads imply more merge operations, that are costly. So this solution is bad and should be avoided. However we weren't able to find a more efficient merging algorithm or a more convenient way to split the algorithm into concurrent tasks.

### Divide and Conquer - OMP

We also tried to apply OpenMP to the _Divide and Conquer_ algorithm, just to check if there was a way to improve its performances (despite the fact that the algorithm isn't completely correct), but the results were not satisfying: in this case the performances degraded consistently.
This is the worst result we obtained so far.

|                        | Mean execution time (ms) |
| ---------------------- | ------------------------ |
| Parallel (2 Threads)   | 14639                    |
| Parallel (4 Threads)   | 256                      |
| Parallel (6 Threads)   | 356                      |
| Parallel (8 Threads)   | 440                      |


## Conclusions
Due to the implicit sequential nature of the algorithm and the lack of literature about the topic the best result we could achieve is also the more obvious: parallelyzing the two inner loops that didn't have any significant data dependency inside them (in particular: the first and the last loop of the sequential implementation of the algorithm). We weren't able to find a way to parallelyze the middle inner loop, nor the outer loop, due to the data dependencies present between iterations. Also, the level of parallelization we achieved isn't really good, since the algorithm is not scalable: increasing the number of concurrent threads does not increase the performances linearly.