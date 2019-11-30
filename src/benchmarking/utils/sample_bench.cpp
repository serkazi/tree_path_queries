//
// Created by kazi on 2019-11-25.
//
#include <benchmark/benchmark.h>
#include <fstream>
#include <memory>
#include "path_query_processor.hpp"
#include "pq_request.hpp"

static void BM_StringCreation(benchmark::State& state) {
    for (auto _ : state)
        std::string empty_string;
}
// Register the function as a benchmark
BENCHMARK(BM_StringCreation);

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
    std::string x = "hello";
    for (auto _ : state)
        std::string copy(x);
}
BENCHMARK(BM_StringCopy);

static void BM_serve_requests( benchmark::State& state ) {
    std::ifstream is("/users/grad/kazi/CLionProjects/tree_path_queries/data/linear_small_weights.txt");
    std::string s; is >> s;
    std::vector<int> w(s.size()/2);
    for ( auto &x: w ) is >> x;
    std::unique_ptr<path_query_processor<int,int,int>> p
        = std::make_unique<path_query_processor<int,int,int>>(s,w);
    path_queries::request_stream<int,int,int> stream;
}

BENCHMARK_MAIN();
