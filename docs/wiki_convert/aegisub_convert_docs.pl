#!/usr/bin/perl

########################
#
# aegisub_convert_docs.pl - downloads and converts the Aegisub documentation wiki to static HTML.
#
# Usage: aegisub_convert_docs.pl
# Will write the entire wiki to the current directory.
#
# Warning: ugly hacking inside.
#
#########
# 
# Written by Karl Blomster (TheFluff) 2008
# This script is hereby given into the public domain. Do whatever you want with it.
#
########################


# includes
use warnings;
use strict;
use utf8;
use LWP;
use URI;
# use CSS; # <--- fucking useless
use HTML::LinkExtor;
use File::Path;


# variables
my $base_dir = '.';
my $host = 'http://docs.aegisub.org';
my $docs_base_url = 'http://docs.aegisub.org/manual/';
my $agent = LWP::UserAgent->new();
my @process_pages = ($docs_base_url . 'Main_Page'); # start at the main page
my %pages = ($base_dir . '/Main_Page' => $docs_base_url . "Main_Page");
my %accepted_types = (
	'application/javascript' => '.js',
	'image/png' 	=> '.png',
	'image/jpeg' 	=> '.jpg',
	'image/gif' 	=> '.gif',
	'text/html'		=> '.html',
	'text/css'		=> '.css',
	'text/javascript' => '.js',
	'text/plain'	=> '.txt',
);
my $timestamp = time();
my ($requests, $writes);


# make sure we have somewhere to write to
mkdir($base_dir) or die("Couldn't create directory ", $base_dir, ": $!")
	unless ( -e $base_dir and -d $base_dir );
chdir($base_dir) or die("Couldn't change directory to ", $base_dir, ": $!");


print("Starting downloading and conversion of documentation wiki to ", $base_dir, "\n",
	"This will probably take a while.\n");


GETLOOP:
while ( @process_pages ) {
	my $current_page = shift(@process_pages);
	# initialize object and download the page
	my $page_object = $agent->get($current_page);
	print("Requesting ${current_page}...\n");

	# warn and skip if we couldn't get it
	unless ( $page_object->is_success() ) {
		warn("Couldn't get ", $current_page, ": ", $page_object->status_line(), ", skipping\n");
		next(GETLOOP);
	}

	my $content_type = $page_object->content_type();

	# skip if we don't know what it is
	unless ( exists($accepted_types{$content_type}) ) {
		warn("I don't know what to do with ", $content_type, ", skipping\n");
		next(GETLOOP);
	}

	# monstrously ugly hack to handle rewriting of .css filenames
	my $name;
	if ( $current_page =~ m!css$!i ) {
		$name = convert_css_link($current_page);
	} else {
		$name = convert_link($current_page);
	}

	# if it's html, parse it, grab new links
	# add them to @process_pages if they're not already in there
	# then write the modified html to disk
	if ( $content_type eq 'text/html' ) {
		my ($filename, $content, @newpages) = parse_and_fix_html($current_page, $page_object->base(), $page_object->decoded_content());

		# skip this page if the parser decided it was a page we don't want
		next(GETLOOP) unless ($content);

		write_to_disk($filename, $content_type, $content);

		foreach my $url (@newpages) {
			my $newname = convert_link($url);
			# check if we already added that page to our todo-list
			if ( exists($pages{$newname}) ) {
				next(); # we did, do nothing
			}
			else {
				# new page, add it to the list of things to process
				push(@process_pages, $url);
				$pages{$newname} = $url;
			}
		}
	}
	# if it's CSS we need the @import'ed links
	elsif ( $content_type eq 'text/css' ) {
		my @newpages = parse_css($current_page, $page_object->base(), $page_object->decoded_content());

		write_to_disk($name, $content_type, $page_object->decoded_content());

		foreach my $url (@newpages) {
			my $newname = convert_link($url);
			# check if we already added that page to our todo-list
			if ( exists($pages{$newname}) ) {
				next(); # we did, do nothing
			}
			else {
				# new page, add it to the list of things to process
				push(@process_pages, $url);
				$pages{$newname} = $url;
			}
		}
	}
	else {
		write_to_disk($name, $content_type, $page_object->decoded_content());
	}
}
continue {
	$requests++;
}


print("Done.\nMade $requests requests and wrote $writes files in ",
	time()-$timestamp, " seconds.\n");

exit(0);



##########################################


