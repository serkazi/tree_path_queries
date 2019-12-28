#!/usr/bin/perl
use strict;
use warnings FATAL => 'all';

my $bitmask= $ARGV[0];
my $data_path= $ARGV[1];
my $dataset="";

if ( $data_path =~ /([^\/]+)$/ ) {
   $dataset= "$1";
}

my $target_path = "../cmake-build-debug/src/tests/experiments/";
my $executable = $target_path."observe_memusage";

# my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=3 --time-unit=ms --detailed-freq=5 --massif-out-file=outfile ".$executable ." ".$data_path."input";
my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=3 --max-snapshots=1000 ".
                      "--time-unit=ms --detailed-freq=100 --massif-out-file=outfile ".$executable.
                      " input bitmask";

printf("Read the bitmask %d\n",$bitmask);

my $vg= $valgrind_command;
my $output_file=$target_path.$dataset.".massif.out"; # massif-visualizer wants to see "massif.out." prefix
$vg =~ s/outfile/$output_file/g;
$vg =~ s/input/$data_path/g;
$vg =~ s/bitmask/$bitmask/g;
 # printf("%s\n",$vg);
system($vg);
system("cp $output_file .");

# use Cwd qw();
# my $path = Cwd::cwd();
# print "$path\n";
