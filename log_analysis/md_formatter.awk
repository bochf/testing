BEGIN {
  FS = ", ";
  if (max_width == "")
  {
    max_width = 0;
  }
  else if (max_width < 20)
  {
    max_width = 20;
  }
  headerPrinted = 0;
}

function urlDecode(url)
{
  for (c = 0x20; c < 0x40; ++c)
  {
    repl = sprintf("%c", c);
    if ((repl == "&") || (repl == "\\"))
    {
      repl = "\\" repl;
    }
    url = gensub(sprintf("%%%02X", c), repl, "g", url);
    url = gensub(sprintf("%%%02x", c), repl, "g", url);
  }
  return url;
}

function shrink(str, width)
{
  if (width == 0)
    return str;

  if (length(str) > width)
  {
    newstr = substr(str, 1, width-3) "...";
    return newstr;
  }
  else
  {
    return str;
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
