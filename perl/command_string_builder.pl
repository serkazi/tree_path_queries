use strict;
use warnings;

# perl complete_queryset_benchmark.pl counting /users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/ant.ice.mst.bas.puu $N 1 resfile01.json
# this script creates strings of the type above, to automate running the benchmarks
my $R="1000000";
my $N="1000000";

# my $datastructure= $ARGV[0];
my $patt="(nv|nv_lca|tree_ext_ptr|wt_hp_ptr)";

my @qtypes = ('counting','median','reporting');
# my @qtypes = ('reporting');
# my @qtypes = ('counting','median');
my @ks= ('1','10','100');
my $executable = "complete_queryset_benchmark.pl";

# my @datasets = ( 'eu.mst.osm.puu','eu.d.mst.dimacs.puu','eu.emst.dem.puu','mars.emst.dem.puu','us.rd.d.dfs.dimacs.puu');
my @datasets = ( 'eu.mst.osm.puu','eu.d.mst.dimacs.puu','eu.emst.dem.puu','mars.emst.dem.puu');
# my @datasets = ('rnd.100mln.sqrt.puu');
# my @datasets = ('eu.d.mst.dimacs.puu','eu.emst.dem.puu','eu.mst.osm.puu','mars.emst.dem.puu');
my $dataset_path = "/users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/";

for my $j (0 .. $#qtypes) {
	my $q = $qtypes[$j];
	for my $i (0 .. $#datasets) {
		my $ds = $datasets[$i];
		if ( $q ne "median" ) {
			foreach(@ks) {
				my $outfile = "resfile_".$q."_K_".$_."_".$ds.".json";
				my $comm = "";
				if ( $q ne "reporting" ) {
						# $comm = create_command_string($patt."_".$q,$ds,$N,$_,$outfile);
					$comm = create_command_string($q,$ds,$N,$_,$outfile);
				} else {
						# $comm = create_command_string($patt."_".$q,$ds,$R,$_,$outfile);
					$comm = create_command_string($q,$ds,$R,$_,$outfile);
				}
				printf("%s\n",$comm);

			}
		}
		else {
			my $outfile = "resfile_".$q."_".$ds.".json";
			# my $comm = create_command_string($patt."_".$q,$ds,$N,"1",$outfile);
			my $comm = create_command_string($q,$ds,$N,"1",$outfile);
			printf("%s\n",$comm);
		}
	}
}

sub create_command_string {
	my $comm = "perl ".$executable." ".$_[0]." ".$dataset_path.$_[1]." ".$_[2]." ".$_[3]." ".$_[4];
	return $comm;
}