# parse out pages this page links to
# modify it
# return filename of the page, modified html and list of links
sub parse_and_fix_html {
	#get arguments
	my ($url, $base, $content) = @_;
	my (@links, @links_to_modify); # list of links to return later

	# strip RSS etc.
	$content =~ s!<link rel=[^>]*xml[^>]* />!!gi;

	# kill the topbar
	$content =~ s!<div id=\"topbar\".*?<\!-- end topbar -->!!s;

	# kill the article/discussion/history thing
	$content =~ s!<div id=\"p-cactions\".*?</div>!!s;

	# kill the "toolbox" at the bottom left
	$content =~ s!<div class=\"portlet\" id=\"p-tb\".*?(<\!-- end of the left)!$1!s;

	# kill "recent changes"
	$content =~ s!<li id=\"n-recentchanges\">.*?</li>!!;

	# parse HTML
	my $html = HTML::LinkExtor->new();
	$html->parse($content);
	$html->eof();

	# loop over the list of links
	# all real work is done here
	LINKLIST:
	foreach my $tag ( $html->links() ) {
		my ($tagname, %attrs) = @{$tag};

		my $quoted = quotemeta($docs_base_url);

		# does the link interest us?
		if ( ($tagname eq 'a') and exists($attrs{'href'}) ) {
			my $href = quotemeta($attrs{'href'}); # quotemeta?
			$href =~ s!\&!\&amp\;!g; # quotemeta kills &amp; things for some reason

			# skip and kill special or "edit" links
			if ( $attrs{'href'} =~ m!index\.php\?!i ) {
				$content =~ s!<a href=\"${href}\".*?>(.*?)</a>!$1!gi;
				next(LINKLIST);
			}
			# skip and kill image/special links
			if ( $attrs{'href'} =~ m!(Special\:|Image\:|Talk\:)!i ) {
				$content =~ s!<a.*?href\=\"${href}\".*?>(.*?)</a>!$1!gi;
				next(LINKLIST);
			}
			# change somepage#anchor links so they point to the right document,
			# but don't return them for further processing
			# BUG: if a page is ONLY referred to by this type of link, it won't get downloaded!
			# (highly unlikely)
			if ( $attrs{'href'} =~ m!.+\#(.*?)$! ) {
				push(@links_to_modify, $attrs{'href'});
				next(LINKLIST);
			}

			# does it go within aegisub.cellosoft.com?
			if ( $attrs{'href'} =~ m!^$quoted!i or (substr($attrs{'href'},0,1) eq '/') ) {
				push(@links_to_modify, $attrs{'href'});
			}
			# is not relative and goes somewhere else than aegisub.cellosoft.com
			# so we're not interested in it (#anchor links are not touched either)
			else { next(LINKLIST); }

			push(@links, URI->new_abs($attrs{'href'}, $base));
		}
		elsif ( ($tagname eq 'link') and exists($attrs{'href'}) ) {
			if ( $attrs{'href'} =~ m!^$quoted!i or (substr($attrs{'href'},0,1) eq '/') ) {
				push(@links_to_modify, $attrs{'href'});
			}
			else { next(LINKLIST); }

			push(@links, URI->new_abs($attrs{'href'}, $base));
		}
		elsif ( ($tagname eq 'script') and exists($attrs{'src'}) ) {
			my $src = quotemeta($attrs{'src'});

			# bogus link, skip it
			if ( $attrs{'src'} =~ m!index\.php\?title=-!i ) {
				next(LINKLIST);
			}

			if ( $attrs{'src'} =~ m!^$quoted!i or (substr($attrs{'src'},0,1) eq '/') ) {
				push(@links_to_modify, $attrs{'src'});
			}
			else { next(LINKLIST); }

			push(@links, URI->new_abs($attrs{'src'}, $base));
		}
		elsif ( ($tagname eq 'img') and exists($attrs{'src'}) ) {
			if ( $attrs{'src'} =~ m!^$quoted!i or (substr($attrs{'src'},0,1) eq '/') ) {
				# "flatten" image links
				my $flatlink = $attrs{'src'};
				$flatlink =~ s!/manual/images/.+/(.+?\.(jpg|gif|png))!${base_dir}/images/$1!i;
				$flatlink =~ s!/manual_real/images/.+/(.+?\.(jpg|gif|png))!${base_dir}/images/$1!i;
				my $quotedsrc = quotemeta($attrs{'src'});
				$content =~ s!$quotedsrc!$flatlink!;
			}
			else { next(LINKLIST); }

			push(@links, URI->new_abs($attrs{'src'}, $base));
		} 
		# else do nothing
	}

	# handle the @import links to get the css right
	while ( $content =~ m!\@import \"(.+?)\";!mg ) {
		my $importlink = $1;

		if ( convert_css_link($importlink) ) {
			push(@links, URI->new_abs($importlink, $base));
			push(@links_to_modify, '@' . $importlink);
		}
	}


	# rewrite all the links
	foreach my $link (@links_to_modify) {
		my $converted = convert_link($link);
		if ( substr($link,0,1) eq '@' ) {
			substr($link,0,1) = ''; 
		}
		$link = quotemeta($link);
		$content =~ s!\"$link\"!\"$converted\"!g;
	}

	$url =~ s!manual_real!manual!;
	$url =~ s!http://docs.aegisub.org!!;

	return($url, $content, @links);
}


