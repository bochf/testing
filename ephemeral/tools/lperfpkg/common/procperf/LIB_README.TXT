-------------------------------------------------------------------------------
Why just a .a library?

I did not create a .so (shared object) library. There should not be a
significant overlap in the number of running instances of this library. That,
weighed against the distribution dependency resolution (making sure that the
right version of the library was pushed to the right location...) did not
warrant a .so creation. For this reason, I only created a static .a library.
Link appropriately.

-------------------------------------------------------------------------------
Why not use an existing perf library? Presumably one exists.

Yes one probably exists, somewhere. A quick search on the Google did not
return resuls. And the overwhelming concern is that it is probably x86 centric
if it does exist. This library has to support LoP specific formatting of the
/proc files.

-------------------------------------------------------------------------------
What are the included binaries (source) about?

These are all test harnesses

testperf - A test harness of basic library functionality
vcpu     - A test harness of CPU stats. NOT the real vcpu!
