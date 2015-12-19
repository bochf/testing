@include "format_util.awk"

BEGIN {
	initColorPrint();
}

# main
{
	bool = (0==1) || (1==1);
	if (bool)
		print "TURE";
	else
		print "FALSE";

	print color("Bold", "White", "Red")$1, color("None", "Green")$2, color("Blue") $3, color("None") $0;
}
