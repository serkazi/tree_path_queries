#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';
use Term::ANSIColor;

if ($#ARGV != 1 ) {
    print "usage: ./<script> <bitmask> <dataset_path>";
    exit;
}

# instantiates given data structures (listed in a bitmask) for the given dataset
# for example, bitmask = 24 would instantiate ext_ptr and hybrid data structures
# the purpose is to make sure the data structures are constructible and fit into memory
my $data_path = $ARGV[1];
my $bitmask= $ARGV[0];
my $dataset= $data_path;
$dataset =~ m/([a-zA-Z0-9.]+)$/;
$dataset=$1;

print color('green');
print "match: $1\n";
print color('reset');

printf("bitmask = %d\n",$bitmask);
printf("path: %s\n",$data_path);

my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/tests/experiments/";
my $executable = $target_path."observe_memusage";
# my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=3 --max-snapshots=1000 --time-unit=ms --detailed-freq=100 --massif-out-file=outfile ".$executable ." ".$data_path." bitmask";
my $command= $executable ." ".$data_path." bitmask";
my $vg= $command;
$vg =~ s/bitmask/$bitmask/g;

print color('red');
printf("Running the command: %s\n",$vg);
print color('reset');

system($vg);
