@include "format_util.awk"

BEGIN {
	initColorPrint();
}

function testBool() {
	bool = (0==1) || (1==1);
	if (bool)
		print "TURE";
	else
		print "FALSE";
}

function testColorPrint() {
	print color("Bold", "White", "Red")$1, color("None", "Green")$2, color("Blue") $3, color("None") $0;
}

function testArrayOrder() {
	split("", list, ";");

	print $0;
	for (i = 1; i < NF; i++) {
		list[$i] = length($i);
	}

	for (x in list) {
		print x, "length", list[x];
	}
}

function testPrintFomat() {
	print color("Bold", "White", "Red");
	printf("::%-*s::", 10, "abc");
	print color("None", "None", "None") "";
}

function testInteraction() {
	# wait for user input to proces next line
	print;
	line++;
	if (line >= 10) {
		line = 0;
		getline x < "/dev/tty";
	}
}

# main
{
	testInteraction();
}
