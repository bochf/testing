#ifndef VERSION_H
#define VERSION_H

/*
  Version History:
   0.0.0   1/13/14 - Prototype / PoC
   0.0.1   1/14/14 - Additional work
   0.0.2   1/14/14 - AIX specific provider added
   0.0.3   1/15/14 - Added Linux provider
                   - Added -c option for passing config file on command line
   0.0.4   1/16/14 - Moved existing output functions to a new source file
                     and extended the formatting options.
   0.0.5   1/21/14 - Started to hack in Solaris ktrace data.
                   - See important notes in SunOS/ktraceprov.c
                   - This (Solaris) code is not set to (attempt to) compile
                     at this time. It will not compile - I checked it in
                     anyways.
   0.0.6   1/22/14 - Solaris port underway - revisions to design as a result.
                     See commentary in providers.c::EnableProvider()
                   - Changed all "ktrace" references to "kstat". Duh.
                   - Yanked references to the bogus provider.
   0.0.7   1/24/14 - Re-design/re-work of the framework based on the large
                     dynamic provider registration issues found with kstat.
   0.0.8   1/27/14 - Continued conversion to the "new framework".
                   - Added the WritingAProvider.txt file.
   0.0.9   1/28/14 - Documented functions in providers.h
                   - Added new data items to AIX providers (using the new
                     framework)
                   - Cleaned up some of the coding using #define macros
                   - stub elimination
                   - The provider->enablepitem function pointer is NOT set
                     in the library code. The make dependencies do not exist.
                     This release is simply not functional for data output.
   0.0.10  1/29/14 - Documentation, framework work
   0.0.11  1/30/14 - Code cleanup
                   - Began move of vminfoprov to the new method
   0.0.12  1/31/14 - Moved both AIX providers to newer methods
   0.0.13   2/2/14 - Work on config file parsing
                   - Compiles, but exits after parsing config file. This
                     version just works through the config file parsing.
   0.0.14   2/3/14 - Additional work on config file parsing
   0.0.15  2/10/14 - Dropping in the thread code
   0.0.16  2/11/14 - Author set
                   - Documented the locking/sychronization methods
                   - Added additional lock/sychronization to prevent readers
                     and writers falling out of sync.
   0.0.17  2/12/14 - Re-design of the writer infrastructure.
   0.0.18  2/13/14 - Continued work on initial implementations of new design
   0.0.19  2/14/14 - Code cleanup additional work
                   - First writer implementation (and accompanying framework)
                     should be done - knock on wood
                   - More documentation
                   - Punched out some of the stubs remaining in the code
                     (Still many remain)
                   - Code is now setup for daemon work (this can be stolen
                     from udp_watch)
   0.0.20  2/14/14 - Pushed to repo, updated the ticket, snapped a version
                   - Finally added full support for timespec and timeval
                   - Config file now parses run_method = (daemon | foreground)
                     (The daemon code is not fully written yet.)
   0.0.21  2/17/14 - Additional todos to this file.
                   - Added support for various list options (-l,-d,-p)
                   - Removed last references to bit-variant PDT_* datatypes
                   - wmfile.c now is functional/compiles but is not linked
                     in.
   0.0.22  2/18/14 - Resolution of the alarm code in configfile.h/c
                   - Moved writers into a library / build tree
                   - Started (re)working on Linux port
                   - MANY minor gcc -Wall -Werror problems found - fixed
                   - Work on meminfo (linux) provider
   0.0.23  2/19/14 - Started work on man pages
                   - Work on Linux port
   0.0.24  2/20/14 - Linux port working
   0.0.25  2/22/14 - Knocking out some of the todos...
                   - Removed the -c usage for config files.
                   - Moving header to pitem (started this support). Cleaning
                     up some of the other data in the provider.h structs.
   0.0.26  2/23/14 - Cleaning up stubs - stregenthening the project
                   - Fixed the parsing multipliers on run_for config file
                     option.
   0.0.27  2/24/14 - Added support (main.c & threadsupt.c/h) for the averaging
                     of statistical data.
                   - Re-added (minor) support for -v
                   - Added parsing support for psavg in Q4.
                   - Added psavg support in opcputprov.c
                   - Touched (minor) roughly half the files in the code base
   0.0.28  2/25/14 - Added psavg number reporting on verbose mode
                   - Basic cleanup of vminfo provider.
                   - Added AIX process provider
   0.0.29  2/26/14 - Added config file arg ingestion
                   - Added basic config file arg substitution
   0.0.30   3/3/14 - Discovered and fixed naming issue in GetPItemByName*()
   0.0.31   3/5/14 - Added the Common library and first (file.stat) provider 
                   - Minor cleanups in providers.h
                   - Minor cleanups (dead dependency removal) in main makefile
   0.0.32   3/6/14 - Finish, cleanup, & testing of new code 
   0.0.33  3/10/14 - Work on the alarm writer module code
                   - Removed some of the noize from the (gmake) build process
   0.0.34  3/11/14 - Finishing out changes started in previous version
   0.0.35  3/12/14 - Ripped out all references to ki list
                   - Ripped out all getpitem() references
                   - SIGNIFICANT CHANGES
   0.0.36  3/12/14 - Knocking out small issues.
                   - Daemon mode now supports writing (via error_msg()) to
                     syslog.
   0.0.37  3/17/14 - Factoring design and coding
                   - Cleanup and additions to AIX vmgetinfo() provider.
                   - Fixed bug in AIX process provider (missing data types)
                   - Converted process provider to malloc() method.
                   - I did NOT fix the PID in the header bug here. I dropped
                     a stub (and discussion points) into the code where this
                     should/would go.
                   - Linux port (and associated -Wall -Werror cleanup)
                   - Linux provider now uses malloc() data_ptr method.
                   - There is *significant* code change here. There were about
                     4 git commits on this version.
   0.0.38  3/18/14 - Return to the Solaris port. Wow; the framework has
                     shifted since this was last built.
                   - The Solaris library now compiles - but is largely stubbed
                     out.
                   - Solaris port now compiles but the kstat_list() function
                     seems to run-on without a terminating condition.
                   - Fixed issue where the file writer module was walking
                     the wrong list.
   0.0.39  3/24/14 - Added core::echoarg
   0.0.40  3/25/14 - Minor fixes
   0.1.0   3/26/14 - Changed over to the new timer code
   0.2.0   3/27/14 - Pulled dead members from pitem struct, including the
                     long-standing reference to "collect".
                   - Made significant changes to parsing of fact() to factor=
                   - Removed references to header in qpart struct. All code
                     was #ifdefed out.
   0.3.0   3/28/14 - kstat() changes - it works with massive stubbed sections
   0.4.0   3/29/14 - Added a CalcData() function to simplify much of the
                     ugliness required to calculate values in the provider
                     implementations. Converted AIX/lpcputprov.c to new method.
   0.5.0   3/31/14 - Converted remaining writer modules to the sign methodology
                   - Changed MAX_QPART_LEN to 64 based on some of the sizes
                     seen in the kstat_list() output.
   0.6.0    4/2/14 - Documented new API
                   - Modifications to overall documentation
   0.7.0    4/7/14 - Laid in the core code for file.ops provider
   0.8.0   4/16/14 - Moving again.
                   - New code compiles, but very incomplete.
   0.9.0   4/29/14 - Cleanup and finish of the file.perfops provider
   0.10.0  4/30/14 - Additional minor cleanup and docuementation for the
                     file.perfops provider
   0.11.0   5/2/14 - Added foundations for mount.df
   0.12.0   5/5/14 - Work on mount.df provider
   0.13.0   5/6/14 - providers.c now supports a return of more than one pitem
                     when calling enablepitem(). This is fundamental in
                     supporting wildcards in user input.
   0.14.0  5/19/14 - Added proc.pid.stat provider for Linux
                   - Compiles, parts still missing
   0.15.0  5/20/14 - Work on proc.pid.stat
   0.15.1  5/21/14 - See above.
   0.16.0   6/5/14 - Continued work on Linux providers
   0.17.0   6/9/14 - Continued work on Linux providers
   0.18.0  6/10/14 - Continued work on Linux providers
   0.19.0  6/13/14 - Cleaning up todos
   0.20.0  6/16/14 - Code cleanup
   0.21.0  6/17/14 - Continued refinement and cleanup
   0.22.0  6/19/14 - Fixed an issue in the Solaris port (Not sure how it 
                     lasted this long).
   0.23.0  6/23/14 - ToDo cleanup
                   - Inserted a PROPOSED_CHANGE in threadsupt.h/c. This is for
                     handling conditions where a provider fails to collect
                     data in an iteration. This has not been tested at this
                     time.
   0.24.0   9/9/14 - Integrated BCD code as a writer.
   0.25.0  9/16/14 - Moved some of BCD functions into common code. (The goal
                     is to provide similar (exactly the same) functionality
                     in other writers.)
   0.26.0  9/17/14 - Fixed issue with broken file format in BCD
   0.27.0  9/22/14 - Added basis for AIX perfstat.diskadapter
                   - Cleaned up the AIX code that perfstat.diskadapter was
                     copied from (perfstat.cputotal)
   0.28.0  9/23/14 - perfstat.diskadapter looks complete.
   0.29.0  4/16/15 - Fixed some options in danger mode.
                   - Added background keyword to run_method option.
*/
#define VERSION_STRING "0.29.0"
/*
  ToDo:
   [ ] Determine why the file writer module dies on atomic_writes.
   [ ] There is a compiler warning in mainhelp.c on Solaris.
   [_] Insert ebcd into source tree/package
   [ ] There is a check for munge options in perfstat CPU code
   [ ] Move file roll and parsing functions into common srouce for writers.
       Specifically, this is the code introduced from BCD.
   [ ] A PROPOSED_CHANGE is hanging out in threadsupt.h/c
   [_] Finish the kstat() implementation
   [ ] alarm_at is "not compatible" with fact() - even though it is. It is
       not compatible *conceptually*, although the alarm module will never
       see factored data. It would be appropriate for this to be an error
       condition.
   [ ] Define an error_to value for the config file. This can be used to
       push errors to a specific location when running as daemon.
   [ ] StrStr() is too weak of a pattern match in InitAlarmWM() and similar
       functions. It should be a StrCmp().
   [ ] mainhelp.h requires some documentation for the actlog functions.
   [_] The AIX port (vmgetinfo()) data is VERY incomplete. Add *some* more.
       I added more - but lots remain. The code to add them is much easier
       now.
   [D] The sample code that fork()ed off as a daemon hung with no writes.
       The code did not respond to a signal - the expectation being that
       it was either hung in a lock, was not properly handling signals, or
       both. The code that broke was samples/membg.cpub. The symptom was that
       it did not respond to signals, and could not be killed unless it was
       kill -9'd. It truncated the file, but would not write new data. I have
       since ran the membg.cpub script multiple times with no issues. I am
       keeping this issue open for now - it could be some sort of edge case,
       or race condition.
   [D] Deleting the file in file.perfops causes the OPEN time to go negative.
       THE PROBLEM: Deleting the file on a NJ server causes the NY server to
       think that it still exists (via the file check), but it is not really
       there. So the stat() call (or whatever in file check) succeeds, but the
       open() fails because the file is not really there. The fix is presumably
       a better file check. This is not a problem when the file is deleted on
       the same server that is running cpub. Marking deferred for now.

  Done:
   [X] Add "background" keyword to run_method option parsing.
   [X] Need to make danger mode more tenable. Should be able to override in
       the config file(?) and should set danger mode based on providers, not
       the number of quads.
   [X] Create an AIX provider for perfstat_diskadapter_t
   [X] Write a StrCmp() function.
   [X] There is a stub_OLD_METHOD sitting in the perfstat CPU code
   [X] Test BCD writer
   [X] ebcd dumps from read_datatype() - Because the skip bits are not written
       when data is skipped.
   [X] BCD writer seems to roll way too quickly (after each write)
   [X] processprov.c uses "manual labor" to calculate stats in process_update()
       It should use the new API provided by providers.h (CalcData())
   [X] Test non-standard sizes in file.perfops 512K, 2056B, 3M, etc.
   [X] file.perfops does not properly handle exceptions (like unable to 
       perform an operation on the file).
   [X] More testing on file.perfops
   [W] Linux linproviders.c::PullNewBuff() needs some exception handling,
       specifically the ability to recognize when the allocated buffer
       has been overflown. Withdrawn: Function was updated and then depricated.
   [X] proc.pid.stat items are not being registered properly. Doh! The .cpub
       file was bad. The errors were simply skipped. This needs to be fixed.
   [X] Validate that diff and psavg works in proc.pid.stat.
   [W] file.perfops flags diff and psavg as errors. It should flag factor
       as an error as well. -- No, it should not. The data is int64 and can
       be factored.
   [X] meminfoprov.c uses old (non CalcData()) method for finding data.
   [X] meminfoprov.c "update function" does not check return values.
   [X] providers.h::qparts->cfpn is not properly documented.
   [X] APIs and data structs are not documneted in linproviders.h
   [X] There is a stub in meminfoprov.c
   [X] Go back to meminfo provider and plug in the new fetch_a_line() function
       that is in the library. Yank fetch_a_line() in the individual file.
   [X] Document factor=dtoh writer option
   [X] proc.stat.almn.cpub sample file alarms on the first iteration.
   [X] The flags item in /proc/PID/stat will print in decimal - a worthless
       representation. An option should exist to render in hex.
   [X] The header info in proc.stat does not show the CPU number.
   [D] Change the name of the project.
   [X] Samples/linpidstat.cpub hangs in a loop. It appears to be a problem 
       with walking the PID list. Also, the comm string option is broken,
       as no memory was ever malloc()ated.
   [X] A "standard" must be specified for when a provider argument is
       *required*. (This has to do with the list mode. What should be in the
       listing for the arg field?) The process, file.stat, and file.ops should
       all agree on what the list() output should be to denote that an argument
       is required. The current thinking is to put a "*" in the quad that is
       empty/not-specified but required.
   [X] providers::EnableDataPoint() should:
       1. Check for wildcard "*" in the quad args list
       2. Look for more than one item returned in the event of "*" was passed.
       Actually.... the enablepitem() code should be able to just get back
       a ready made list. Provided that list is in order, the existing code
       should work. The only thing needed to be changed is the alarm code
       just before the linked list code.
   [X] There is a mountain of stubs in the mount.df provider.
   [X] Remove dead code from file.perfops.
   [X] Account for all local function prototypes in file.perfops provider.
   [X] Update the man page (part) for file.perfops provider
   [W] Consider the following feature:
       core::icount exit_at = 1
       The framework would exit when this value is 1
       I considered it not such a worthwhile idea. Use run_for instead.
   [X] Stubs remain from the fact() to factor= change in providers.c
   [X] Stubs remain from fact()/factor= for the qpart->header removal in
       providers.c
   [X] Validate that the run_for option is properly supported. I can't recall
       if this was ever implemented or not. It is not - fix it.
   [X] Convert the writers to the new sign_flag usage. stdout done. factor
       data (wmcommon) done. Remainder done.
   [X] Document providers::CalcData().
   [X] Concept worth investigating: When diffing on a unsigned value - see
       if the results will go negative, and if they do, then make it positive
       and then set a sign flag. This would require an additional value in the
       pitem. Some of the calculation code could be cleaned up with common
       functions over this (that should happen anyways).
   [X] Convert the core provider over to CalcData(). Wrong answer stupid...
       think about it. The core provider data types do not get diffed or
       calculated - they are simply assigned. Duh.
   [X] providers::DumpDataType() should not need to be called conditionally.
       In fact, the whole private part of the Dump* API can be re-written to
       make output clearer for the providers. Now remove the old method.
   [W] The process provider needs to set its headers. See notes in the code
       about this. ---> At this time, no provider requires or actually does
       this. But the support is now there for it to happen.
   [X] The providers have code for parsing the iargs. (See processprov.c)
       This *should* be set elsewhere (in NewPItem()) and only checked
       to see if it is incompatible here. As of now, it is set twice.
   [X] The following quad does not work as expected (when the file does not
       exist): [file.stat:$ARG1:size:diff alarm_at != 0]
   [X] Consider the following (global) feature:
       ignore_first_write=true
       When true, the first iteration data is ignored / not written.
   [X] Remove references to pitem->collect from documentation.
   [X] There are references to "drift problems" in the documentation. This
       has been changed. Fix, remove, update.
   [X] Is pitem->collect used anywhere? If not, then remove it.
   [X] Remove references to PRIVATE PROPOSED in the documentation.
   [X] Is the _POSIX_C_SOURCE declaration in coreprov.c: 1) necessary? Could
       we just use the regular localtime() function? and 2) the most
       appropriate (and not just sufficient) value?
   [X] The file provider is walking the wrong list for header output. The
       writer struct should NOT keep a reference to the prov_name struct.
   [X] Bring the Solaris port back into compliance.
   [X] All providers need to be converted to "malloc() data_ptr" to support
       multiple instances of a quad. (Done: vminfo, process, perfstat.cputotal)
   [W] proot->pitem_count was incremented on insert, it should be accounted
       for somewhere now that the value is not set on pitem insert. (withdrawn)
   [X] The parsing of header info (providers.c) fails for the spaced version
       that is in proc_watch.cpub [header = "str"] instead of [header=str].
   [X] Two of the process providers have unknown data types. (pi_pri and
       pi_adspace_ldr)
   [X] There is no option to factor data. (A writer option)
   [X] vminfoprov uses a lame pattern matching in vminfo_enablepitem(). The
       lpcputprov implementation uses a much better pattern matching method.
       the vminfo version should be changed to the better libperfstat match.
   [X] The WritingAProvider.html document needs to have pointers to reference
       implementations.
   [X] The header option is not properly assigned to the header string when
       using the alarm writer module. The expectation here is that it is
       not set properly for ANY of the writer outputs. This likely has to
       do with the other writer modules still using the depricated means
       of writing header information.
   [X] There is a stub on the header portion of quad parsing in the
       StrToQuadParts[Create|Fill]() function (in providers.c).
   [X] How do you handle messages when in daemon mode? There is currently
       no method for handling this. Should the messages go to a "cpub" tag
       in act.log, syslog, a specialized log file? Should the act.log tag
       option be global to the config file?
   [X] Need to write an alarm writer module.
   [X] Daemon mode. This probably should be done from the config file. The
       thinking is that output methods that are sensitive to running without
       stdout/stderr can check for it before backgrounding. The other issue
       is that there are fprintf(stderr,...) lines everywhere. These need
       to be converted into "situationally aware" error loggers. Set a log-
       to global variable. Write an error writer to utilize it. A mask can
       be used to write to act.log, stdout, or syslog.
   [X] Some stubs remain in wmalarm.c
   [X] The core provider does not support more than one registration of a 
       single item. This is a problem for any provider that uses a common
       struct for the result data!
   [X] The core provider does not check data types!
   [X] Resolve the makefile dependency issues in cpub.1
   [X] Un-stub actlog functions in mainhelp.c
   [X] Test the daemon mode.
   [X] Alarm writer module needs to valdate that the alarm values are all
       numeric and alarms are set on ALL quads. (Why include data that will
       not alarm - when in the alarm "mode".)
   [X] Insure that the InitAlarmWM() code walks through the opi list to 
       determine if all items have alarms set.
   [X] Where is the test for attempts to compare (alarm_at) complex types and
       strings? This must be caught.
   [X] The fourth quad items are not parsed. At this time the only supported
       item is diff. The factor arg may play, but it has some issues on
       assumptions about what the input data is.
   [X] vmgetinfo provider returns a NULL in a int prototyped function.
       Specifically for vminfo_enablepitem(). There are probably more of these
       that the compiler is not complaining about.
   [X] The AIX/vminfo provider returns a NULL on an int. (I copied the code
       and then noticed the error.)
   [X] Kripes! Now the simple example cpub file is not outputting data.
       What changed? Makefile dependendy problem. make clean ; make
       fixed it. There is a 1000000% chance this had to do with providers.h.
   [X] Make the following changes to ALL reference provider implementations:
       - Do an assert(sizeof(type) == sizeof(expected)) for ALL types used
         in the povider.
       - Do NOT initialize memory until it is required.
       - No printing to stderr when things go wrong on update.
   [X] Cleanup in isle meminfoprov.c! Some of the code here has been kicked
       around for a while - it needs some documentation and cleanup.
   [X] meminfoprov.c::meminfo_enablepitem() needs the string compares to
       be moved to another function. The howto and why are documented in
       the code.
   [X] Linux meminfo minfo_data struct uses unsigned 32 bit values. This
       should likely be signed for diff purposes.
   [X] The sample providers do not average per-second. The concept of 
       averaging data is not even properly supported (although enabling it
       should not be a major issue). (This would be a provider option)
   [X] stubs in vminfoprov.c
   [X] Basic stats for providers registered, possibly data items registered,
       and output items should be sent to stderr prior to running. This can
       be overridden with a -q(uiet) option.
   [X] lpcputprov.c initializes data on int, not the first insert. This
       is fairly common in the reference implementations and should be
       killed - it is wastefull.
   [X] Config file parser reads hard values (ints) but does not read the
       time format specifiers/multipliers (sSmMhHdDwW...)
   [X] Tracking a pointer to collect seems kind of pointless. This might be
       depricated - or at least moved to PRIVATE.
   [X] Some stubs live on in wmfile.c. The parse_option stub exists in the
       stdout writer - but is not documented there.
   [X] A writer module "library" is required. This is partially for build
       tree cleanliness, but also for common helper functions specific to
       the writers such as checking for config writer args, and factoring
       data before writing.
   [X] Remove the reference to readygo in the worker thread data. That 
       approach did not work - and I may have left a dangling reference.
   [X] man pages for the command and config file should be written.
   [X] Remove the -c requirement for passing a config file. cpub should be
       callable as "magic" from the interpreter shebang of the config file.
       This can be done, provided the -c option is used in the config file
       interpreter magic string, but it is really unnessary and redundant
       for an option that is required (when not in one of the list modes).
   [X] Write a second writer module implementation. Ideally a file writer.
   [X] Writers should probably be moved to a library / sub-directory.
   [X] The thread code is hanging on Linux (after it worked fine on AIX).
       Fixing this is of-course a top priority. This is a race condition
       caused by startup that works one way on Linux and another on AIX.
       This was caused by a startup race condition that was found only on
       Linux.
   [X] There is no provision for a quad argument for alarm. This is still
       a major design issue.
   [X] wmfile.h exists, now make a wmfile.c and put it in the build.
   [X] List only the provider names
   [X] List only the data items for a given provider
   [X] Add option parsing to the config file parser for daemon/foreground.
   [X] The output (first implementation in stdout writer) for the core
       provider does not handle timespec and timeval data.
   [X] The core provider needs a shortened (HHMMSS) string. The full string
       (with the year date) messes with fixed width output because it is so
       long. Plus, just the time is more appropriate for "interactive" type
       output.
   [X] Write a close() method for the writer module. This will be responsible
       for whatever actions are required to wrap up writing data. This may
       be a no-op, it may explicitly close files, or it may do more.
   [X] Data types need to be defined in non-system-variant types. For example
       it is not appropriate to use "long" as this type varies by system and
       bitness. This makes the code more "portable" / extensible across
       systems. This still remains in the textout code.
   [X] Create New* functions in providers to initialize the structs.
   [X] config file parser needs to validate input. Most notably is the
       run_every time that is now converted to ms.
   [X] Minor stubs left in the config file code. As of now it *assumes* the
       strings are numbers and allows atoi() to parse them. The code should
       look for non-numerical input and parse multiplying factors (if that
       is appropriate)
   [W] Document *why* the EnableDataPoint() function MUST be called in the
       threaded model (for the safe data type).
   [W] For the PDT_* data types in providers.h, there needs to be malloc
       support in EnableDataPoint() to allocate the "safe" copy of the data.
       Currently there is no method to handle strings.
   [X] Depricate the "old/bad" PDT_* types in providers.h
   [W] Implement "option 3" for the concurrent writer problem. The safe data
       pointer has been created - do the rest. Yoink - I have a better idea.
   [X] Write a better print handler for the output row function in providers.c
   [X] Tons of stubbed out code in configile.c. This needs to be cleaned up.
   [X] The AIX (and other OS) library-based providers need to utilize the
       new API. For sake of getting this done, consider this AIX only. We
       will cross the other-OS road when we get there.
   [X] Comments in the PROVIDER_LIST within the config file are not properly
       parsed. <space><space>#provider::dataitem <--- seen as valid quad
   [X] Code is stubbed for exit (in main()) after parsing the config file.
   [X] Config file parser does not parse argument items from the provider
       list.
   [X] vminfoprov.c has lots of dead code post changeover. This needs to be
       cleaned out.
   [X] stubs and depricated code in configfile.c. This is some easy yanking,
       most of it is commented out and a non-issue.
   [X] Lots of stubs that MUST be cleaned up before we can get a semi-working
       version on the newer framework.
   [X] Revisit the AIX library-based providers. Walking the list may be
       much more appropriate than walking through all the data items.
   [X] Consider moving collect pointer in ptiem to PRIVATE section (redundant)
   [X] Remove format string from pitem
   [X] Does RegisterProvider() properly NULL all linked lists? Please check.
   [X] Write a enable function for vminfoprov.c
   [X] vminfoprov::vminfo_getpitem() - Should look up in the ui list, not
       the ki list.
   [X] lpcputprov is stubbed for all functionality of the new framework.
   [X] Document the APIs in providers.h. (This is important)
   [X] Add something better than ctime (that does not use a colon) to the
       core provider. It should do a localtime() lookup and format, but
       not include colons.
   [X] Add option for field separator (other than ":", such as tab, ",")
   [X] Add a runfor option to the config file parsing
   [X] Add a iteration value option to the config file parsing
   [X] EnableProvider() needs to update the list of output items.
   [X] The output list needs to be a linked list instead of output_order.
   [X] Add the two high resolution timers to the core provider.
   [X] Write core provider
   [X] Refactor all the struct names

*/
#endif
