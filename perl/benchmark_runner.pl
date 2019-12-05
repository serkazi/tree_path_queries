#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

my $queries_to_filter= $ARGV[0];
my $data_path= $ARGV[1];

my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/benchmarking/utils/";
my $executable = $target_path."hunch_bench";

system("$executable --benchmark_filter=$queries_to_filter $data_path");
