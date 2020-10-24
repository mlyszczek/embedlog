/* ==========================================================================
    Licensed under BSD 2clause license. See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ==========================================================================

         -------------------------------------------------------------
        / This is option module, here we parse and manage all options \
        | regarding logging. Also this is home for global default     |
        \ option object                                               /
         -------------------------------------------------------------
          \                         .       .
           \                       / `.   .' "
            \              .---.  <    > <    >  .---.
             \             |    \  \ - ~ ~ - /  /    |
               _____          ..-~             ~-..-~
              |     |   \~~~\.'                    `./~~~/
             ---------   \__/                        \__/
            .'  O    \     /               /       \  "
           (_____,    `._.'               |         }  \/~~~/
            `----.          /       }     |        /    \__/
                  `-.      |       /      |       /      `. ,~~|
                      ~-.__|      /_ - ~ ^|      /- _      `..-'
                           |     /        |     /     ~-.     `-. _  _  _
                           |_____|        |_____|         ~ - . _ _ _ _ _>
   ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#include "el-private.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>


/* ==========================================================================
          __             __                     __   _
     ____/ /___   _____ / /____ _ _____ ____ _ / /_ (_)____   ____   _____
    / __  // _ \ / ___// // __ `// ___// __ `// __// // __ \ / __ \ / ___/
   / /_/ //  __// /__ / // /_/ // /   / /_/ // /_ / // /_/ // / / /(__  )
   \__,_/ \___/ \___//_/ \__,_//_/    \__,_/ \__//_/ \____//_/ /_//____/

   ========================================================================== */


/* global el object, used with all functions that does not take "el"
 * as argument
 */

struct el g_el;

/* mask of outputs that have been compiled and are supported by current
 * build
 */

static const int VALID_OUTS = 0

#if ENABLE_OUT_STDERR
	| EL_OUT_STDERR
#endif

#if ENABLE_OUT_STDERR
	| EL_OUT_STDOUT
#endif

#if ENABLE_OUT_SYSLOG
	| EL_OUT_SYSLOG
#endif

#if ENABLE_OUT_FILE
	| EL_OUT_FILE
#endif

#if ENABLE_OUT_NET
	| EL_OUT_NET
#endif

#if ENABLE_OUT_TTY
	| EL_OUT_TTY
#endif

#if ENABLE_OUT_CUSTOM
	| EL_OUT_CUSTOM
#endif
	;


/* ==========================================================================
                  _                __           ____
    ____   _____ (_)_   __ ____ _ / /_ ___     / __/__  __ ____   _____ _____
   / __ \ / ___// /| | / // __ `// __// _ \   / /_ / / / // __ \ / ___// ___/
  / /_/ // /   / / | |/ // /_/ // /_ /  __/  / __// /_/ // / / // /__ (__  )
 / .___//_/   /_/  |___/ \__,_/ \__/ \___/  /_/   \__,_//_/ /_/ \___//____/
/_/
   ========================================================================== */


/* ==========================================================================
    sets 'option' with 'ap' values in 'el' object.

    errno
            EINVAL      option is invalid
            EINVAL      value for specific option is invalid
            ENOSYS      option was disabled during compilation
   ========================================================================== */


