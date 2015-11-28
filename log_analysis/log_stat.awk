BEGIN {
	events = 0;
}

/NOV2015/ {
	events++;

	split($1, lstDateTime, "_");
	strDate = lstDateTime[1];
	strTime = lstDateTime[2];

	split(strTime, lstTime, ":");
	hh = lstTime[1];
	mi = lstTime[2];
	ss = lstTime[3];

	print $1;
	print strDate, strTime;
	print hh/2, mi/2, ss/2;
}

END {
	print "total events: ", events;
}
