#!/usr/bin/perl

use strict;

my @vec;
my $xor = 0;

while (<STDIN>) {
    foreach my $tok (/(\S+)/g) {
	if ($tok =~ /^@([0-9a-fA-F]+)/) {
	    push(@vec, (0) x (hex($1) - @vec));
	}
	else {
	    push(@vec, $tok);
	}
    }
}

foreach my $i (0 .. $#vec) {
    $xor ^= ($i * hex($vec[$i])) & 0xffffffff;
}

print(@ARGV, "{".join(",\n    ", map { "0x$_" } @vec)."}; /* $xor */\n");
