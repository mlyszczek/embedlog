.. include:: in/ref-list.in

========
overview
========
NAME
----

**el_overview** - quick overview of **embedlog** logging library

SYNOPSIS
--------

**embedlog** - is a highly portable **c89** complaint logger (with
additional features for users with **c99** compilers and/or **POSIX**
systems). This library is designed mainly for embedded devices, but can
also be used in high level OSes like **Linux**.

DESCRIPTION
-----------

Logger incorporates features like:

* printing to different outputs (simultaneously) like:

  * stderr
  * syslog (very limited, works on *nuttx* for now)
  * directly to serial device (like ``/dev/ttyS0``)
  * file (with optional rotating and syncing to prevent data loss)
  * automatic file reopening on unexpected events (file deletion, SD remount)
  * custom routine, can be anything, embedlog just calls your
    function with string to print

* timestamping using following clocks:

  * ``clock_t``
  * ``time_t``
  * ``CLOCK_REALTIME`` (requires POSIX)
  * ``CLOCK_MONOTONIC`` (requires POSIX)
  * configurable precision of fraction of seconds (mili, micro, nano)

* file log rotation based on:

  * file size - limit number of files and their sizes
  * date - new file is created every day (or hour, or other choosen timeslice)

* printing file and line information
* 8 predefined log levels (total rip off from **syslog** (2) levels)
* colorful output (for easy error spotting)
* print memory block in wireshark-like output
* fully binary logs with binary data (like CAN frames) to save space

Library implements following functions:

Basic functions are:

.. list-table::
   :header-rows: 1

   * - function
     - description
   * - |el_init|
     - initialize embedlog, call before anything else
   * - |el_cleanup|
     - cleans up any resources allocated by el_init
   * - |el_flush|
     - flushes all buffers to respective underlying devices
   * - |el_print|
     - printf-like, prints log, with previously configured options
   * - |el_perror|
     - prints log, and errno string, saves you time to type strerror(errno)
   * - |el_pmemory|
     - dumps memory location in wireshark/hexdump like format

There are multiple different print functions when standard is not enough:

.. list-table::
   :header-rows: 1

   * - function
     - description
   * - |el_print|
     - standard printf-like function
   * - |el_perror|
     - standard perror function, but it's also printf-like (perror(3) does not take fmt)
   * - |el_pmemory|
     - dumps memory in wireshark/hexdump like format
   * - |el_pmemory_table|
     - like el_pmemory, but also adds ascii table
   * - |el_vprint|
     - just like |el_print| but takes **va_arg** instead format
   * - |el_pbinary|
     - used to print fully binary logs (with binary metadata) to save space
   * - |el_puts|
     - like puts(3), these logs won't be processed and will be printed as-is
   * - |el_putb|
     - similar to |el_puts| but used for binary logging

Printing can be altered to suit your needs:

.. list-table::
   :header-rows: 1

   * - |el_enable_output|
     - allows you to enable specified outputs
   * - |el_enable_file_log|
     - configures and enables logging to file with (or without) log rotation
   * - |el_set_timestamp|
     - configures how (and if) timestamps should be printed with each log
   * - |el_print_extra_info|
     - configures if extra info should be added to logs (file, line and
       function or log origin)
   * - |el_set_file_sync|
     - configures how often **embedlog** should sync logs, to prevent data
       loss on program crash or power loss.
   * - |el_enable_colors|
     - enable ANSI colors, different log level will have different color
   * - |el_set_log_level|
     - set maximum log level to print (like you can leave only warning and more
       severe logs on production)
   * - |el_set_prefix|
     - add custom prefix, that will be added to each log, usefull when multiple
       programs log to single output (like serial device)
   * - |el_set_custom_put|
     - can be used to implement own printing, embedlog will pass fully processed
       string.
   * - |el_enable_thread_safe|
     - can be used if you want to print from multiple threads
   * - |el_set_tty_dev|
     - configure and enable tty (serial device) output
   * - |el_disable_output|
     - disable specified outputs
   * - |el_option|
     - if above functions are not enough, this can be used to really fine tune
       embedlog behaviour.

Each function has its equivalent function that accepts *el* object as
argument. These functions have ``el_o`` prefix instead of ``el_``. This is
helpful if we want to have more than one, separated loggers in a single
program, or when using embedlog on RTOS. Few examples are:

