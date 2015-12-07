#ifndef VERSION_H
#define VERSION_H
/*
  Version History:
   0.0.0    9/2/15 - Initial design kick off
   0.1.0    9/2/15 - Skeletal layout of source files
   0.2.0    9/3/15 - Stole config file processing and helper functions from
                     cpub code.
   0.3.0    9/4/15 - Moved tree management to forest.c.
                   - Wrote all the uname_X functions.
                   - Added OS specific support to the Makefile.
                   - Removed the complex, and unused, args from gettor funcs.
   0.4.0    9/8/15 - Started work on HTTPd server.
                   - Basic config file options are passed to options struct.
   0.5.0    9/9/15 - HTTPd server now uses threads.
                   - Cleanup of fprintf(stderr...) to error_msg().
                   - Additional config file parsing.
                   - grep $TUB *.c | wc -l ===> 40
   0.6.0   9/10/15 - Added some todos
                   - Strengthened ParseURLString() after reading (some of)
                     RFC2616.
                   - Added parsing of MESSAGE_FILE config file option.
                   - grep $TUB *.c | wc -l ===> 48
                   - HTTPd server now works correctly (in a skeletal sense)
                   - grep $TUB *.c | wc -l ===> 58
   0.7.0   9/14/15 - Dropped in (incomplete) doc headers for all .h functions.
                   - Added basic/skeletal bbenv tree
                   - Fixed bug in YAML output
   0.8.0   9/14/15 - Added basic .json formatter
   0.9.0   9/15/15 - Added process tree capability
                   - Prevented IOBuf overruns
                   - Wrote start of httpd error handling
   0.10.0  9/16/15 - Code cleanup, minor fixes.
                   - Fixed /proc directory read code in Linux/process.h/c
                   - grep $TUB *.c | wc -l ===> 57
                   - Code cleanup & documentation
                   - Wrote a (skeletal) access logger
                   - grep $TUB *.c | wc -l ===> 45
   0.11.0  9/17/15 - Moved trees into a forest.
   0.12.0  9/18/15 - Added ability to log access attempts to a file
                   - Code cleanup and $TUB removal
                   - grep $TUB *.c | wc -l ===> 29
                   - More code cleanup
                   - grep $TUB *.c | wc -l ===> 21
                   - Initial stab at AIX port
   0.13.0  9/22/15 - Added a skeletal hardware tree (Linux only)
                   - Ported to AIX
                   - grep $TUB *.c | wc -l ===> 14
   0.14.0  9/24/15 - Added requests serviced counter output item
                   - Added process trunk to bbenv tree
                   - Changed timestamp to ISO format
   0.15.0  9/25/15 - AIX port now properly reports uptime and proc count. 
                   - AIX port now properly reports SRADs (sockets)
   0.16.0  9/25/15 - Start of Solaris port
                   - It compiles, it runs, most data stubbed out
   0.17.0  10/1/15 - Continued work on Solaris port
                   - Only documentation / notes were added
   0.18.0  10/6/15 - Fixed macros in bbenv.c that were causing compiler
                     warnings on Solaris compile.
   0.19.0  10/9/15 - Modifications to this file
                   - Project paused for now
   0.20.0 10/14/15 - Added support for the qcconr pass-thru tree.
   0.20.1 10/15/15 - Fixed minor bug in qcconr pass-thru for END data
   0.21.0 10/16/15 - Re-jigging the means of passing app options into the
                     forest of trees.
                   - Added ability to query day=X in URL for qc_conr tree.
   0.22.0 10/19/15 - Fixed the AIX process listing.
                   - Fixed problem where httpd server was returning content
                     even when it had an error code.
                   - Started work on handling additional HTTP methods
   0.23.0 10/19/15 - Continued refactoring in httpd.c and continuing to make
                     the HTTPd server more standards compliant.
                   - The "sysstatd starting" message is now largely conditional.
                   - Minor hanging variable that broke -Wall -Werror on gcc/
                     Linux build.
   0.24.0 10/21/15 - Replaced the shamefully ugly StrStr() function with
                     a more reasonable implementation.
                   - Added Solaris process listing. (Doh! I thought I had
                     platform parity. It does now.)
   0.25.0 10/22/15 - Added new data items to process tree members.
                   - Added timestamp to access.log output.
                   - Added process start time to process output.
                   - Added check for /HOSTNAME.bypass to bbenv tree.

*/
#define VERSION_STRING "0.25.0"
/*
  ToDos:
   [_] Some documentation for every ("public") function.
   [ ] Threre is some rigidity in the QC module in terms of parsing the
       QC results file. Some of this needs to be cleaned up a bit and made
       more reliable.
   [ ] The tree dumps should have a mutex on them. This would prevent multiple
       simultaneous tree refreshes.
   [ ] Document the API (the external one).
   [ ] There is no "proper" app shutdown (such as a sighandler).
   [ ] Check to see if -o is used without -u. Warn, and unset. (or ignore)
   [ ] Daemonize code will need to grab o->cf_msgfile and save it to some
       static (to support.c) or global location so that error_msg() can
       properly use it.
   [_] error_msg() needs to use the config file MESSAGE_FILE option if it is
       set.
   [ ] The listening socket needs to be properly closed on Ctrl-C exit.
   [ ] Parse encoded URLs.
   [ ] URL parsing needs to return a list of strings. This can then be passed
       as an error to the output. (Ideally by just filling the output with
       the error strings, and not trying to dump a tree.)
   [ ] Should daemonize.c::MakeDaemon() drop a pid file?
   [ ] Can we fall back to defaults if the config file cannot be found?
   [ ] What are the "rules" / best practices for REST APIs? 

  Done:
   [X] access.log output requires a timestamp.
   [X] Put UTC timestamp into the access log output.
   [X] Re-implement support.c::StrStr(). Do it without a malloc().
   [X] Clean up some of the data types in urlparse.h::struct query.
   [X] Lots of hanging $TUBs/todos in httpd.c
   [X] What methods (GET, POST, HEAD, DELETE, PUT) will the daemon support?
   [X] Handle the error when the QC file is not found.
   [X] AIX port overflowed the output buffer and sent the data anyways (and
       then printed an error). (It dies at 10K - because of the larger number
       of processes on the AIX target system. (3094))
   [X] AIX: The process tree PIDs do not match the process names. This is
       an AIX thing with the API that you need to pull the PID from the 
       struct, not what you requested.
   [X] Fix (meaning: Actually write) the parsing of arguments to the web
       server.
   [X] Does AIX/process.h properly refrernce the header file for getprocs()?
   [X] Initial ports to AIX and SunOS
   [X] Create Linux, AIX, and SunOS source trees
   [X] AIX: AIX/aixbasic.c::GetUptimeSecs() has refrence to unused var.
   [X] AIX: aixbasic.c::GetCPUSocketCount() does not count SRADs.
   [X] The AIX version does not have process count or system uptime values.
   [X] AIX: aixbasic.c::GetProcCount() returns a bogous number.
   [X] AIX: sysinfo.h should be mined for potential data. (1. It is all in
       libperfstat; 2. The sysinfo symbol breaks the linker.)
   [X] Change the timestamp to proper ISO format.
   [X] The core tree should have the number of requests satisfied by the
       daemon as one of its variables.
   [X] JSON output prints an empty string if a function is registered that
       returns 0 items. If this is on a branch, then it should NOT try to
       print something (nothing with empty quotes).
   [X] Add number served counter to output.
   [X] Now that httpd.c is using IOBuf for input, it should call ResetIOBuf().
   [X] jsonfmtr should check output value. If numeric then no quotes.
   [X] Lay in code for daemonize::MakeDaemon().
   [X] Trees are not properly planted in httpd.c (main.c for now)
   [X] Linux/linbasic.c::InitBasic() initializes strings that are not used.
   [X] Linux should count the number of CPUs (all versions should output this).
   [X] Clean up data initialization and refresh.
   [X] AIX: systemcfg.h::_system_configuration->smt_threads should be used.
   [X] All the init_* functions in coredata.c do not check return values
       from things like mkstring().
   [X] Remove about 20 $TUBs
   [X] Carry over the "remaining" cpub mainhelp code (into support).
   [X] Fix all *.h make file dependencies
   [X] Comment blocks on all functions.
   [X] Develop a log-to option for both the config file options and output
       methods.
   [X] Write an access log module / capability.
   [X] Tighten up the code in main.c. There remains the kruft of many 
       test development scenarios.
   [X] datatree should have a root structure to pull it all together into
       one struct.
   [X] The logging function needs a pthread mutex on the write operation.
   [X] main.c does not handle dynamic trees (yet).
   [X] What should you do if access_log writing times out? Set it to NULL
       so that future attempts are skipped? (NO... Linux (RHEL 5.8) does 
       not support pthread_mutex_timedlock(). Just hang - that is what the
       clowns who implemented this OS desire.)
   [X] Finish the access log writer.
   [X] When running in a loop... Linux/process.c::fill_procitem() has a
       tendency to generate errors on the fopen() call. This needs to be
       handled in a NORMAL manner - as it is highly likely that pid dirs
       will disappear in the window that we are doing the readdir.
   [X] The httpd header needs to be application/json if json.
   [X] Write a dynamic tree implemetation for yaml (process).
   [X] Remove usage of is_a_number() in support.h/c.
   [X] The timestamp is a $TUB in proctree.c.
   [X] httpd.c::send_header() needs to handle application/json data type.
   [X] Remove the DEBUG parse_failure.... status line in httpd output.
   [X] Makefile clean/lean is not recursive.
   [X] Determine how to send error messages in json or yaml format (via
       httpd).
   [X] Insure that iobuf meets a minumum size of 1024 (so we can handle 
       returning error conditions).
   [X] Write a dynamic tree variant of process tree.
   [X] Define the "base" set of info items to be returned.
   [X] What are the ideal output formats (other than JSON)?
   [X] What are the rules for JSON output / formatting?
   [X] In the *fmtr.c functions there is a variable called "written". This
       value needs to be checked before appending it to the eof.
   [X] Write a sprintf() for obuf that handles the write accounting for you.
   [X] Write the json formatter.
   [X] YAML output does not print the branch labels.
   [X] Implement thread safe gettor functions.
   [X] datatree.h::struct dataitem needs to be updated to (potentially) support
       data on branch members. (Do away with the union?)
   [X] Create a (basic/skeletal) bbenv tree.
   [X] Yank htmlio.c/h. This code is in httpd.c/h.
   [X] The HTTPd server needs to parse URL input.
   [X] The HTTPd server needs to return real output.
   [X] Write: cfgfile::ParseAsInt(&local_int, cfoptions, char *label).
   [X] Write the (basic) HTTPd server.
   [X] fork() for data output, don't thread. This is because if the worker
       hangs, it will not hang the parent. This also means that the output
       module will need to check for dropped connections. --> NOPE. I am
       using threads (for a number of reasons).
   [X] Remove all new lines in error_msg() calls.
   [X] Lay in code/file for parsing the "query string/url"
   [X] Write a basic URL parser
   [X] Write about()
   [X] Write help()
   [X] Write framework to write to buffer (that can be dumped locally) before
       writing the HTTP support.

*/
#endif
