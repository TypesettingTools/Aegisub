#/usr/bin/perl

#########
#
# Written by Karl Blomster (TheFluff) 2008.
# (OK, mostly just a translation of utils-auto4.lua.)
# 
# This script is hereby given into the public domain.
# If that is not possible according to local laws, I, the author, hereby grant
# anyone the right to use this script for any purpose.
#
#########

package Auto4Utils;
require Exporter;

use warnings;
use strict;
use feature ":5.10";
use utf8; # just to be safe
use POSIX (); # gah, we only need floor(), no need to import all of IEEE 1003.1


# Export everything by default
our @ISA 		= qw(Exporter);
our @EXPORT		= qw(extract_color alpha_from_style color_from_style HSV_to_RGB HSL_to_RGB interpolate_color interpolate_alpha
	ass_color ass_alpha ass_style_color string_trim clamp interpolate);


# Given 3 integers R,G,B, returns ASS formatted &HBBGGRR& string
sub ass_color {
	my ($r, $g, $b) = @_;
	return(sprintf("&H%02X%02X%02X&", $b, $g, $r));
}
# Perlier version of that:
# sub ass_color { return sprintf "&H%02X%02X%02X&", reverse }
	# I don't think reverse reverses @_ by default, rats :(

# Convert decimal alpha value to &H00& form
sub ass_alpha {
	return(sprintf("&H%02X&", shift(@_)));
}

# Given 4 integers R,G,B,A, returns a v4+ formatted style color string
# (note no terminating &)
sub ass_style_color {
	my ($r, $g, $b, $a) = @_;
	return(sprintf("&H%02X%02X%02X%02X", $a, $b, $g, $r));
}

# Tries its best to convert a string to 4 integers R,G,B,A.
# Returns them in that order if it succeeds, or undef if it can't do it.
# Useless in scalar context.
sub extract_color {
	my $string = shift(@_);
	
	# This here thingie is a switch statement. Magic!
	given ( $string ) {
		# try v4+ style (ABGR)
		when ( /\&H([[:xdigit:]]{2})([[:xdigit:]]{2})([[:xdigit:]]{2})([[:xdigit:]]{2})/i ) {
			return(hex($4), hex($3), hex($2), hex($1));
		}
		# color override? (BGR)
		when ( /\&H([[:xdigit:]]{2})([[:xdigit:]]{2})([[:xdigit:]]{2})\&H/i ) {
			return(0, hex($3), hex($2), hex($1));
		}
		# alpha override? (A)
		# (bug: bogus results with \c&H<hex>& with the first four zeros omitted)
		when ( /\&H([[:xdigit:]]{2})\&/i ) {
			return(hex($1), 0, 0, 0);
		}
		# try HTML format for laffs (RGB)
		when ( /\#([[:xdigit:]]{2})([[:xdigit:]]{2})?([[:xdigit:]]{2})?/i ) {
			return(0, (hex($2) or 0), (hex($2) or 0), (hex($3) or 0));
		}
		default {
			return(undef, undef, undef, undef);
		}
	}
}

# Given a a style color string, returns the alpha part formatted as override
sub alpha_from_style {
	my $color_string = shift(@_);
	my ($r, $g, $b, $a) = extract_color($color_string);
	return(ass_alpha($a or 0));
}

# Given a style color string, returns the color part formatted as override
sub color_from_style {
	my $color_string = shift(@_);
	my ($r, $g, $b, $a) = extract_color($color_string);
	return(ass_color(($r or 0), ($g or 0), ($b or 0)));
}


# Converts 3 integers H, S, V (hue, saturation, value) to R, G, B
sub HSV_to_RGB {
	my ($H, $S, $V) = @_;
	my ($r, $g, $b);
	
	# saturation is zero, make grey
	if ($S == 0) {
		$r = $V * 255;
		$r = clamp($r, 0, 255);
		($g, $b) = ($r, $r);
	}
	# else calculate color
	else {
		# calculate subvalues
		$H 		= $H % 360; # put $h in range [0,360]
		my $Hi	= POSIX::floor($H/60);
		my $f	= $H/60 - $Hi;
		my $p	= $V * (1 - $S);
		my $q	= $V * (1 - $f * $S);
		my $t	= $V * (1 - (1 - $f) * $S);
		
		# do math based on hue index
		if 		($Hi == 0)	{ $r = $V*255; $g = $t*255; $b = $p*255; }
		elsif	($Hi == 1)	{ $r = $q*255; $g = $V*255; $b = $p*255; }
		elsif	($Hi == 2)	{ $r = $p*255; $g = $V*255; $b = $t*255; }
		elsif	($Hi == 3)	{ $r = $p*255; $g = $q*255; $b = $V*255; }
		elsif	($Hi == 4)	{ $r = $t*255; $g = $p*255; $b = $V*255; }
		elsif	($Hi == 5)	{ $r = $V*255; $g = $p*255; $b = $q*255; }
		# TODO: replace this with Aegisub::Script::debug_out() or whatever it is
		else	{ warn("HSV_to_RGB: Hi got an unexpected value: $Hi"); }
	}
	
	$r = POSIX::floor($r);
	$g = POSIX::floor($g);
	$b = POSIX::floor($b);
	return($r, $g, $b);
}


