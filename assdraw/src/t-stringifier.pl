#!/usr/bin/perl

###########################
# t-stringifier.pl - replaces "strings" with _T("strings")
#
# Usage: t-stringifier.pl file1 [file2 [file...]]
# NOTE: changes existing files in place so backup stuff or rewrite the script
# (more specifically comment out the rename() call at the end of the main loop)
# if you're paranoid.
#
# Written by Karl Blomster 2008
# This script is in the public domain.
###########################


use warnings;
use strict;

my @infiles = @ARGV;

foreach my $infile (@infiles) {
	my $outfile = $infile . ".out";
	open(INFILE, "<", $infile) or die("Couldn't open $infile for reading: $!");
	open(OUTFILE, ">", $outfile) or die("Couldn't open $outfile for writing: $!");
	
	print("Processing: $infile \n");

	while (<INFILE>) {
		print OUTFILE $_ and next() if (m!^#\s*include!);
		
		my $line = $_;
		
		$line =~ s/(_T\(|_\(|wxT\()?"(.*?)(?<!\\)"(\))?/replacementstring($1,$2,$3)/eg;

		print OUTFILE $line;
	}
	
	close(OUTFILE);
	close(INFILE);

	
	rename($outfile,$infile) or die("Couldn't overwrite ${infile}: $!");
}

sub replacementstring {
	my ($pre, $string, $post) = @_;

	if ($pre) {
		if ($pre eq "_(") { return( qq!_("${string}")! ); }
		else { return( qq!_T("${string}")! ); }
	}
	elsif ($post) {
		return( qq!_T("${string}"))! );
	}
	else {
		return( qq!_T("${string}")! );
	}
}
