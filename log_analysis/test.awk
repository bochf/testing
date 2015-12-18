BEGIN {
	initColorPrint();
}

# main
{
	print color("Red")$1, color("Green")$2, color("Blue") $3, color("None") $0;
}
