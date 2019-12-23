#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

my $queries_to_filter= $ARGV[0];
my $data_path= $ARGV[1];
my $num_queries= $ARGV[2];
my $K_val= $ARGV[3];
my $res_name=$ARGV[4];

my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime(time);

my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/benchmarking/utils/";
my $res= $target_path.$res_name;
# by convention, we just supply the name of the file, and it is going to be stored inside the cmake-build dir
my $executable = $target_path."aggregate_bench";

my $command_to_run=
    "$executable ".
    "$data_path $num_queries $K_val ".
    "--benchmark_counters_tabular=true --benchmark_format=json --benchmark_out=${res} ".
    "--benchmark_filter=$queries_to_filter";
printf("Running: %s\n",$command_to_run);
system($command_to_run);
