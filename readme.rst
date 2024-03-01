========
embedlog
========
Quick intro
-----------

.. code-block:: c

   #include <embedlog.h>

   int main(void) {
       el_init();

       /* Turn on timestamp and information from which log originates */
       el_set_timestamp(EL_TS_LONG, EL_TS_TM_REALTIME, EL_TS_FRACT_US);
       el_print_extra_info(1);

       /* Enable log to file, create max 10 files, each of 1MB of max size */
       el_enable_file_log("/tmp/embedlog-test-file.log", 10, 1024 * 1024);
       el_print(ELN, "from now on, logs will be printed to both file and stderr");
       el_print(ELF, "log with fatal severity");

       el_cleanup();
       return 0;
   }

Above program will print::

   [2024-02-29 14:05:51.929980][print-example.c:12:main()] n/from now on, logs will be printed to both file and stderr
   [2024-02-29 14:05:51.930005][print-example.c:13:main()] f/log with fatal severity

Dump memory locations in readable formats

.. code-block:: c

   const char *ascii = get_ascii_table();
   el_pmemory(ELN, ascii, strlen(ascii));

Output::

   i/0x0000  00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  ........ ........
   i/0x0010  10 11 12 13 14 15 16 17  18 19 1a 1b 1c 1d 1e 1f  ........ ........
   i/0x0020  20 21 22 23 24 25 26 27  28 29 2a 2b 2c 2d 2e 2f   !"#$%&' ()*+,-./
   i/0x0030  30 31 32 33 34 35 36 37  38 39 3a 3b 3c 3d 3e 3f  01234567 89:;<=>?
   i/0x0040  40 41 42 43 44 45 46 47  48 49 4a 4b 4c 4d 4e 4f  @ABCDEFG HIJKLMNO
   i/0x0050  50 51 52 53 54 55 56 57  58 59 5a 5b 5c 5d 5e 5f  PQRSTUVW XYZ[\]^_
   i/0x0060  60 61 62 63 64 65 66 67  68 69 6a 6b 6c 6d 6e 6f  `abcdefg hijklmno
   i/0x0070  70 71 72 73 74 75 76 77  78 79 7a 7b 7c 7d 7e 7f  pqrstuvw xyz{|}~.

About
-----

Logger written in **C89** targeting embedded systems. It's main goals are to be
small and fast. Many features can be disabled during compilation to generate
smaller binary. Amount of needed stack memory can be lowered by adjusting
maximum printable size of single log message. Altough logger focuses on embedded
systems, it is general **c/c++** logger that can be used in any application.
Implemented features are (most of them optionally configured in runtime):

* printing to different outputs (simultaneously) like:

  * syslog (very limited, works on *nuttx* for now)
  * directly to serial device (like /dev/ttyS0)
  * standard error (stderr)
  * standard output (stdout)
  * file (with optional log rotate, and syncing to prevent data loss)
  * automatic file reopening on unexpected events (file deletion, SD remount)
  * custom routine - can be anything **embedlog** just calls custom function
    with string to print

* appending timestamp to every message

  * clock_t
  * time_t
  * CLOCK_REALTIME (requires POSIX)
  * CLOCK_MONOTONIC (requires POSIX)
  * configurable precision of fraction of seconds (mili, micro, nano)

* file log rotation based on:

  * file size - limit number of files and their sizes
  * date - new file is created every day (or hour, or other choosen timeslice)

* print location of printed log (file and line)
* print function name of printed log (required at least c99)
* 8 predefinied log levels (total rip off from syslog(2))
* colorful output (ansi colors) for easy error spotting
* print memory block in wireshark-like output
* fully binary logs with binary data (like CAN frames) to save space
* full thread safety using *pthread*
* stripping binary from all strings and **embedlog** code in final release image

Almost all of these features can be disabled to save some precious bytes of
memory.

Functions description
---------------------
Overview of the library (functions, use cases etc.) can be found at
:ref:`el_overview(7) <el_overview.7:overview>` This is very good starting
point to get to know library better.

For detailed description of every function check out
project documentation at `manual pages <manuals/index.html#manual-pages>`_

Examples
--------
Check `examples <https://git.bofc.pl/embedlog/tree/examples>`_ directory to get
the idea of how to use **embedlog**. Examples can also be compiled to see how
they work.

Short examples are also in the documentation.

Dependencies
------------
Library is written in **C89** but some features require implemented **POSIX** to
work. Also there are some additional features for users with **C99** compiler.

To run unit tests, you also need `librb <https://librb.bofc.pl>`_

Robustness
----------
Code is about 800 lines of code of which 300 are just to set and validate
passed options. -Wall and -Wextra are standard, and code is code is passed
through sanitizers and static code analyzers before release.
Code is also very heavily tested::

   # total tests.......:61620
   # passed tests......:61620
   # failed tests......:   0
   # total checks......:66181
   # passed checks.....:66181
   # failed checks.....:   0

   Overall coverage rate:
     lines...........: 89.5% (739 of 826 lines)
     functions.......: 90.6% (77 of 85 functions)

