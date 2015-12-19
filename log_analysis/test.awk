@include "format_util.awk"

BEGIN {
	initColorPrint();
}

# main
{
	print color("Bold", "White", "Red")$1, color("None", "Green")$2, color("Blue") $3, color("None") $0;
}
