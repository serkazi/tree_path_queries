use warnings FATAL => 'all';
use strict;

my $bitmask = $ARGV[0];

my $data_path = "/users/grad/kazi/CLionProjects/tree_path_queries/data/";
my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/tests/experiments/";
my $executable = $target_path."observe_memusage";

# my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=3 --time-unit=ms --detailed-freq=5 --massif-out-file=outfile ".$executable ." ".$data_path."input";
my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=3  --max-snapshots=1000 --time-unit=ms --detailed-freq=5 --massif-out-file=outfile ".$executable ." ".$data_path."input bitmask";

opendir my $dir, $data_path or die "Cannot open directory: $!";
my @files = readdir $dir;
closedir $dir;

printf("Read the bitmask %d\n",$bitmask);

foreach(@files) {

    if ( $_ =~ /.*\.txt/ && $_ =~ /sqrt/ ) {
    # if ( $_ =~ /.*\.txt|.*\.puu/ ) {
        my $vg= $valgrind_command;
        my $output_file=$target_path.$_.".massif.out";
        $vg =~ s/outfile/$target_path$_.massif.out/g;
        $vg =~ s/input/$_/g;
        $vg =~ s/bitmask/$bitmask/g;
        # printf("%s\n",$vg);
        system($vg);
        system("cp $output_file .");
    }
}

use Cwd qw();
my $path = Cwd::cwd();
print "$path\n";
