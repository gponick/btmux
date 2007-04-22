#!/usr/bin/perl

# Cleans mess made by makedepend(1) by eliminating system includes from
# the file, and generally prettifying the output.

$lfn = "";
$lft = "";
$lc=0;
$nlc=0;

sub
maybeprint
{
    if ($lfn ne "")
    {
	print "$lfn: $lft\n";
	$nlc++;
    }
    $lfn="";
    $lft="";
}

sub
update
{
    local($fn,$dep)=@_;
    if ($lfn eq $fn)
    {
	$lft .= " $dep";
	return;
    }
    maybeprint;
    $lfn=$fn;
    $lft=$dep;
}
print "# Cleaned by MakeDepCleaner " . scalar(localtime) . "\n";
while (<>)
{
    chomp;
    s/ \/\S+//g;
    # Strip absolute paths
    if (/^(\S+): (\S.*)$/)
    {
	update($1,$2);
    }
    $lc++;
#    print "$_\n";
}
maybeprint;
print "# Original lines: $lc - New lines: $nlc\n";
