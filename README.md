# Tree Path Queries
=========

What is it?
-----------
This library realizes the theoretical findings in published reasearch on path queries in weighted trees. It implements both
plain pointer-based as well as succinct data structures.

Requirements
------------

The library requires:

* A modern, `C++17` ready compiler such as `g++` version 5 or higher or `clang` version 5 or higher.
* The [cmake][cmake] build system.
* A 64-bit operating system. 
[//]: <> (* For increased performance the processor of the system should support fast bit operations available in `SSE4.2`)

[//]: <> (Dependencies)

### How to build the experiments with pre-generated queries
The target `aggregate_bench` benchmarks _all_ the data structures
against the supplied dataset, for the given query type and (when applicable --
for counting and reporting) parameter K.
The target is defined in `CMakeLists.txt` inside
`${PROJECT_SOURCE_DIR}/src/benchmarking/utils`.
To build: 
```sh
cmake from ${PROJECT_SOURCE_DIR}/build 
```
and then 
```sh
make aggregate_bench
```
### Perl script for running a benchmark
A wrapper around `aggregate_bench` is a Perl sctip `complete_queryset_benchmark.pl`.
```sh
perl complete_queryset_benchmark.pl <regex> <dataset_path> <num_of_queries> <K> <output_file_name>
```
where 
	* `<regex>` is a regular expression that is recognized by `googletest/googlebench`,
		which can be `tree_ext_ptr_reporting` for the `tree_ext_ptr` data structure
		in the above set, and `reporting` is the type of query;
		if one is willing to run all data structures
		for, say, `counting`, then one would simply omit the data structure part
		altogether and simply say `counting`;
	* `<dataset_path>` is the absolute path of a `*.puu`-file
	* `<num_of_queries>` is the number of queries to generate
	* `<K>` parametrizes the weight-range
	* `<output_file_name>` is a `json` filename to store the results
		- note however that specifying the output filename does not suppress `stdout`

### Commandline interface
To get a quick idea about the performance of a _single_ data structure,
on a given dataset, and for the given type of query (and again,
when applicable, for given `K`), one can also build
`cli_bench` target defined in 
`${PROJECT_SOURCE_DIR}/src/benchmarking/utils`.
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

[//]: <> (We provide an extensive set of documentation describing all data structures)
[//]: <> (and features provided by the library. Specifically we provide)

[//]: <> (* A [cheat sheet][SDSLCS] which succinctly describes the usage of the library.)
[//]: <> (* An doxygen generated [API reference][DOXYGENDOCS] which lists all types and functions of the library.)
[//]: <> (* A set of [example](examples/) programs demonstrating how different features of the library are used.)
[//]: <> (* A tutorial [presentation][TUT] with the [example code](tutorial/) using in the sides demonstrating all features of the library in a step-by-step walk-through.)
[//]: <> (* [Unit Tests](test/) which contain small code snippets used to test each library feature.)

Installation
------------

[//]: <> (To download and install the library use the following commands.

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
)
#### TODO
	[] install script
	[] uninstall script

Getting Started
------------

## Building the library
We use [cmake](`CMake`) as build system. The global `CMakeLists.txt` in the root
directory performs setup steps, such as finding necessary packages [gtest](`gtest`), [SDSL](`sdsl`), etc.
Directories sometimes have `CMakeLists.txt` of their own,
such as e.g. test targets in `${PROJECT_SOURCE_DIR}/src/test/`.
Building the library is a matter of (out-of-source build):
```sh
mkdir build && cd build && cmake ../
```
followed by `make <target>`, where `<target>` is target name defined in one `CMakeLists.txt`s.

### Input file format
For simplicity, at this point our data structures
accept the tree in the following format 
	- a balanced parentheses-encoding of the topology; and 
	- the weights in preorder.
We give such a file an extension `*.puu`.
#### TODO
	[] enable constructors accepting `std::istream &amp`; and
	[] read/write in binary format

### Sample program
To get you started with the library you can start by compiling the following
sample program which constructs a succinct tree extraction-based
data structure and counts the number of nodes on the path from `0` to `n/2`
with weight in `(a+b)/2` to `b`, where `a` and `b` are min/max weights in the supplied tree.

```cpp
#include "tree_ext_sct.hpp"
#include "pq_types.hpp"
#include <fstream>
#include <memory>
#include <string>

// all of these are essentially td::uint64_t
using node_type= pq_types::node_type;
using size_type= pq_types::size_type;
using value_type= pq_types::value_type;

int main() {
	std::string s;
	std::cin >> s; // read the topology -- BP sequence
	std::vector<value_type> w(s.size()/2); 
	for ( auto &x: w ) std::cin >> x; // read the weights
	auto a= *(std::min_element(w.begin(),w.end())), b= *(std::max_element(w.begin(),w.end()));
	// the constructor accepts topology and weights
	auto processor= std::make_unique<tree_ext_sct<node_type,size_type,value_type>>(s,w);
	// execute a counting query
	std::cout << processor->count(0,n/2,(a+b)/2,b) << std::endl;
}
```

### Generating a tree, uniformly at random
The syntax is as follows:
```sh
./gentree -n=42 -a=1 -b=10
```
outputs to `stdout` the tree in the above `*.puu` format.
`gentree`'s source is `${PROJECT_SOURCE_DIR}/src/misc/gentree_uar.cpp`.

[//]: <> (Next we suggest you look at the comprehensive [tutorial][TUT] which describes)
[//]: <> (all major features of the library or look at some of the provided [examples](examples).)
#### TODO
	[] add a commandline flag -- the output file (with default being `stdout`)


Test
----

To ensure that all data structures behave as expected, we created a large 
collection of unit tests which can be used to check the correctness of the library on your computer.
The [test](./test) directory contains test code. We use [googletest][GTEST]
framework and [make][MAKE] to run the tests. 
See the README file in the directory for details.

[//]: <> (To simply run all unit tests after installing the library type)

[//]: <> (```sh
cd sdsl-lite/build
make test-sdsl
```
)

[//]: <> (Note: Running the tests requires several sample files to be downloaded from the web
and can take up to 2 hours on slow machines.)

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

[//]: <> (The Latest Version
------------------

The latest version can be found on the SDSL github project page https://github.com/simongog/sdsl-lite .

If you are running experiments in an academic settings we suggest you use the
most recent [released](https://github.com/simongog/sdsl-lite/releases) version
of the library. This allows others to reproduce your experiments exactly.)

Licensing
---------

The library is free software provided under the GNU General Public License (GPLv3). 
For more information see the [COPYING file][CF] in the library directory.

We distribute this library freely to foster the use and development of advanced data structures. 
A preliminary version of the paper this code primarily used in is available [here on arxiv][SEAPAPER].

## External Resources used in the library

We use the 
	* [sdsl][SDSL] for our succinct data structures
	* [googletest][GTEST] framework to provide unit tests
	* [google-benchmark][GBENCH] for time measurements
	* [malloc\_count][MCNT] for measuring space occupancy
	* [gflags][GLFAGS] to define and handle commandline flags for CLI interface
For optional functionality, we also use
	* [json][JSON] for ease of reporting
	* [sha512][SHA512] to facilitate the sanity check of the data structures
		- by creating a hash of the answers to the queries in the query set
In addition, we have GUI based on 
	* [Qt][QT]; and
	* [QCustomPlot][QCUSTOMPLOT] library for plotting
The GUI functionality is currently work in progress.

Authors
--------

The main contributors to the library are:
* [Serikzhan Kazi](https://github.com/serkazi) (Creator)

[//]: <> (* [Johannes Bader] (https://github.com/olydis))
[//]: <> (* [Timo Beller](https://github.com/tb38))
[//]: <> (* [Simon Gog](https://github.com/simongog) (Creator))
[//]: <> (* [Matthias Petri](https://github.com/mpetri))

Contribute
----------

Are you working on a new or improved implementation of a path queries data structure?
We encourage you to contribute your implementation to the our library to make
your work accessible to the community within the existing library framework.
Feel free to contact any of the authors or create an issue on the
[issue tracking system](https://github.com/serkazi/tree_path_queries/issues).


Caveats
------------
[sdsl-lite][SDSL] comes with its own version of [gtest][GTEST], which can be way too old
for our tests, which use newest (as of time of writing) features of the latter.
(Such as e.g. value-parametrized tests).
This may result in problems when trying to test data structures depending on
`sdsl` (i.e. which link to `sdsl`). The workaround is to disable building
`gtest` inside `sdsl` -- e.g. via appropriate edits inside `CMakeLists.txt` of `sdsl` 
distribution. Essentially, one can to comment-out the lines pertaining
to building `gtest` as an external project and use `cmake`'s `find_packge/find_library` instead
(pointing to your `gtest` distribution if necessary).


[STL]: http://www.sgi.com/tech/stl/ "Standard Template Library"
[cmake]: http://www.cmake.org/ "CMake tool"
[MAKE]: http://www.gnu.org/software/make/ "GNU Make"
[gcc]: http://gcc.gnu.org/ "GNU Compiler Collection"
[clang]: https://clang.llvm.org/docs/
[LS]: http://www.sciencedirect.com/science/article/pii/S0304397507005257 "Larson &amp; Sadakane Algorithm"
[MCNT]: https://github.com/bingmann/malloc_count "malloc_count"
[GTEST]: https://code.google.com/p/googletest/ "Google C++ Testing Framework"
[SDSLCS]: http://simongog.github.io/assets/data/sdsl-cheatsheet.pdf "SDSL Cheat Sheet"
[SDSLLIT]: https://github.com/simongog/sdsl-lite/wiki/Literature "Succinct Data Structure Literature"
[TUT]: http://simongog.github.io/assets/data/sdsl-slides/tutorial "Tutorial"
[CF]: https://github.com/simongog/sdsl-lite/blob/master/COPYING "Licence"
[//]: <> ([SEAPAPER]: http://arxiv.org/pdf/1311.1249v1.pdf "SDSL paper")
[SEAPAPER]: https://arxiv.org/submit/3022354/view "Path Query Data Structures in Practice"
[DOXYGENDOCS]: http://algo2.iti.kit.edu/gog/docs/html/index.html "API Reference"
[GBENCH]: https://github.com/google/benchmark "Google Benchmark Micro-Benchmarking Framework"
[SHA512]: http://www.atwillys.de/content/cc/swlib-cc/include/sw/hash/sha512.hh "Open-Source implementation of SHA512"
[QCUSTOMPLOT]: "https://www.qcustomplot.com/" "QCustomPlot Qt library"
[QT]: https://www.qt.io/ "Qt"
[SDSL]: https://github.com/simongog/sdsl-lite "sdsl-lite"
[GLFAGS]: https://github.com/gflags/gflags "glfags"
[JSON]: https://github.com/nlohmann/json "JSON for Modern C++"