# Converts 3 integers H, S, L (hue, saturation, luminance) to R, G, B
# NOTE: THE OUTPUT AND S,V INPUT IS IN THE RANGE [0,1]!
# Routine is best performed to "The HSL Song" by Diablo-D3 and the #darkhold idlers.
# The lyrics are as follows:
# I see a little silluetto of a man 
# It's in color, its in color, can you convert to HSL?
# Cyan, yellow and magenta, very very outdated now 
# Alvy Smith, Alvy Smith, Alvy Smith, Alvy Smith, Fiigaarrooo
# I'm just a poor boy, stuck with RGB
#	(He's just a poor boy, from a poor colorspace, spare him his eyes from this monstrosity)
#
# Easy come, easy go, will you let me HSL?
#    (No! We will not let you HSL!)
# Let him HSL!
#    (No! We will not let you HSL!)
# Let him HSL!
#    (No! We will not let you HSL!)
# Let me HSL!
#    (Will not HSL!)
# Let me HSL!
#    (Will not HSL!)
# Let me HSL! Let me HSL!
#    (Never never never never never!)
# Let me HHHHHSSSSSLLLLL!
#    (No no no no no no no!)
#
# [70's rock/bad humour segment ends here. We now return to your regularily scheduled Perl hacking...]
sub HSL_to_RGB {
	my ($H, $S, $L) = @_;
	my ($r, $g, $b, $Q);
	
	# make sure input is in range
	$H = $H % 360;
	$S = clamp($S, 0, 1);
	$L = clamp($L, 0, 1);
	
	# simple case if saturation is 0, all grey
	if ($S == 0) {
		($r, $g, $b) = ($L, $L, $L);
	}
	# more common case, saturated color
	else {
		if ($L < 0.5)	{ $Q = $L * (1 + $S); }
		else { $Q = $L + $S - ($L * $S); }
		
		my $P = 2 * $L - $Q;
		my $Hk = $H / 360;
		
		my ($Tr, $Tg, $Tb);
		$Tg = $Hk;
		if 		($Hk < 1/3) { $Tr = $Hk + 1/3; $Tb = $Hk + 2/3; }
		elsif	($Hk > 2/3) { $Tr = $Hk - 2/3; $Tb = $Hk - 1/3; }
		else 				{ $Tr = $Hk + 1/3; $Tb = $Hk - 1/3; }
		
		# anonymous subroutine required for closure reasons
		my $get_component = sub {
			my $T = shift(@_);
			if		($T < 1/6)					{ return($P + (($Q - $P) * 6 * $T)) }
			elsif	(1/6 <= $T and $T < 1/2)	{ return($Q) }
			elsif	(1/2 <= $T and $T < 2/3)	{ return($P + (($Q - $P) * (2/3 - $T) * 6)) }
			else	{ return($P) }
		};
		
		$r = $get_component->($Tr);
		$g = $get_component->($Tg);
		$b = $get_component->($Tb);
	}
	
	return($r, $g, $b);
}


# Removes whitespace at the start and end of a string
# (will anyone ever use this in a perl program?)
sub string_trim {
	my $string = shift(@_);
	$string =~ s!^\s*(.+?)\s*$!$1!;
	return($string);
}


# Clamp a numeric value to a range
sub clamp {
	my ($val, $min, $max) = @_;
	if		($val < $min) { return($min) }
	elsif	($val > $max) { return($max) }
	else	{ return($val) }
}

# interpolate between two numbers
sub interpolate {
	my ($pct, $min, $max) = @_;
	if 		($pct <= 0) { return($min) }
	elsif	($pct >= 1) { return($max) }
	else	{ return($pct * ($max - $min) + $min) }
}

# interpolate between two color values (given as &HBBGGRR strings)
# returns string formatted with \c&H override format
sub interpolate_color {
	my ($pct, $start, $end) = @_;
	my ($r1, $g1, $b1) = extract_color($start);
	my ($r2, $g2, $b2) = extract_color($end);
	my ($r, $g, $b)	= 
		(interpolate($pct, $r1, $r2), interpolate($pct, $g1, $g2), interpolate($pct, $b1, $b2));
	return(ass_color($r, $g, $b));
}

# interpolate between two alpha values (given as either override or part of style color strings)
# returns string formatted with \c&H override format
sub interpolate_alpha {
	my ($pct, $start, $end) = @_;
	my ($r1, $g1, $b1, $a1) = extract_color($start);
	my ($r2, $g2, $b2, $a2) = extract_color($end);
	return(ass_alpha(interpolate($pct, $a1, $a2)));
}