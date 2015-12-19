@include "format_util.awk"

BEGIN {
  FS = ", ";

	initColorPrint();

	max_width = formalize(max_width, 10, 200, 200);
	page_size = formalize(page_size, 5, 50, 1000);
	header_style = formalize(header_style, 1, 1, 2); # 1 header per page (default)
	                                                 # 2 header on first page
	pause_per_page = formalize(pause_per_page, 0, 1, 1); # 0 no wait
	                                                     # 1, pause after print one page
}

# print one page of data
# rows: number of lines
# columns: number of fields per line
# data: decoded text, seperated by comma
# columnWidth: width of each column
# header_style: 1 print header on each page, 2 print header only on first page
function printPage(rows, data, columnWidth, header_style,   currentLine) {
	if (rows < 1)
		return;

	if (g_pageNo == 1 || header_style == 1) {
		# adjust size of each column, width[n] = min(max_width, max(headerWidth[n], dataWidth[n]))
		for (i = 1; i <= NF; i++)
			columnWidth[i] = min(max_width, max(g_headerWidth[i], columnWidth[i]));

		printf color("Bold", "White", "Red") "";
		printLine(g_lineHeader, columnWidth);
	}

	for (currentLine = 1; currentLine <= rows; currentLine++) {
		printf color("None", "None", "Blue") "";
		printLine(data[currentLine], columnWidth);
	}
}

function printLine(data, columnWidth,   columns, col, fields) {
	columns = split(data, fields, ",");
	printf("|");
	for (col = 1; col <= columns; col++) {
		printf(" ");  # left margin
		printf("%-*s", columnWidth[col], shrink(fields[col], columnWidth[col]));
		printf(" |"); # right margin and boarder
	}
	print color("None", "None", "None") "";
}

function fillPage(body, header) {
	if (header == "")
		return;
}

# reset global variables of a page
function newPage() {
	g_pageNo++;
	for (i = 1; i <= NF; i++)
		g_columnWidth[i] = 1;
	g_currentPageLines = 0;
	split("", g_buffer, ":");  # destroy the current page g_buffer
}

# add rows in page buffer
function process(line)
{
	n = split(line, fields, ",");
	for (i = 1; i <= n; i++) {
		g_columnWidth[i] = max(g_columnWidth[i], length(fields[i]));
	}

	g_currentPageLines++;
	g_buffer[g_currentPageLines] = line;

	if (g_currentPageLines == page_size) {
		printPage(g_currentPageLines, g_buffer, g_columnWidth, header_style);
		newPage();

		if (pause_per_page == 1)
			getline pause < "/dev/tty";
	}
}

function init() {
	if (g_initialized)
		return;

	for (i = 1; i <= NF; i++) {
		pos = index($i, "=");
		if (pos > 1) 
			key = substr($i, 1, pos-1);
		else
			key = "N/A";

		if (g_lineHeader == "")
			g_lineHeader = key;
		else
			g_lineHeader = g_lineHeader "," key;

		g_headerWidth[i] = min(length(key), max_width);
	}

	g_pageNo = 0;
	newPage();

	g_initialized = 1;
}

function main() {
	lineValue = "";
	for (i = 1; i <= NF; i++) {
		pos = index($i, "=");
		if (pos > 1) {
			value = substr($i, pos+1);
		}
		else {
			value = "N/A";
		}

		if (lineValue == "")
			lineValue = value;
		else
			lineValue = lineValue "," value;
	}

	process(urlDecode(lineValue));
}

# main process
{
	gsub(/^\(/, "", $0);
	gsub(/\)$/, "", $0);

	init();

	main();
}

END {
	if (g_currentPageLines > 0)
		printPage(g_currentPageLines, g_buffer, g_columnWidth, header_style);
	print color("None", "None", "None") "";
}