static int el_vooption
(
	struct el      *el,          /* el object to set option to */
	enum el_option  option,      /* option to set */
	va_list         ap           /* option value(s) */
)
{
	int             ret;         /* return code from various functions */
	int             value_int;   /* ap value treated as integer */
	unsigned long   value_ulong; /* ap value treated as unsigned long */
	const char     *value_str;   /* ap value treated as string */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	VALID(EINVAL, 0 <= option && option < EL_OPT_ERROR);

	switch (option)
	{
	/* ==================================================================
	                          __                 __
	                         / /___  _  __ ___  / /
	                        / // -_)| |/ // -_)/ /
	                       /_/ \__/ |___/ \__//_/

	   ================================================================== */


	case EL_LEVEL:
		value_int = va_arg(ap, int);
		VALID(EINVAL, value_int <= 7);
		el_lock(el);
		el->level = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	             ___                      __                 __
	            / _/___ __ __ ___  ____  / /___  _  __ ___  / /
	           / _/(_-</ // // _ \/ __/ / // -_)| |/ // -_)/ /
	          /_/ /___/\_, //_//_/\__/ /_/ \__/ |___/ \__//_/
	                  /___/
	   ================================================================== */


#   if ENABLE_OUT_FILE

	case EL_FSYNC_LEVEL:
		value_int = va_arg(ap, int);
		VALID(EINVAL, value_int <= 7);
		el_lock(el);
		el->fsync_level = value_int;
		el_unlock(el);
		return 0;

#   endif /* ENABLE_OUT_FILE */


	/* ==================================================================
	                                        __
	                            ___  __ __ / /_
	                           / _ \/ // // __/
	                           \___/\_,_/ \__/

	   ================================================================== */


	case EL_OUT:
		value_int = va_arg(ap, int);
		value_int = value_int == EL_OUT_ALL ? VALID_OUTS : value_int;
		VALID(EINVAL, (value_int & ~ALL_OUTS) == 0x00);
		VALID(ENODEV, (value_int & ~VALID_OUTS) == 0x00);
		el_lock(el);
		el->outputs = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	                         _       __    __                 __
	             ___   ____ (_)___  / /_  / /___  _  __ ___  / /
	            / _ \ / __// // _ \/ __/ / // -_)| |/ // -_)/ /
	           / .__//_/  /_//_//_/\__/ /_/ \__/ |___/ \__//_/
	          /_/
	   ================================================================== */


	case EL_PRINT_LEVEL:
		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->print_log_level = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	                               _       __          __
	                   ___   ____ (_)___  / /_  ___   / /
	                  / _ \ / __// // _ \/ __/ / _ \ / /
	                 / .__//_/  /_//_//_/\__/ /_//_//_/
	                /_/
	   ================================================================== */


	case EL_PRINT_NL:
		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->print_newline = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	                                       ___ _
	                      ___   ____ ___  / _/(_)__ __
	                     / _ \ / __// -_)/ _// / \ \ /
	                    / .__//_/   \__//_/ /_/ /_\_\
	                   /_/
	   ================================================================== */


#   if ENABLE_PREFIX

	case EL_PREFIX:
		value_str = va_arg(ap, const char *);
		el_lock(el);
		el->prefix = value_str;
		el_unlock(el);
		return 0;

#   endif /* ENABLE_PREFIX */


	/* ==================================================================
	                                 __
	                     ____ ___   / /___   ____ ___
	                    / __// _ \ / // _ \ / __/(_-<
	                    \__/ \___//_/ \___//_/  /___/

	   ================================================================== */


#   if ENABLE_COLORS

	case EL_COLORS:
		/* only 1 or 0 is allowed, if any other bit is set return EINVAL */

		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->colors = value_int;
		el_unlock(el);
		return 0;

#   endif /* ENABLE_COLORS */


	/* ==================================================================
	                                __
	                               / /_ ___
	                              / __/(_-<
	                              \__//___/

	   ================================================================== */


#   if ENABLE_TIMESTAMP

	case EL_TS:
		value_int = va_arg(ap, int);
		VALID(EINVAL, 0 <= value_int && value_int < EL_TS_ERROR);

		el_lock(el);
		el->timestamp = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	                   __         ___                 __
	                  / /_ ___   / _/____ ___ _ ____ / /_
	                 / __/(_-<  / _// __// _ `// __// __/
	                 \__//___/ /_/ /_/   \_,_/ \__/ \__/

	   ================================================================== */


#       if ENABLE_FRACTIONS

	case EL_TS_FRACT:
		value_int = va_arg(ap, int);
		VALID(EINVAL, 0 <= value_int && value_int < EL_TS_FRACT_ERROR);

		el_lock(el);
		el->timestamp_fractions = value_int;
		el_unlock(el);
		return 0;

#       endif /* ENABLE_FRACTIONS */


	/* ==================================================================
	                          __         __
	                         / /_ ___   / /_ __ _
	                        / __/(_-<  / __//  ' \
	                        \__//___/  \__//_/_/_/

	   ================================================================== */


	case EL_TS_TM:
		value_int = va_arg(ap, int);
		VALID(EINVAL, 0 <= value_int && value_int < EL_TS_TM_ERROR);


#       if ENABLE_REALTIME == 0
		VALID(ENODEV, value_int != EL_TS_TM_REALTIME);
#       endif

#       if ENABLE_MONOTONIC == 0
		VALID(ENODEV, value_int != EL_TS_TM_MONOTONIC);
#       endif

#       if ENABLE_CLOCK == 0
		VALID(ENODEV, value_int != EL_TS_TM_CLOCK);
#       endif

		el_lock(el);
		el->timestamp_timer = value_int;
		el_unlock(el);
		return 0;

#   endif /* ENABLE_TIMESTAMP */


	/* ==================================================================
	                          ___ _        ___
	                         / _/(_)___   / _/___
	                        / _// // _ \ / _// _ \
	                       /_/ /_//_//_//_/  \___/

	   ================================================================== */


#   if ENABLE_FINFO

	case EL_FINFO:
		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->finfo = value_int;
		el_unlock(el);
		return 0;

#   endif /* ENABLE_FINFO */


	/* ==================================================================
	                  ___                 _        ___
	                 / _/__ __ ___  ____ (_)___   / _/___
	                / _// // // _ \/ __// // _ \ / _// _ \
	               /_/  \_,_//_//_/\__//_//_//_//_/  \___/

	   ================================================================== */


#   if ENABLE_FUNCINFO

	case EL_FUNCINFO:

#       if (__STDC_VERSION__ < 199901L)

		/* funcinfo is only supported on c99 compilers */
		errno = ENOSYS;
		return -1;

#       else /* __STDC_VERSION__ < 199901L */

		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->funcinfo = value_int;
		el_unlock(el);
		return 0;

#       endif /* __STDC_VERSION__ < 199901L */

#   endif /* ENABLE_FUNCINFO */


	/* ==================================================================
	                         ___            __   __
	                        / _/___  ___ _ / /_ / /
	                       / _// _ \/ _ `// __// _ \
	                      /_/ / .__/\_,_/ \__//_//_/
	                         /_/
	   ================================================================== */


#   if ENABLE_OUT_FILE

	case EL_FPATH:
		value_str = va_arg(ap, const char *);
		VALID(EINVAL, value_str);
		el_lock(el);
		el->fname = value_str;
		ret = el_file_open(el);
		el_unlock(el);
		return ret;


	/* ==================================================================
	   ___           __         __                           __
	  / _/____ ___  / /_ ___ _ / /_ ___   ___  __ __ __ _   / /  ___  ____
	 / _// __// _ \/ __// _ `// __// -_) / _ \/ // //  ' \ / _ \/ -_)/ __/
	/_/ /_/   \___/\__/ \_,_/ \__/ \__/ /_//_/\_,_//_/_/_//_.__/\__//_/

	   ================================================================== */


	case EL_FROTATE_NUMBER:
	{
		int previous_frotate; /* previous value of frotate number */
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		value_int = va_arg(ap, int);
		VALID(EINVAL, 0 <= value_int && value_int <= USHRT_MAX);
		el_lock(el);
		previous_frotate = el->frotate_number;
		el->frotate_number = (unsigned short)value_int;
		ret = 0;

		if (value_int > 0)
		{
			/* count number of digits in value_int */
			value_int--;
			el->frotate_number_count = 1;
			while ((value_int /= 10)) el->frotate_number_count++;
		}

		/* if user turned on file rotation when file is already
		 * opened without rotation then to prevent weird situations
		 * and even data loss, we reopen file as opening with log
		 * rotation is a bit different. el_file_open() function
		 * will close file before reopening */
		if (previous_frotate == 0 && el->file) ret = el_file_open(el);

		el_unlock(el);
		return ret;
	}


	/* ==================================================================
	           ___           __         __              _
	          / _/____ ___  / /_ ___ _ / /_ ___   ___  (_)___ ___
	         / _// __// _ \/ __// _ `// __// -_) (_-< / //_ // -_)
	        /_/ /_/   \___/\__/ \_,_/ \__/ \__/ /___//_/ /__/\__/

	   ================================================================== */


	case EL_FROTATE_SIZE:
		value_ulong = va_arg(ap, unsigned long);
		VALID(EINVAL, value_ulong >= 1);
		el_lock(el);
		el->frotate_size = value_ulong;
		el_unlock(el);
		return 0;


	/* ==================================================================
	  ___           __         __                          __ _        __
	 / _/____ ___  / /_ ___ _ / /_ ___   ___ __ __ __ _   / /(_)___   / /__
	/ _// __// _ \/ __// _ `// __// -_) (_-</ // //  ' \ / // // _ \ /  '_/
	_/ /_/   \___/\__/ \_,_/ \__/ \__/ /___/\_, //_/_/_//_//_//_//_//_/\_\
	                                       /___/
	   ================================================================== */


	case EL_FROTATE_SYMLINK:
		value_int = va_arg(ap, int);
		VALID(EINVAL, (value_int & ~1) == 0);

		el_lock(el);
		el->frotate_symlink = value_int;
		el_unlock(el);
		return 0;


	/* ==================================================================
	           ___
	          / _/___ __ __ ___  ____  ___  _  __ ___  ____ __ __
	         / _/(_-</ // // _ \/ __/ / -_)| |/ // -_)/ __// // /
	        /_/ /___/\_, //_//_/\__/  \__/ |___/ \__//_/   \_, /
	                /___/                                 /___/
	   ================================================================== */


	case EL_FSYNC_EVERY:
		value_ulong = va_arg(ap, unsigned long);
		VALID(EINVAL, value_ulong >= 0);
		el_lock(el);
		el->fsync_every = value_ulong;
		el_unlock(el);
		return 0;


#   endif  /* ENABLE_OUT_FILE */


	/* ==================================================================
	                    __   __             __
	                   / /_ / /_ __ __  ___/ /___  _  __
	                  / __// __// // / / _  // -_)| |/ /
	                  \__/ \__/ \_, /  \_,_/ \__/ |___/
	                           /___/
	   ================================================================== */


#   if ENABLE_OUT_TTY

	case EL_TTY_DEV:
	{
		unsigned int speed;
		/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


		value_str = va_arg(ap, const char *);  /* serial tty to open */
		speed = va_arg(ap, unsigned int);

		VALID(EINVAL, value_str);

		el_lock(el);
		ret = el_tty_open(el, value_str, speed);
		el_unlock(el);
		return ret;
	}

#   endif /* ENABLE_OUT_TTY */


	/* ==================================================================
	                         __                            __
	         ____ __ __ ___ / /_ ___   __ _    ___  __ __ / /_ ___
	        / __// // /(_-</ __// _ \ /  ' \  / _ \/ // // __/(_-<
	        \__/ \_,_//___/\__/ \___//_/_/_/ / .__/\_,_/ \__//___/
	                                        /_/
	   ================================================================== */


#   if ENABLE_OUT_CUSTOM

	case EL_CUSTOM_PUTS:
		el_lock(el);
		el->custom_put = va_arg(ap, el_custom_put);
		el->custom_put_user = va_arg(ap, void *);
		el_unlock(el);
		return 0;

#   endif /* ENABLE_OUT_CUSTOM */


	/* ==================================================================
	          __   __                       __            ___
	         / /_ / /   ____ ___  ___ _ ___/ / ___ ___ _ / _/___
	        / __// _ \ / __// -_)/ _ `// _  / (_-</ _ `// _// -_)
	        \__//_//_//_/   \__/ \_,_/ \_,_/ /___/\_,_//_/  \__/

	   ==================================================================
	    param
	            int [0,1]

	    description
	            initializes or destroys lock for given "el" object,
	            function does make sure not to double initialize or
	            destroy mutex. This case must be called when no
	            other threads are accessing any of "el" fields.
	  ================================================================== */


#   if ENABLE_PTHREAD

	case EL_THREAD_SAFE:
		value_int = va_arg(ap, int);

		if (value_int)
		{
			/* if lock is already initialized, don't initialize it
			 * again, as this will result in undefined behaviour */
			if (el->lock_initialized) return 0;

			/* lock not initialized, do it now */
			ret = pthread_mutex_init(&el->lock, NULL);
			if (ret > 0)
			{
				/* pthread does not need to set errno, and it
				 * returns errno value as return value from
				 * function, convert it to our standard error
				 * reporting */
				errno = ret;
				return -1;
			}

			el->lock_initialized = 1;
			return 0;
		}

		/* user wants to disable thread safety but lock was not
		 * initialized, can't destroy what has no yet been created. */
		if (el->lock_initialized == 0) return 0;

		pthread_mutex_destroy(&el->lock);
		el->lock_initialized = 0;
		return 0;

#   endif /* ENABLE_PTHREAD */


	/* ==================================================================
	                      __      ___             __ __
	                  ___/ /___  / _/___ _ __ __ / // /_
	                 / _  // -_)/ _// _ `// // // // __/
	                 \_,_/ \__//_/  \_,_/ \_,_//_/ \__/

	   ================================================================== */


	default:
		/* if we get here, user used option that was disabled
		 * during compilation time and is not implemented
		 */

		errno = ENOSYS;
		return -1;
	}

	/* (void) all args here to silent compiler in case some
	 * var is not used due to #if being false */
	(void)value_str;
	(void)value_int;
	(void)value_ulong;
}


/* ==========================================================================
                       __     __ _          ____
        ____   __  __ / /_   / /(_)_____   / __/__  __ ____   _____ _____
       / __ \ / / / // __ \ / // // ___/  / /_ / / / // __ \ / ___// ___/
      / /_/ // /_/ // /_/ // // // /__   / __// /_/ // / / // /__ (__  )
     / .___/ \__,_//_.___//_//_/ \___/  /_/   \__,_//_/ /_/ \___//____/
    /_/
   ========================================================================== */


/* ==========================================================================
    initializes global el object to default state
   ========================================================================== */


/* public api */ int el_init
(
	void
)
{
	return el_oinit(&g_el);
}


/* ==========================================================================
    Sets el object to sane values

    errno
            EINVAL      el is invalid (null)
   ========================================================================== */


/* public api */ int el_oinit
(
	struct el  *el  /* el object */
)
{
	VALID(EINVAL, el);

	memset(el, 0, sizeof(struct el));
	el->outputs = EL_OUT_STDERR;
	el->print_log_level = 1;
	el->print_newline = 1;
	el->level = EL_INFO;
	el->level_current_msg = EL_DBG;

#if ENABLE_OUT_TTY
	el->serial_fd = -1;
#endif

#if ENABLE_OUT_FILE
	el->fsync_every = 32768;
	el->fsync_level = EL_FATAL;
	el->frotate_symlink = 1;
#endif

#if ENABLE_PTHREAD
	el->lock_initialized = 0;
#endif

	return 0;
}


/* ==========================================================================
    Same as el_init but struct el is initialized on heap, so user does not
    have to know anything about internal structure of 'el'.

    errno
            ENOMEM      not enough memory in the system to allocate data
   ========================================================================== */


/* public api */ struct el *el_new
(
	void
)
{
	struct el *el;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	el = malloc(sizeof(*el));
	VALIDR(ENOMEM, NULL, el);

	el_oinit(el);
	return el;
}


/* ==========================================================================
    cleans up whatever has been initialized/reserved by el_init. Due to the
    nature of pthread, this functions must be called only when no other
    threads operate on "el" object.
   ========================================================================== */


/* public api */ int el_cleanup
(
	void
)
{
	return el_ocleanup(&g_el);
}


/* ==========================================================================
    cleans up whatever has been initialized/reserved by el_option and
    el_init calls. Due to the nature of pthread, this functions must be
    called only when no other threads operate on "el" object.

    errno
            EINVAL      el is invalid (null)
   ========================================================================== */


/* public api */ int el_ocleanup
(
	struct el  *el  /* el object */
)
{
	VALID(EINVAL, el);

	el->outputs = 0;
#if ENABLE_OUT_FILE
	el_file_cleanup(el);
#endif

#if ENABLE_OUT_TTY
	if (el->serial_fd != -1) el_tty_close(el);
#endif

#if ENABLE_PTHREAD
	if (el->lock_initialized) pthread_mutex_destroy(&el->lock);
#endif
	return 0;
}


/* ==========================================================================
    Same as el_ocleanup but can be used only with object created on heap
    with el_new().
   ========================================================================== */


/* public api */ int el_destroy
(
	struct el  *el  /* el object */
)
{
	if (el_ocleanup(el) != 0) return -1;
	free(el);
	return 0;
}


/* ==========================================================================
    checks wheter log print is allowed if not. Print is not allowed when log
    level is no high enough or no output has been configured.
   ========================================================================== */


int el_log_allowed
(
	struct el      *el,    /* el object */
	enum el_level   level  /* log level to check */
)
{
	return el->level >= (int)level;
}


/* ==========================================================================
    same as el_ooption but for default g_el object
   ========================================================================== */


/* public api */ int el_option
(
	int      option,   /* option to set */
	         ...       /* option value */
)
{
	va_list  ap;       /* variadic arguments */
	int      rc;       /* return code from el_voooption */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	va_start(ap, (int)option);
	rc = el_vooption(&g_el, option, ap);
	va_end(ap);

	return rc;
}


/* ==========================================================================
    same as el_voel but accepts variadic arguments
   ========================================================================== */


/* public api */ int el_ooption
(
	struct el  *el,       /* el object to set option to */
	int         option,   /* option to set */
	            ...       /* option value(s) */
)
{
	va_list     ap;       /* variadic arguments */
	int         rc;       /* return code from el_voooption */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/


	va_start(ap, option);
	rc = el_vooption(el, option, ap);
	va_end(ap);

	return rc;
}


/* ==========================================================================
    Returns const pointer to global el struct used by all el_ functions
    that don't take el object.
   ========================================================================== */


/* public api */ const struct el *el_get_el
(
	void
)
{
	return (const struct el *)&g_el;
}
