use warnings FATAL => 'all';
use strict;

my $data_path = "/users/grad/kazi/CLionProjects/tree_path_queries/data/";
my $target_path = "/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/tests/experiments/";
my $executable = $target_path."observe_memusage";

my $valgrind_command= "valgrind --tool=massif --heap=yes --stacks=no --depth=32 --time-unit=ms --detailed-freq=5 --massif-out-file=outfile ".$executable ." ".$data_path."input";

opendir my $dir, $data_path or die "Cannot open directory: $!";
my @files = readdir $dir;
closedir $dir;

foreach(@files) {
    if ( $_ =~ /.*\.txt|.*\.puu/ ) {
        my $vg= $valgrind_command;
        $vg =~ s/outfile/$target_path$_.massif.out/g;
        $vg =~ s/input/$_/g;
        # printf("%s\n",$vg);
        system($vg)
    }
}

use Cwd qw();
my $path = Cwd::cwd();
print "$path\n";
