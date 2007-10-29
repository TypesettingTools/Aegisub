#!/usr/bin/perl

#############################
# unicodereplace.pl - replaces high codepoints in infile with \uHEX escapes
####
# Usage: unicodereplace.pl infile outfile
#############################

use warnings;
use strict;
use utf8;

my ($infile, $outfile) = @ARGV; # get arguments

open(INFILE, "<:utf8", $infile) or die("Can't open $infile: $!");
open(OUTFILE, ">:utf8", $outfile) or die("Can't open $outfile: $!");

# loop over lines in the infile
while (<INFILE>) {
	for (split('', $_)) { 									# loop over characters in the line
		my $cp = ord;
		if ($cp < 127 or $cp == 0xFFFE or $cp == 0xFEFF) {  # is it a normal ASCII codepoint or a BOM?
			print OUTFILE $_;								# then pass through unchanged
		} 
		else {
			print OUTFILE "\\u", sprintf("%04x", $cp);  	# otherwise print as \uHEX
		}
	}
}
