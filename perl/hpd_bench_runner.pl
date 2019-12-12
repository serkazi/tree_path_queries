#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

my $data_path = $ARGV[0];

my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/benchmarking/utils/";
my $executable = $target_path."hpd_bench";

my $command_to_run = "$executable --benchmark_counters_tabular=true $data_path";
printf("Running: %s\n",$command_to_run);
system($command_to_run);
