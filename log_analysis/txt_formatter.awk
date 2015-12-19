BEGIN {
  FS = ", ";

	formalize(max_width, 10, 200, 200);
	formalize(page_size, 5, 50, 1000);

  headerPrinted = 0;
}

# print one page of data
# rows: number of lines
# columns: number of fields per line
# data: decoded text, seperated by comma
# hasHeader: first line of data is header or not
# columnWidth: width of each column
function printPage(rows, columns, data, hasHeader, columnWidth) {
	if (rows < 1 || columns < 1)
		return;

	currentLine = 1;
	if (hasHeader)
		printLine(data[1], columnWidth, "Blue");

	for (currentLine = 2; currentLine <= rows; currentLine++) {
		printLine(data[currentLine], columnWidth, "Yellow");
	}
	}
function printHeader(headers, size)
{
  if (headerPrinted > 0)
    return;

  if (size <= 1)
    return;

  printf("|");
  for (i = 1; i <= size; i++)
  {
    printf("**%s**|", shrink(headers[i], max_width));
  }
  print "";

# print split line
  printf("|");
  for (i = 1; i <= size; i++)
  {
    printf(":--|");
  }
  print "";

  headerPrinted = 1;
}

function printData(fields, size)
{
  if (size <= 1)
    return;

  printf("|");
  for (i = 1; i <= size; i++)
  {
    printf("%s|", shrink(urlDecode(fields[i]), max_width));
  }
  print "";
}

function main()
{
  for (i = 1; i <= NF; i++)
  {
    header = "N/A";
    data = "N/A";
    pos = index($i, "=");
    if (pos > 1)
    {
      header = substr($i, 1, pos-1);
      data = substr($i, pos+1);
    }
    headers[i] = header;
		fields[i] = data;
  }

  printHeader(headers, NF);
  printData(fields, NF);
}

# main process
{
  main();
}
