use strict;
use warnings;

my $inFile = shift;
my $outFile = shift;

open(my $in, "< $inFile") or die("Can't open input file \"$inFile\": $!");
open(my $out, "> $outFile") or die("Can't open output file \"$outFile\": $!");

print $out <<RAW;
#include "StdAfx.h"

#include "Globals.h"
#include <stdio.h>

void FullHelp()
{
RAW

while (<$in>)
{
	last if /<end of help text>/i;
	chomp;
	s!\\!\\\\!g;
	s/"/\\"/g;
	s/%/%%/g;
	print $out "_tprintf(_T(\"$_\\n\"));\n";
}

print $out "}\n";

close $in;
close $out;

