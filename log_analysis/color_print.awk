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