.. code-block:: c

   int el_oinit(struct el *el)
   int el_ocleanup(struct el *el)
   struct el * el_new(void)
   int el_destroy(struct el *el)
   int el_oputs(struct el *el, const char *string)
   int el_oprint(LEVEL, struct el *el, const char *fmt, ...)
   int el_oflush(struct el *el)

**LEVEL** in function declaration is a macro that expands to

.. code-block:: c

   __FILE__, __LINE__, __func__, LOG_LEVEL

but it would be very cumbersome to type that each time, so these helper
macros are defined.  Awailable macros are:

.. code-block:: none

   ELF    Fatal errors, usually precedes application crash
   ELA    Alert, vey major error that should be fixed as soon as possible
   ELC    Critical
   ELE    Error
   ELW    Warning
   ELN    Normal log, but of high importance
   ELI    Information message, shouldn't spam too much here
   ELD    Debug messages, can spam as much as you'd like

So instead of calling

.. code-block:: c

   el_print(__FILE__, __LINE__, __func__, EL_NOTICE, "Notice message");

You can simply call it like

.. code-block:: c

   el_print(ELN, "Notice message");

There are also equivalent macros for use with functions that also
provides *el* object, so the can be used with **el_o** function family.
To make these work, you need to provide **EL_OPTIONS_OBJECT** macro.
Check |el_print| for more info about that.

So instead of calling

.. code-block:: c

   el_oprint(__FILE__, __LINE__, __func__, EL_NOTICE, &g_log_object, "Notice message");

or

.. code-block:: c

   el_oprint(ELN, &g_log_object, "Notice message");

You can simply call it like

.. code-block:: c

   /* One time macro declaration */
   #define EL_OPTIONS_OBJECT &g_log_object

   el_oprint(OELN, "Notice message");

STABLE ABI CONSIDERATION
------------------------

Since **embedlog** is versatile and can work on both hardcore embedded
systems (nuttx, freertos, bare metals) and on big CPUs that can run
Linux, library must provide a way for user to use stack-allocated
objects and stable ABI. Unfortunately there is no way to provide both at
the same time, so **embedlog** provides two solutions so everybody is
happy.

Note that library versioning follows **STABLE ABI** way of using
library. So if feature is added that adds new field to **struct el**,
then this will not be treated as ABI breakage, because if you are using
**el_new** (3) or **el_init** (3) functions, ABI will not be broken.
Only when using **el_oinit** (3) ABI will be broken.

STABLE ABI
^^^^^^^^^^
When stable ABI is required, user must initialize **embedlog** either
with |el_init| or |el_new| function. User also must not
access internal fields directly and all operations on *el* object must
be done through the API functions. |el_new| will malloc necessary
memory for the user which must be freed with |el_destroy|
function. Under no circumstances use **sizeof** on the object nor
struct. Operation will succeed, but may return invalid results when
library is updated. To put it in as few word as possible - you don't
know anything about internals of **struct el**.

STACK ALLOCATED OBJECT
^^^^^^^^^^^^^^^^^^^^^^
When you use library in hardcore embedded environment, where there is
not dynamic linking involved, you are save to ignore any ABI changes,
since you always build your programs alongside the library. But thanks
to that you can avoid **malloc** (3) call and allocate everything on the
stack. To allocate object on stack you must use **el_oinit** (3)
function, and deinitialize object with **el_ocleanup** (3).

If you are using **el_oinit** (3) on systems with dynamic library
support you are suscible to ABI breakage and undefined behaviour when
only new features are added, or even when bugfix introduces new field to
**struct el**. Heck, even recompilation with different flags can break
ABI here. Do not use this approach if your system have dynamic library
support unless you can rebuild all programs against new version of
library.

**You've been warned!**

STRIPPING BIN FROM STRINGS
--------------------------
If you want to use **embedlog**, but during release strip all logs from
final binary, you just have to globally define ``DISABLE_ALL_EMBEDLOG``

``DISABLE_ALL_EMBEDLOG`` will change all embedlog functions into functions
that return 0, so these can still be used in ``if()`` checks, or blocks,
or anywhere else you would normally use **embedlog** functions. During
compilation, compiler should optimize all calls, and linker should remove
all embedlog code from final binary, saving space, and hiding all strings.

This case is usefull during development and testing, when development may
happen on fat MCU with a lot of memory, but final program should work on,
smaller - and cheaper - MCU.

EXAMPLE
-------

Initial setup is very trivial. One should call init function and it will
print to stderr by default. Output can also be customized with proper el
object, see **el_option** (3) for more details.

.. literalinclude:: ../examples/print-simple.c
   :language: c
