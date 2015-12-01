
# number of seconds elapsed since 1970-1-1 0:0:0, no negative
function decodeTimestamp(timestamp) {
  # timestamp format: DDMONYYYY_hh24:mi:ss.ms
  # DDMONYYYY is mandatory
  day = substr(timestamp, 1, 2);
  mon = month[substr(timestamp, 3, 3)];
  year = substr(timestamp, 6, 4);

  hour = 0;
  minute = 0;
  second = 0;

  sep = index(timestamp, "_");
  if (sep > 0) {
    timestamp = substr(timestamp, sep + 1);
    hour = substr(timestamp, 1, 2);

    sep = index(timestamp, ":");
    if (sep > 0) {
      timestamp = substr(timestamp, sep + 1);
      minute = substr(timestamp, 1, 2);

      sep = index(timestamp, ":");
      if (sep > 0) {
        timestamp = substr(timestamp, sep + 1);
        second = substr(timestamp, 1, 2);
      }
    }
  }

  fmtTime = sprintf("%4d %02d %02d %02d %02d %02d", year, mon, day, hour, minute, second);
  return mktime(fmtTime);
}

function floor(x, i) {
  if (i == 0)
    return x;
  else
    return int(x/i) * i;
}

# add event into lists
function addEvent(time, event) {
  id = sprintf("ID:%s,%010d", event, time);
  arrID[id] = id;
  arrTimestamp[id] = time;
  arrEvent[id] = event;
  arrCount[id] = arrCount[id] + 1;
  arrSubtotal[event] = arrSubtotal[event] + 1;
  totalEvents++;
}

function getResult(id) {
  printf("%-6d%-20s%-20d%s\n",
         arrCount[id],
         strftime("%Y%m%d_%H:%M:%S", arrTimestamp[id]),
         arrTimestamp[id],
         arrEvent[id]);
}

BEGIN {
  totalEvents = 0;

  month["JAN"] = 1;
  month["FEB"] = 2;
  month["MAR"] = 3;
  month["APR"] = 4;
  month["MAY"] = 5;
  month["JUN"] = 6;
  month["JUL"] = 7;
  month["AUG"] = 8;
  month["SEP"] = 9;
  month["OCT"] = 10;
  month["NOV"] = 11;
  month["DEC"] = 12;

  if (beginTime == "")
    begin = 0;
  else
    begin = decodeTimestamp(beginTime);
  # print "begin=" begin;

  if (endTime == "")
    end = systime();
  else
    end = decodeTimestamp(endTime);
  # print "end=" end;

  if (interval == "")
    interval = 0;
  # print "interval=" interval;
}

/^[0-3][0-9]NOV[[:digit:]]{4}_[0-5][0-9]:[0-5][0-9]:[0-5][0-9]/ {
  current = decodeTimestamp($1);

  if (begin <= current && current <= end) {
    n = split($4, arr, "/");
    if (interval > 0)
      addEvent(floor(current, interval), arr[n]);
    else
      addEvent(begin, arr[n]);
  }
}

END {
  n = asort(arrID);
  # printf("%-6s%-20s%s\n", "Count", "Timestamp", "Event");
  for (i = 1; i <= n; i++) {
    getResult(arrID[i]);
  }

  # print "total events: ", totalEvents;
}
