BEGIN {
  FS = ", ";

	formalize(max_width, 10, 200, 200);
	formalize(page_size, 5, 50, 1000);
	formalize(header_style, 1, 1, 2); # 1 header per page (default)
	                                  # 2 header on first page

  lineHeader = "";
	currentPageLines = 0;
	pageNo = 0;
}

# print one page of data
# rows: number of lines
# columns: number of fields per line
# data: decoded text, seperated by comma
# hasHeader: first line of data is header or not
# columnWidth: width of each column
function printPage(rows, data, hasHeader, columnWidth,   currentLine) {
	if (rows < 1)
		return;

	currentLine = 1;
	if (hasHeader) {
		print color("Bold", "None", "Red");
		printLine(data[1], columnWidth);
	}

	for (currentLine = 2; currentLine <= rows; currentLine++) {
		print color("Blue");
		printLine(data[currentLine], columnWidth);
	}
}

function printLine(data, columnWidth,   columns, col, fields) {
	columns = split(data, fields, ", ");
	printf("|");
	for (col = 1; col <= columns; col++) {
		printf(" ");  # left margin
		printf("%-*s", shrink(fields[col], columnWidth[col]));
		printf(" |");
	}
	print color("None", "None", "None");
}

function fillPage(body, header) {
	if (header == "")
		return;
}

function process(line)
{
	if (currentPageLines == 0) {
		for (i = 1; i <= NF; i++) {
			columnWidth[i] = 1;
		}
	}

	split(line, fileds, ",");
	for (i = 1; i <= NF; i++) {
		width = length(fields[i]);
		formalize(width, columnWidth[i], width, max_width);
		columnWidth[i] = width;
	}

	currentPageLines++;
	buffer[currentPageLines] = line;

	if (currentPageLines == page_size) {
		pageNo++;

		hasHeader = (pageNo == 1) || (header_style == 1);
		printPage(currentPageLines, buffer, hasHeader, columnWidth);

		currentPageLines = 0;
		split("", buffer, ":");  # destroy the current page buffer
	}
}

function init() {
	if (g_initialized)
		return;

	for (i = 1; i <= NF; i++) {
		g_columnWidth[i] = 1;

		pos = index($i, "=");
		if (pos > 1) {
			key = substr($i, 1, pos-1);
	}

	g_initialized = 1;
}

function main() {
	gsub(/^\(/, "", $0);
	gsub(/\)$/, "", $0);

	lineHeader = "";
	lineData = "";

	for (int i = 1; i <= NF; i++) {
		pos = index($i, "=");
		if (pos > 1) {
			key = substr($i, 1, pos-1);
			value = substr($i, pos+1);
		}
		else {
			key = "N/A";
			value = "N/A";
		}
		lineHeader = lineHeader "," key;
		lineData = lineData "," value;
	}

	process(lineData);
}

# main process
{
	line = urlDecode($0);

	if (lineHeader == "")
		lineHeader = line;

  process(line);
}
