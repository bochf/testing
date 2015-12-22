function max(a, b) {
	if (a > b)
		return a;
	else
		return b;
}

function min(a, b) {
	if (a > b)
		return b;
	else
		return a;
}

function formalize(value, min, def, max) {
	if (value == "")
		value = def;
	else if (min > value)
		value = min;
	else if (value > max)
		value = max;
	return value;
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

function isnumeric(x)
{
  return ( x == x+0 );
}

function colorIndex(name, predefined)
{
  if (isnumeric(name))
    return name;

  if (name in predefined)
    return predefined[name];

  return name;
}

# control output format
# color(foreground) default background color and attribute
# color(background, foreground) default attribute
# color(attribute, background, foreground) print in specified format
function color(v1, v2, v3)
{
  if (v3 == "" && v2 == "" && v1 == "")
    return;

  if (v3 == "" && v2 == "")
    return sprintf("%c[%dm", 27, colorIndex(v1, fgcolours));
  else if (v3 == "")
    return sprintf("%c[%d;%dm", 27, colorIndex(v1, bgcolours),
                   colorIndex(v2, fgcolours));
  else
    return sprintf("%c[%d;%d;%dm", 27, colorIndex(v1, attributes),
                   colorIndex(v2, bgcolours), colorIndex(v3, fgcolours));
}

function initColorPrint()
{
# hack to use attributes for just "None"
  fgcolours["None"] = 0;

  fgcolours["Black"] = 30;
  fgcolours["Red"] = 31;
  fgcolours["Green"] = 32;
  fgcolours["Yellow"] = 33;
  fgcolours["Blue"] = 34;
  fgcolours["Magenta"] = 35;
  fgcolours["Cyan"] = 36;
  fgcolours["White"] = 37;

  bgcolours["None"] = 0;
  bgcolours["Black"] = 40;
  bgcolours["Red"] = 41;
  bgcolours["Green"] = 42;
  bgcolours["Yellow"] = 43;
  bgcolours["Blue"] = 44;
  bgcolours["Magenta"] = 45;
  bgcolours["Cyan"] = 46;
  bgcolours["White"] = 47;

  attributes["None"] = 0;
  attributes["Bold"] = 1;
  attributes["Underscore"] = 4;
  attributes["Blink"] = 5;
  attributes["ReverseVideo"] = 7;
  attributes["Concealed"] = 8;
}
