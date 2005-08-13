#!/usr/local/bin/perl

$relname = "muxsrc";

system("rm -rf $relname"); # Eeep!
system("cvs export -Dnow $relname");
open(FOO, "find $relname -print |");
while (<FOO>)
{
    chomp;
    if (/\/nicestuff\//)
    {
	# Always delete these
	unlink($_);
	next;
    }
    next if /.cvsmapfs/;
    next if /\.h$/;          # All include files
    next if /\.pl$/;         # All Perl scripts
    next if /\/bin\//;       # other fun stuff in bin, too
    next if /\/.cvsignore$/; # cvsignore
    next if /mux.general$/;  # general rules
    next if /generate_changelog$/;
    next if /\/hcode\/btech\//;# BT hcode
    print "Deleting: $_\n";
    unlink($_);
}
close(FOO);
system("tar cvfz limited.tgz muxsrc");
system("rm -rf muxsrc")