Compiling and installing
------------------------
Compiling library
^^^^^^^^^^^^^^^^^

Project uses standard automake so to build you need to:::

  $ autoreconf -i
  $ ./configure
  $ make
  # make install

Running tests
^^^^^^^^^^^^^
To run test simply run::

  $ make check

Compiling examples
^^^^^^^^^^^^^^^^^^
Compile examples with::

  $ cd examples
  $ make

Build time options
------------------
Many features can be disabled to save space and ram. While this may not be
neccessary to change on big operating systems such as **linux** or **freebsd**,
it may come in handy when compiling for very small embedded systems. All options
are passed to configure script in common way **./configure --enable-_feature_**.
Run **./configure --help** to see help on that matter. For all **--enable**
options it is also valid to pass **--disable**. Enabling option here does not
mean it will be hard enabled in runtime, this will just give you an option to
enable these settings later in runtime.

--enable-out-stderr (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When set, library will be able to print logs to standard error output (stderr)
and standard output (stdout). Nothing fancy.

--enable-out-file (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows to configure logger to print logs to file. Optional file rotation can be
enabled. Number of rotation files and maximum size of rotation log file can be
defined in runtime

--enable-out-custom (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows to pas own function which will receive fully constructed message to print
as **const char \***. Usefull when there is no output facility that suits your
needs.

--enable-timestamp (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, logger will be able to add timestamp to every message. Timestamp
can be in short or long format and timer source can be configured. Check out
:ref:`el_set_timestamp(3) <manuals/options/el_set_timestamp.3:el_set_timestamp>`
to read more about it.

--enable-fractions (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, logger will be able to add fractions of seconds to each message.
Fractions are added after reguler timestamp in format ".mmm" where mmm is
fractions of seconds in milliseconds. This can be tuned to use micro or even
nanoseconds - if system has such resolution.

--enable-realtime, --enable-monotonic (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows to use better precision timers - **CLOCK_REALTIME** and
**CLOCK_MONOTONIC** but requires **POSIX**

--enable-clock (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Allows logger to use clock(3) as time source

--enable-binary-logs (default: disable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This will allow you to log binary data (like data read from CAN). Such logs
cannot be read with ordinary *cat* or *less* and will ned custom-made log
decoder, but such logs will use much less space on block devices. This of
course can be used with file rotation. This doesn't work with *stderr* or
*syslog* output as it would make no sense to send binary data there

--enable-prefix (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
This will allow user to add custom string prefix to each message printed.
Very usefull when multiple programs logs to single source (like *syslog* or
*stderr*, it's easier to distinguish who sent that log. It's also usefull
when you want to merge logs from multiple files into on big file of logs.

--enable-finfo (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, information about line and file name from where log originated
will be added to each message.

--enable-funcinfo (default: disable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, information about function name from where log originated
will be added to each message. This uses *__func__* so you need compiler
that supports that. It was added in *c99* standard.

--enable-colors (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
If enabled, output logs can be colored depending on their level. Good for
quick error spotting.

--enable-colors-extended (default: disable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enable, *embedlog* will use more colors for some log levels. Without that
some log levels will have same output color. Not all terminals/tools supports
extended colors.

--enable-reentrant (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Uses reentrant functions where possible. Not available on every platform, but
if enabled, provides thread-safety on line level - that means, lines won't
overlap with another thread. This is true only when output is *stderr* or
*stdout*, when output is *file*, you need to use true thread safety with
the help of **EL_THREAD_SAFE** and **--enable-pthread**.

--enable-pthread (default: enable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, you will be able to configure **embedlog** to use
**EL_THREAD_SAFE**, which will provide full thread safety in all circumstances.
This is critical if output is other than *stderr* or *stdout* - like *file*,
as there is internal state in *el* object that is kept between calls.

--enable-portable-snprintf (default: disable)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
When enabled, library will use internal implementation of **snprintf** even if
**snprintf** is provided by the operating system.

Contact
-------
Michał Łyszczek <michal.lyszczek@bofc.pl>

License
-------
Library is licensed under BSD 2-clause license. See
`LICENSE <https://git.bofc.pl/embedlog/tree/LICENSE>`_ file for details

See also
--------
* `c89 snprintf function family <https://www.ijs.si/software/snprintf>`_ by
  Mark Martinec
* `mtest <https://mtest.bofc.pl>`_ unit test framework **embedlog** uses
* `librb <https://librb.bofc.pl>`_ ring buffer used in unit tests
* `git repository <http://git.bofc.pl/embedlog>`_ to browse code online
* `continous integration <http://ci.embedlog.bofc.pl>`_ for project
* `polarhome <http://www.polarhome.com>`_ nearly free shell accounts for virtually
  any unix there is.
* `pvs studio <https://www.viva64.com/en/pvs-studio>`_ static code analyzer with
  free licenses for open source projects
