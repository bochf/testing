#ifndef VERSION_H
#define VERSION_H
/*
  Version History:
   0.1.0   4/5/13 - Original creation. Base read file capability.
   0.2.0   4/6/13 - Added base compile support
   0.3.0   4/6/13 - Reached a snap point
   0.4.0   4/6/13 - mmap code is in place. Some stubs remain.
                  - Taking a snap and a nap. 
   0.5.0   4/6/13 - Added (full) endian support
                  - File swapping now complete
                  - Debugged issue with mmap()ing an existing DB (and trying
                    to write to the read-only mmap().
   0.6.0   4/7/13 - Added -p(latform) support
                  - Fixed some of the query stuff (case sensitive/insensitive,
                    and the unique list problem)
                  - Time to snap
   0.7.0   4/7/13 - All list queries are done
                  - Basic input validation (source files)
                  - Cleaned up help file
                  - Updated todos
                  - Time to snap
   0.8.0   4/7/13 - Limit by keyword support
                  - Basic input validataion (command line)
                  - Bug squasing
                  - Documenation
   0.9.0   4/7/13 - Start work on the "compressed" database file
   0.10.0  4/8/13 - Successful AIX port
                  - Re-numbered dates in this file
                  - Minor make file fixes
                  - V2 DB file write looks complete. No read at this time.
   0.10.1  4/8/13 - Fixed bug in command line options parsing.
                  - Changed DB version back to 1 - temporarily
   0.11.0  4/8/13 - Changes to DB format to support new data
   0.12.0  4/9/13 - Fixed issues with leading and trailing whitespace in
                    the input fields. This is chomped off from the start
                    and end of the input string data.
                  - Impact is now parsed
                  - Snaping before I start yanking DB V1 support.
   0.13.0  4/9/13 - Wholsale slaughter of DB V1 code
                  - Added query support for V2 databases
   0.13.1  4/9/13 - Changes to this file
   0.14.0 4/10/13 - Added exception handling in ftruncate() section of code
                  - Added -l desc[ription] option
   0.15.0 4/10/13 - Added -k key1,key2 option
   0.16.0 4/11/13 - Added -i option
                  - Linux port fine. (Compiles and runs just peachy)
                  - Solaris port fine. (Fixed a DB alignment issue)
   0.16.1 4/11/13 - Changes only to this file (additional todos)
   0.17.0 4/11/13 - No entries for platform now means ALL platforms
                  - Added code for truncating the -l desc output (This is
                    currently disabled through an ifdef bracket)
   0.17.1 4/11/13 - Enabled the above feature
   0.17.2 4/11/13 - Changes only to this file
   0.18.0 4/18/13 - De-compile (-o) option added
                  - Code exists and is "deactivated". Allow the options.c
                    function ParseOptions to parse the -o option by defining
                    ALLOW_DECOMPILE in this file.
   0.19.0 4/19/13 - Wrote some basic docuemtnation (very limited)
                  - Compile notification (per file) is much more stream-lined.
   0.20.0 10/4/13 - Included <stdint.h> in decompile.c for Linux compile.
                  - Added endian info the version output
*/
#define VERSION_STRING "0.20.0"

/* #define ALLOW_DECOMPILE <------- DISABLED! */

/*
  ToDo:
   [_] Write documentation on usage - FAQ/HowTo

  Done:
   [X] The compile/parse file section should continue on to the validation
       section while continuing "." printing. Currently it has really complex
       logic to handle the validation & display.
   [X] Write a reverse compiler that will generate source files from the
       compiled DB.
   [X] -l desc has issues when the name is > than the allowed size. It 
       adds a CR into the output. This is really annoying.
   [X] When reading in input files... A blank Platform should be interpreted
       as all platforms.
   [X] Get a "Bus Error" when running on Solaris. (Memory alignment issue)
   [X] Put support for -i in query.c
   [X] query.c::dump_item_list() has a STUB/TODO for exception handing with
       case sensitivity - handle it.
   [X] The make file needs to be multi-platform (It is, but uses gcc)
   [X] Need to be able to key (limit/filter) on multiple keys at once.
   [X] Add a -l desc(ription) option to list items with their descriptions.
   [X] Need a "name   Description" output option
   [X] Insure that actual_entries never goes above reserved_entries. [No..
       because actual entries *can* go above reserved entries... but how
       would it? This would be *exceptionally* unlikely. The real test is
       that we are not writing beyond the end of the buffer... and we have
       a fairly good catch for that - a bus error. Either way... because
       of the insanely large defaults (size pre-determination), we are
       VERY unlikely to ever hit this limit. So much so, that it is not
       really worth the effort to "fix".]
   [X] Need to handle exceptions when ftruncate()ing a file in compile.c::
       CloseDatabase();
   [X] Format version 2 of the file needs to have variable length fields
   [X] Every function in query.c that calls GetDBIOffset() needs a V2
       variant.
   [X] Need to implement the "compressed" file format
   [X] Depricate the V1 database. It does not hold the data you need, so
       toss it... then V2DBs can be the only/standard DB.
   [X] Depricate the ability to read mis-endian compiled files. Just 
       recognize that the endian is wrong and then bail. Otherwise you have
       to byte swap on all the offset reads - and there are just too many.
   [X] filefmt.h only requires 6 keys. This should be 7. slurp.c should
       parse the impact strings.
   [X] We have 4 bytes left at the end of the file. I think this is caused
       by two byte writes in compile.c.
   [X] Need to *adjust* when encountering a V1 DB file. This should be
       an impossible event. But we may have some crossover time. This means
       that the read_header() API must grab the info and begin conditionally
       looking for info based on version.
   [X] The use of -k and -l flags are marked as wrong. But should be ok.
   [X] All int accesses from the DB need to be endian safe... or... we need
       to force a re-compile on endian mis-matches.
   [X] None of the query options are parsed (from the command line)
   [X] When reading file, the buffer overflow issue needs to be handled better.
       If an entire line was not read, then we have to read to the EOL (or
       give up).
   [X] Write the query code
   [X] List queries and other queries are not compatible.
   [X] -p ALL is not currently supported. (Low priority)
   [X] -r/Anonymous has a "verbose" DEBUG statement.
   [X] Check input on keys & platforms. Can only be letters, spaces, or 0.
       See discussion notes on Input Validation in slurp.c
   [X] When reading a file, the platform can be assumed/derived? (This can
       be passed via a define in the Makefile.)
   [X] Need to make a uniq_list and ci_uniq_list - a list that only inserts
       unique items, and a case insensitive version of the same. This is
       used for listing of names/keywords/etc...
   [X] Handle when the DB was written on a different endian platform. THere
       is only one piece of data that we care about.
   [X] Unstub compile.c::factor_page_size()
   [X] Write endian determination code
   [X] When mmap()ing an existing db, the metadata is not read into the 
       struct (where it is expected). This is kind of annowying (two copies
       of the data)... but it is a minor duplication and the struct should
       be populated for consistency amongst functions. This should be done
       as part of a read_header() call that validates the header version, 
       handles endian translation, etc...
   [X] Consider using gzopen() for the db file. (Or at least putting a
       define option into the code). No.. Not when it is mmap()ed.
   [X] A failed compile should not leave a missing DB file (if it previously
       existed). So... write the DB file, then move it into place.
   [X] (Possibly) account for offset based access
   [X] Parse paramaters
   [X] Design data structures
   [X] Skeletal layout
*/
#endif
