use warnings FATAL => 'all';
use strict;

opendir my $dir, "/users/grad/kazi/CLionProjects/tree_path_queries/data" or die "Cannot open directory: $!";
my @files = readdir $dir;
closedir $dir;

foreach(@files) {
    if ( $f_ =~ /*.txt/ ) {
        printf("%s\n", $_);
    }
}