sub write_to_disk {
	my ($path, $type, $thing) = @_;
	# return() if ( -e $path ); # this was a dumb idea

	$path =~ m!/(.*)/(.*?)$!;
	my ($tree, $filename) = ($1, $2);

	# I don't think this is necessary really
	mkpath($tree) unless ( -e $tree and -d $tree );

	if ( $type =~ m!^text! ) {
		write_text('.' . $path, $thing);
	}
	else {
		write_bin('.' . $path, $thing);
	}

	print("Writing $filename to ${path}...\n");

	$writes++;
}


sub write_text {
	my ($outfile, $thing) = @_;

	open(OUT, ">:utf8", $outfile) or die("Couldn't open $outfile for writing: $!");
	print OUT $thing;
	close(OUT) or die("Couldn't close ${outfile}: $!");

	return();
}


sub write_bin {
	my ($outfile, $thing) = @_;

	open(OUT, ">", $outfile) or die("Couldn't open $outfile for writing: $!");
	binmode(OUT);
	print OUT $thing;
	close(OUT) or die("Couldn't close ${outfile}: $!");

	return();
}


# converts links to relative starting with $base_dir
sub convert_link {
	my $link = shift(@_);

	# dereference if necessary
	if ( ref($link) ) {
		$link = $$link;
	}

	# SPECIAL CASE: it's one of those fukken @import links, do something else with it
	if ( substr($link,0,1) eq '@' ) {
		substr($link,0,1) = '';
		return(convert_css_link($link));
	}
	elsif ($link =~ /\.css$/) {
		return(convert_css_link($link));
	}
	elsif ($link =~ /\.js/) {
		return(convert_js_link($link));
	}

	$link =~ s!http://docs.aegisub.org!!;
	$link =~ s!/manual/images/.+/(.+?\.(jpg|gif|png))!/manual/images/$1!i;
	$link =~ s!/manual_real/images/.+/(.+?\.(jpg|gif|png))!/manual/images/$1!i;

	$link =~ s!manual_real/skins/.*?!manual/!;

	#print("link: $link\n");
	return($link);

	# is it relative?
	if ( substr($link,0,1) eq '/' ) {
		$link =~ s!^/docs/!$base_dir/!i;
	}
	else {
		my $quoted = quotemeta($host);
		$link =~ s!${quoted}/docs/!$base_dir/!i;
	}

	# if it doesn't have a filename extension it's probably a page,
	# and then we need to tack on .html to the end (fuck internet explorer)
	# oh and jfs's .lua pages aren't really lua scripts either
	my $bdirquoted = quotemeta($base_dir);
	if ( $link =~ m/^(${bdirquoted}.+?)\#.*$/ ) {
		my $pagename = $1;
		if ( $pagename !~ m!\.html$! or (substr($pagename,-4) eq '.lua') ) {
			$link =~ s!^${pagename}!${pagename}.html!;
		}
	} elsif ( $link !~ m!/.*?\.\w{2,4}$! or (substr($link,-4) eq '.lua') ) {
		$link = $link . '.html'; 
	}

	$link =~ s!\:!_!g; # replace : with _
	$link =~ s!\?!_!g; # replace ? with _

	return($link);
}

# HAX
sub convert_css_link {
	my $link = shift(@_);

	# does it seem like css?
	if ( $link =~ m!MediaWiki:(.+?)\.css!i ) {
		return("/manual/css/$1.css");
	}
	# has a sensible name already, don't fuck with it
	elsif ( $link =~ m!/([^/]+?)\.css$!i ) {
		return("/manual/css/$1.css");
	}
	# doesn't seem like anything useful
	else {
		print("UNKNOWN CSS: $link\n");
		return(undef);
	}
}

sub convert_js_link {
	my $link = shift(@_);
	$link =~ m!/([^/]+\.js)!;
	return("/manual/js/$1");
}

# argh
sub parse_css {
	my ($url, $base, $content) = @_;
	my @links;
	# my $quoted = quotemeta($docs_base_url); # <--- not used

	# find url("stuff") blocks
	LINKLIST:
	while ( $content =~ m!url\((\")?(.+?)(\")?\)!mgi ) {
		my $text = $2;

		# skip it if it's nonrelative and goes somewhere else than aegisub.cellosoft.com
		if ( $text =~ m!^http!i ) {
			# actually fuck this there shouldn't be any nonrelative links in there anyway
			next(LINKLIST)
				#unless ( $text =~ m!^${quoted}!i );
		}

		push(@links, URI->new_abs($text, $base));
	}

	return(@links);
}
