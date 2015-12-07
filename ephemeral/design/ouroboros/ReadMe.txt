
>>> What does the name mean?

See: https://en.wikipedia.org/wiki/Ouroboros

>>> Why did you choose the name?

Because the heart of ob (ouroboros) is a circular queue with a head and tail
that mark were the reader (head) and writer (tail) are operating.

>>> What does ob do?

ob is a threaded cp. It is a copy tool that tries to minimalize time spent
in a serial read/write loop. The idea is that both can proceed at the maximum
speed possible.

It also can potentially copy a file using O_DIRECT while still "buffering"
the reads / writes. 

>>> Is this a proposed tool for production use?

No. It is a test design.

It lacks the full input exception handling and proper design thinking that
goes into tools like cp. For example: It does not support recursive directory
copying.
