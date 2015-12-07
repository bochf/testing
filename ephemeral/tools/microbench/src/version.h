/*
  Version History:
   0.1.0   1/14/15 - Initial versioned release.
                   - Make framework mostly defined.
   0.2.0   1/16/15 - Added verbosity functionality
   0.3.0   1/16/15 - Added the ReportStart() API.
   0.4.0   1/20/15 - Added forkmmap code into the framework. Some of the
                     defines are not set via the config file (and should be).
   0.5.0   1/29/15 - Documentation of APIs and some cleanup of the header files
   0.6.0   1/30/15 - Brought in the netmirror code (far from complete)
                   - Compiles clean (AIX / Linux)
   0.7.0   1/30/15 - Inserted stub of receiver code into netmirror
   0.8.0    2/2/15 - Added the sink process management code
                   - Started in on the socket code for netmirror
   0.9.0    2/3/15 - Inserted actual socket setup into sender.c
                   - Compiles clean AIX and Linux
   0.10.0   2/5/15 - Basic socket transfer from sender to mirror working
                   - Snapped a version (git commit)
                   - Added OPTIMAL_BLOCK option to netmirror
   0.11.0   2/9/15 - Fix of time parse in netmirror code
                   - Inserted actual timer in the sender
   0.12.0  2/10/15 - Continued work
   0.13.0  2/18/15 - Added boolean parsing to optparser.c/h
                   - Converted all the Get???Value() functions in optparser
                     to a common "search" function.
                   - Numerous minor fixes to functions in optparser
                   - Quoted (double) values now handled
                   - Fixed boolean test
                   - Added beginnings of the documentation
   0.14.0  2/26/15 - Added ZombieRepleenet() to the MBLibrary
   0.15.0  4/10/15 - Work on library function GetOptionValue().
                   - Added library function EvalOptions() and support for the
                     "eval" keyword. Documented this in the design doc.
                   - Tested on AIX, Linux
*/
#define VERSION_STRING "0.15.0"
/*
  ToDo:
   [ ] Resolve the bitness issues for the library build. This fails on SunOS
       builds specifically. As it stands, all SunOS is 32, and all AIX is 64.
   [ ] Test ZombieRepellent()
   [ ] Document ZompieRepellent() in the MicroBench "whitepaper".
   [_] Spaces are not allowed in the value part of the label = value pair.
       For example:
         set TIME_VALUE = 3 m
       is read in as TIME_VALUE = 3, which defaults to 3 seconds. Taking
       out the space fixes the issue.
   [ ] netmirror: Mirror code is not reaping children
   [ ] All code (main.c) should have this file as a dependency
   [ ] netmirror: Actually write nethelp.c::Send()
   [ ] netmirror: Check for return values on send() in SetupSender()
   [ ] netmirror: The setup of the sink is entirely stubbed.
   [ ] netmirror: Finish out the mirror (receiver) and sink code
   [ ] Write a run_for paramater option for receiver in netmirror
   [ ] Netmirror needs a sequence number in the UDP packets so that the
       sink can watch for gaps.
   [ ] forkmmap should use the config file to set the defines (optionally)
   [ ] Have a method to test time format(s)
   [ ] Properly parse time more complex than a single modifier - such as:
       1 h 1 s
       1:0:1

  Done:
   [X] Create a GetBoolValue() function for the parse library.
   [X] Have all GetXXValue() functions in optparser use get_str_value()
   [X] netmirror needs a run_for = X [<hour>:[<min>:]]<sec> option (with a
       default to seconds)
   [X] Write a packet size option for netmirror. This needs to be supported
       on both inbound and outbound options.
   [X] Potentially put an option in the language for:
       set NAME = value
       wait 30   <------- wait 30 seconds
       go
       (NO: This extension serves no purpose that could not be better
       orchestrated with a wrapper script.)
   [X] lib/optparser.c::IsInvalidOption() only does one test and does not
       allow for or'ing the tests. (There is no point in or'ing the tests at
       this time as they are all incompatible.)
   [X] Write a GetTimeValue() API for optparser.c
   [X] netmirror: Get the bind/listen code working in the receiver.
   [X] lib/optparser.c::IsInvalidOption() should assert() that lower_bound
       is less than upper_bound
   [X] All utilities should use this file.
   [X] forkmalloc should check the VERBOSE value.
   [X] Fill in the version history (start using it).


*/
