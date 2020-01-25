# Tree Path Queries
=========

What is it?
-----------
This library realizes the theoretical findings in published reasearch on path queries in weighted trees.

Dependencies

How to build the experiments with pre-generated queries
The target `aggregate_bench` benchmarks all the data structures
against the supplied dataset, for the given query type and (when applicable --
for counting and reporting) parameter K.
The target is defined in `CMakeLists.txt` inside
`${PROJECT_SOURCE_DIR}/src/benchmarking/utils`
To build: cmake from ${PROJECT_SOURCE_DIR}/build 
and then make aggregate_bench

To get a quick idea about the performance of a single data structure,
on a given dataset, and for the given type of query (and again,
when applicable, for given K), one can also build
`cli_bench` target defined in 
`${PROJECT_SOURCE_DIR}/src/benchmarking/utils`
The program is run from commandline with flags:
* `--dataset_path=` absolute path to the dataset
* `--query_type=` one of 
	+ `median` 
	+ `counting`
	+ `reporting`
* `--data_structure=` one of 
	+ `nv`
	+ `nv_lca`
	+ `nv_sct`
	+ `ext_ptr`
	+ `whp_ptr`
	+ `ext_sct_un`
	+ `ext_sct_rrr`
	+ `whp_un`
	+ `whp_rrr`
* `--K=` any positive integer, but typically `1`,`10`,`100` to replicate what we dubbed as 
	+ `large`
	+ `medium`
	+ `small`
Launched with the above flags set, the program instantiates the given data structure over the given
dataset and then measures the query time for the given type of query.


Documentation
-------------

We provide an extensive set of documentation describing all data structures
and features provided by the library. Specifically we provide

* A [cheat sheet][SDSLCS] which succinctly describes the usage of the library.
* An doxygen generated [API reference][DOXYGENDOCS] which lists all types and functions of the library.
* A set of [example](examples/) programs demonstrating how different features of the library are used.
* A tutorial [presentation][TUT] with the [example code](tutorial/) using in the sides demonstrating all features of the library in a step-by-step walk-through.
* [Unit Tests](test/) which contain small code snippets used to test each library feature.

Requirements
------------

The SDSL library requires:

* A modern, `C++17` ready compiler such as `g++` version 5 or higher or `clang` version 5 or higher.
* The [cmake][cmake] build system.
* A 64-bit operating system. Either Mac OS X or Linux are currently supported.
* For increased performance the processor of the system should support fast bit operations available in `SSE4.2`

Installation
------------

To download and install the library use the following commands.

```sh
git clone https://github.com/simongog/sdsl-lite.git
cd sdsl-lite
./install.sh
```

This installs the sdsl library into the `include` and `lib` directories in your
home directory. A different location prefix can be specified as a parameter of
the `install.sh` script:

```sh
./install /usr/local/
```

To remove the library from your system use the provided uninstall script:

```sh
./uninstall.sh
```

Getting Started
------------

To get you started with the library you can start by compiling the following
sample program which constructs 

```cpp
#include "tree_ext_sct.hpp"
#include "pq_types.hpp"
#include <fstream>
#include <memory>
#include <string>

using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

int main() {
	std::string s;
	std::cin >> s;
	std::vector<value_type> w(s.size()/2);
	auto a= *(std::min_element(w.begin(),w.end()));
	auto b= *(std::max_element(w.begin(),w.end()));
	for ( auto &x: w ) std::cin >> x;
	auto processor= std::make_unique<tree_ext_sct<node_type,size_type,value_type>>(s,w);
	std::cout << processor->count(0,n/2,(a+b)/2,b) << std::endl;
}
```

To compile the program using `g++` run:

```sh
g++ -std=c++17 -O2 -DNDEBUG -I ~/include -L ~/lib program.cpp -o program -lsdsl 
```

Next we suggest you look at the comprehensive [tutorial][TUT] which describes
all major features of the library or look at some of the provided [examples](examples).

Test
----

Implementing succinct data structures can be tricky. To ensure that all data
structures behave as expected, we created a large collection of unit tests
which can be used to check the correctness of the library on your computer.
The [test](./test) directory contains test code. We use [googletest][GTEST]
framework and [make][MAKE] to run the tests. See the README file in the
directory for details.

To simply run all unit tests after installing the library type

```sh
cd sdsl-lite/build
make test-sdsl
```

Note: Running the tests requires several sample files to be downloaded from the web
and can take up to 2 hours on slow machines.

