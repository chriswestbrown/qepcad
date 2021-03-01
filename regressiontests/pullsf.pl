#!/usr/bin/perl

my $state = 0;
while(<>) {
    if ($_ =~ /^An equivalent quantifier-free formula:/) {
	$state++;
    }
    elsif ($_ =~ /^\s*[\n]$/ && $state > 0) {
	$state = ($state + 1) % 3;
    }
    elsif ($state == 2) {
	print $_;
    }
}
