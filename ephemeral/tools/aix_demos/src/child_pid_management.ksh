#!/bin/ksh93
#
# @(#) child_pid_management.ksh
#
# A ksh93 specific method for watching child PIDs and harvesting the
# exit codes or signal status, at the point of exit.
#
# Use cases are included.
#
# Author:  Eugene Schulman <ESchulman6@bloomberg.net>, R&D Systems Performance (G325)
#

# Child PID table
typeset -A PIDTAB
# For efficiency in the signal handler
typeset -i NUL
exec {NUL}>"/dev/null"


print "Master PID: $$"

# RV_DECODE: Takes a ksh93 return value; delivers a useful output string
# Inputs: 1- Return value, pass-by-value   2- Output string, pass-by-reference
# Output: Output string in arg2 reference
function rv_decode
{
	typeset -i rv="${1?rv_decode: Missing arg1, return code}"
	typeset -n out="${2?rv_decode: Missing arg2, output code}"

	if (( rv == 0 )); then
		out="OK"
	elif (( rv < 127 )); then
		out="FAIL, exit code $rv"
	elif (( rv >= 127 )); then
		# AIX/ksh93t specific:  Signal is 0x0100 | signum
		typeset -i signum=$(( rv & 0xff ))
		typeset -a signames
		set -A signames $( kill -l )
		typeset signame="${signames[$(( signum -1 ))]}"
		out="SIG$signame/$signum"
	fi
}

# SIGCHLD_HANDLER:  Target of SIGCHLD trap;
#  detects exits among monitored child PIDs;  reports
# Inputs: N/A arguments;   Uses PIDTAB assoc array
# Outputs: Reports change-of-status in monitored PIDs via print
function sigchld_handler
{
	typeset -i p
	typeset my_status
	# Yuck!  Because the ksh93 signal handler does not directly reveal the
	# child PID that threw the SIGCHLD, and because 'jobs' doesn't do this
	# in a useful way, there is no information about which child died.
	# ('jobs' will report the child as running until manually reaped with 'wait'.)
	# This means the code has to derive this by throwing each individual monitored PID
	# a test signal.
	for p in "${!PIDTAB[@]}"
	do
		if ! kill -0 "$p" 2>$NUL   # 'jobs' builtin doesn't cover this need
		then
			wait $p 2>$NUL
			rv=$?
			rv_decode "$rv" 'my_status'
			printf "%T  EXIT:  INSTANCE=${PIDTAB[$p]}  PID=$p  rv=$rv ($my_status)\n"
			unset PIDTAB[$p]
		fi
	done
}

# WATCH_KID:  Registers a PID and it's free-form Instance name to be monitored for SIGCHLD
# Inputs: 1- PID, pass-by-val,  2- Instance string, pass-by-val
# Outputs: None
function watch_kid
{
	typeset -i p_kid="${1?watch_kid: Missing arg1, PID}"
	typeset instance="${2?watch_kid: Missing arg2, Instance name}"
	PIDTAB[$p_kid]="$instance"
}

# Trap integration required
trap sigchld_handler SIGCHLD


## MAIN:  TEST CASES

/usr/bin/sleep 2 &
watch_kid "$!" "instance1"

(/usr/bin/sleep 2 && exit 1) &
watch_kid "$!" "instance2"

(/usr/bin/sleep 2 && exit 3) &
watch_kid "$!" "instance3"

/usr/bin/sleep 2 &
watch_kid "$!" "instance4"
kill -1 $!


/usr/bin/sleep 2 &
watch_kid "$!" "instance5"
kill -15 $!


/usr/bin/sleep 2 &
watch_kid "$!" "instance6"
kill -9 $!


wait
