#!/usr/bin/perl

$curr = "";
while(<>)
{
    if ($_ eq "\n")
    {
	if ($curr ne "")
	{
	    push(@S,$curr);
	    $curr = "";
	}
    }
    else 
    {
	$curr = $curr.$_;
    }
}

if ($curr ne "")
{
    push(@S,$curr);
    $curr = "\n";
}

while($#S >= 0)
{
    print pop(@S)."\n";
}
