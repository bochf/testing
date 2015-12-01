
# number of seconds elapsed since 1970-1-1 0:0:0, no negative
function decodeTimestamp(timestamp) {
	print "decodeTimestamp(" timestamp ")";
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
	print "fmtTime=" fmtTime;
	return mktime(fmtTime);
}

function floor(x, i) {
	if (i == 0)
		return x;
	else
		return int(x/i) * i;
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
	print "begin=" begin;

	if (endTime == "")
		end = systime();
	else
		end = decodeTimestamp(endTime);
	print "end=" end;

	if (interval == "")
		interval = 0;
	print "interval=" interval;
}

/^[0-3][0-9]NOV[[:digit:]]{4}_[0-5][0-9]:[0-5][0-9]:[0-5][0-9]/ {
	print $0;
	current = decodeTimestamp($1);
	print "current=" current;

	if (begin <= current && current <= end) {
		totalEvents++;

		if (interval > 0)
			current = floor(current, interval);
		else
			current = begin;
		event = $4;
		id = sprintf("ID:%s,%010d", event, current);
		arrID[totalEvents] = id;
		arrTimestamp[id] = current;
		arrEvent[id] = event;
		arrCount[id] = arrCount[id] + 1;
		arrSubtotal[event] = arrSubtotal[event] + 1;
	}
}

END {
	asort(arrID);
	for (i in arrID) {
		id = arrID[i];
		printf("%-5d%-20s%s\n", arrCount[id], strftime("%Y%m%d_%H:%M:%S", arrTimestamp[id]), arrEvent[id]);
	}

	print "total events: ", totalEvents;
}
