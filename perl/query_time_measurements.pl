#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

if ($#ARGV != 1 ) {
    print "usage: ./<script> <bitmask> <num_of_queries>";
    exit;
}

my $bitmask = $ARGV[0];
my $num_queries= $ARGV[1];

# std::cerr << "usage: ./<this_binary> <input_file> <bitmask> <num_of_queries>" << std::endl;
my $data_path = "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";
my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/tests/experiments/";
my $executable = $target_path."test_experiment_utils";
my $command_to_run= $executable ." ".$data_path."input bitmask num_of_queries >> results.out";
my $results=$target_path."results.out";
system("rm $results");

opendir my $dir, $data_path or die "Cannot open directory: $!";
my @files = readdir $dir;
closedir $dir;

foreach(@files) {
    if ( $_ =~ /.*\.puu/ ) {
        # if ( $_ =~ /.*\.txt|.*\.puu/ ) {
        my $vg= $command_to_run;
        $vg =~ s/input/$_/g;
        $vg =~ s/bitmask/$bitmask/g;
        $vg =~ s/num_of_queries/$num_queries/g;
        system($vg);
    }
    system("cp $results .");
}



