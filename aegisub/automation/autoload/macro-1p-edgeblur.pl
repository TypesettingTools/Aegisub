#!/usr/bin/perl

use warnings;
use strict;
use Aegisub; # don't forget this

$script_name = "Add/remove edgeblur macro (Perl version)";
$script_description = "A demo macro showing how to do simple macros in Auto4-Perl";
$script_author = "Karl Blomster";
$script_version = "1";


# this is a line-by-line translation of the Lua macro of the same name.
sub add_edgeblur {
	# get the arguments; they are all references (important later)
	my ($subtitles, $selected_lines, $active_line) = @_;
	
	# loop over the selected lines (note the dereferencing)
	foreach my $lineno ( @{$selected_lines} ) {
		# $line now contains a reference to the line we're working on.
		# Note that the "line" is actually a hash with the dialogue line fields as keys.
		my $line = $subtitles->[$lineno];
		# Tack on {\be1} to the start of the "text" field...
		$line->{"text"} = '{\\be1}' . $line->{"text"};
		# And write our $line back to the file.
		$subtitles->[$lineno] = $line;
	}
	
	# This ain't implemented yet :(
	# Aegisub::Script::set_undo_point("Add edgeblur");
}


# This routine is NOT a Lua translation and may therefore seem more perlish. :>
sub remove_edgeblur {
	# same as above
	my ($subtitles, $selected_lines, $active_line) = @_;
	
	foreach my $lineno ( @{$selected_lines} ) {
		# Since we're only going to change the text field of the line,
		# why bother copying the entire line? We copy only the text field instead.
		# We could also do stuff directly on $subtitles->[$lineno]->{"text"} but
		# that's too long to write and is also risky if you blow something up.
		my $text = $subtitles->[$lineno]->{"text"};
		
		# remove any \be1 tags contained in the first {} block
		$text =~ s!^\{(.*?)\\be1(.*?)\}!\{${1}${2}\}!;
		# if that leaves nothing in it, remove it
		$text =~ s!^\{\}!!;
		
		# write back
		$subtitles->[$lineno]->{"text"} = $text;
	}
	
	# Still not implemented :/
	# Aegisub::Script::set_undo_point("Remove edgeblur");
}


# Register macros with Aegisub
Aegisub::Script::register_macro("Add edgeblur (Perl)", "Adds \\be1 tags to all selected lines", \&add_edgeblur);
Aegisub::Script::register_macro("Remove edgeblur (Perl)", "Removes \\be1 tags from the start of all selected lines", \&remove_edgeblur);