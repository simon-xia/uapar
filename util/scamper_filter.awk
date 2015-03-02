#!/usr/bin/awk -f 
BEGIN {
	FS = "[, \t;]+"
}
{
	if ($1 !~ /^#/) {
		for(i = 4; i <= NF; i++) 
		{
			if ($i ~ /(^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$)|^q$/)
			printf $i"\t"
		}
		printf "\n"
	}
}