Benchmarks
----------

To ensure the library runs efficiently on your system we suggest you run our
[benchmark suite](benchmark). The benchmark suite recreates a
popular [experimental study](http://arxiv.org/abs/0712.3360) which you can
directly compare to the results of your benchmark run.

Bug Reporting
------------

While we use an extensive set of unit tests and test coverage tools you might
still find bugs in the library. We encourage you to report any problems with
the library via the [github issue tracking system](https://github.com/simongog/sdsl-lite/issues)
of the project.

The Latest Version
------------------

The latest version can be found on the SDSL github project page https://github.com/simongog/sdsl-lite .

If you are running experiments in an academic settings we suggest you use the
most recent [released](https://github.com/simongog/sdsl-lite/releases) version
of the library. This allows others to reproduce your experiments exactly.

Licensing
---------

The SDSL library is free software provided under the GNU General Public License
(GPLv3). For more information see the [COPYING file][CF] in the library
directory.

We distribute this library freely to foster the use and development of advanced
data structure. If you use the library in an academic setting please cite the
following paper:

    @inproceedings{gbmp2014sea,
      title     = {From Theory to Practice: Plug and Play with Succinct Data Structures},
      author    = {Gog, Simon and Beller, Timo and Moffat, Alistair and Petri, Matthias},
      booktitle = {13th International Symposium on Experimental Algorithms, (SEA 2014)},
      year      = {2014},
      pages     = {326-337},
      ee        = {http://dx.doi.org/10.1007/978-3-319-07959-2_28}
    }

A preliminary version is available [here on arxiv][SEAPAPER].

## External Resources used in SDSL

We have included the code of two excellent suffix array
construction algorithms.

Additionally, we use the [googletest][GTEST] framework to provide unit tests.
Our visualizations are implemented using the [qcustomplot][QCUSTOMPLOT]-library.

Authors
--------

The main contributors to the library are:

* [Johannes Bader] (https://github.com/olydis)
* [Timo Beller](https://github.com/tb38)
* [Simon Gog](https://github.com/simongog) (Creator)
* [Matthias Petri](https://github.com/mpetri)

Contribute
----------

Are you working on a new or improved implementation of a succinct data structure?
We encourage you to contribute your implementation to the SDSL library to make
your work accessible to the community within the existing library framework.
Feel free to contact any of the authors or create an issue on the
[issue tracking system](https://github.com/simongog/sdsl-lite/issues).


[STL]: http://www.sgi.com/tech/stl/ "Standard Template Library"
[cmake]: http://www.cmake.org/ "CMake tool"
[MAKE]: http://www.gnu.org/software/make/ "GNU Make"
[gcc]: http://gcc.gnu.org/ "GNU Compiler Collection"
[clang]: https://clang.llvm.org/docs/
[LS]: http://www.sciencedirect.com/science/article/pii/S0304397507005257 "Larson &amp; Sadakane Algorithm"
[GTEST]: https://code.google.com/p/googletest/ "Google C++ Testing Framework"
[SDSLCS]: http://simongog.github.io/assets/data/sdsl-cheatsheet.pdf "SDSL Cheat Sheet"
[SDSLLIT]: https://github.com/simongog/sdsl-lite/wiki/Literature "Succinct Data Structure Literature"
[TUT]: http://simongog.github.io/assets/data/sdsl-slides/tutorial "Tutorial"
[QSUFIMPL]: http://www.larsson.dogma.net/qsufsort.c "Original Qsufsort Implementation"
[CF]: https://github.com/simongog/sdsl-lite/blob/master/COPYING "Licence"
[SEAPAPER]: http://arxiv.org/pdf/1311.1249v1.pdf "SDSL paper"
[DOXYGENDOCS]: http://algo2.iti.kit.edu/gog/docs/html/index.html "API Reference"
[GBENCH]: https://github.com/google/benchmark "Google Benchmark Micro-Benchmarking Framework"
[SHA512]: http://www.atwillys.de/content/cc/swlib-cc/include/sw/hash/sha512.hh "Open-Source implementation of SHA512"
[QCUSTOMPLOT]: "https://www.qcustomplot.com/" "QCustomPlot Qt library"
[QT]: https://www.qt.io/ "Qt"
[SDSL]: https://github.com/simongog/sdsl-lite "sdsl-lite"
[GLFAGS]: https://github.com/gflags/gflags "glfags"
