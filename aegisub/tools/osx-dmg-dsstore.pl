#!/usr/bin/perl -w

use Mac::Finder::DSStore qw( writeDSDBEntries makeEntries );
use Mac::Memory qw( );
use Mac::Files qw( NewAliasMinimal );

&writeDSDBEntries("$ARGV[0]",
    &makeEntries(".",
        BKGD_alias => NewAliasMinimal("$ARGV[2]"),
        ICVO => 1,
        fwi0_flds => [ 308, 397, 658, 848, "icnv", 0, 0 ],
        fwvh => 350,
        icgo => "\0\0\0\0\0\0\0\0",
        icvo => pack('A4 n A4 A4 n*', "icv4", 90, "none", "botm", 0, 0, 0, 0, 0, 1, 0, 100, 1),
        icvt => 12
    ),
    &makeEntries("Applications", Iloc_xy => [ 133, 250 ]),
    &makeEntries("$ARGV[1]", Iloc_xy => [ 133, 55 ])
);
