/* ==========================================================================
    Licensed under BSD 2clause license See LICENSE file for more information
    Author: Michał Łyszczek <michal.lyszczek@bofc.pl>
   ========================================================================== */


/* ==========================================================================
          _               __            __         ____ _  __
         (_)____   _____ / /__  __ ____/ /___     / __/(_)/ /___   _____
        / // __ \ / ___// // / / // __  // _ \   / /_ / // // _ \ / ___/
       / // / / // /__ / // /_/ // /_/ //  __/  / __// // //  __/(__  )
      /_//_/ /_/ \___//_/ \__,_/ \__,_/ \___/  /_/  /_//_/ \___//____/

   ========================================================================== */


#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include "el-private.h"

#include <time.h>


/* ==========================================================================
    returns seconds and nanoseconds calculated from clock() function
   ========================================================================== */


#if ENABLE_TIMESTAMP
#if ENABLE_CLOCK
static void el_ts_clock
(
    time_t   *s,   /* seconds will be stored here */
    long     *ns   /* nanoseconds will be stored here */
)
{
    clock_t  clk;  /* clock value */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    clk = clock();

    *s   = clk / CLOCKS_PER_SEC;
    *ns  = clk % CLOCKS_PER_SEC;        /* [cps] */
    *ns *= (1000000L / CLOCKS_PER_SEC); /* [ms] */
    *ns *= 1000L;                       /* [ns] */
}
#endif /* ENABLE_CLOCK */
#endif /* ENABLE_TIMESTAMP */

/* ==========================================================================
    returns seconds and nanoseconds calculated from time() function.
   ========================================================================== */


#if ENABLE_TIMESTAMP
static void el_ts_time
(
    time_t  *s,  /* seconds will be stored here */
    long    *ns  /* nanoseconds will be stored here */
)
{
    *s = time(NULL);
    *ns = 0;
}
#endif /* ENABLE_TIMESTAMP */

/* ==========================================================================
    returns seconds and nanoseconds calculated from clock_gettime function
   ========================================================================== */


#if ENABLE_TIMESTAMP
#if ENABLE_REALTIME || ENABLE_MONOTONIC
static void el_ts_clock_gettime
(
    time_t         *s,     /* seconds will be stored here */
    long           *ns,    /* nanoseconds will be stored here */
    clockid_t       clkid  /* clock id */
)
{
    struct timespec tp;    /* clock value */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    clock_gettime(clkid, &tp);

    *s  = tp.tv_sec;
    *ns = tp.tv_nsec;
}
#endif /* ENABLE_REALTIME || ENABLE_MONOTONIC */
#endif /* ENABLE_TIMESTAMP */


/* ==========================================================================
    returns current  timestamp  in  'buf'  location.   Depending  on  global
    settings timestamp can be in long or short format.  Function will return
    number of bytes copied to 'buf'.  If timestamp has been disabled  during
    compilation time or in runtime during settings, function will return  0.
   ========================================================================== */


size_t el_timestamp
(
    struct el_options  *options,  /* options defining printing style */
    void               *b,        /* buffer where timestamp will be stored */
    int                 binary    /* 1 if timestamp should be binary */
)
{
#if ENABLE_TIMESTAMP
    time_t              s;        /* timestamp seconds */
    long                ns;       /* timestamp nanoseconds */
    size_t              tl;       /* timestamp length */
    char               *buf;      /* b threated as known type */
    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    if (options->timestamp == EL_TS_OFF)
    {
        /*
         * user doesn't want us to print timestamp, that's fine
         */

        return 0;
    }

    buf = b;

    /*
     * first we get seconds and nanoseconds from proper timer
     */

    switch (options->timestamp_timer)
    {
#   if ENABLE_REALTIME

    case EL_TS_TM_REALTIME:
        el_ts_clock_gettime(&s, &ns, CLOCK_REALTIME);
        break;

#   endif

#   if ENABLE_MONOTONIC

    case EL_TS_TM_MONOTONIC:
        el_ts_clock_gettime(&s, &ns, CLOCK_MONOTONIC);
        break;

#   endif

    case EL_TS_TM_TIME:
        el_ts_time(&s, &ns);
        break;

#   if ENABLE_CLOCK

    case EL_TS_TM_CLOCK:
        el_ts_clock(&s, &ns);
        break;

#   endif

    default:
        /*
         * bad timer means no timer
         */

        return 0;
    }

    if (binary)
    {
        /*
         * put encoded seconds timestamp in buf
         */

#   ifdef LLONG_MAX
        tl = el_encode_number((unsigned long long)s, (unsigned char *)buf);
#   else
        tl = el_encode_number((unsigned long)s, (unsigned char*)buf);
#   endif

        /*
         * put encoded nano/micro/milli seconds in buf if enabled
         */
#   if ENABLE_FRACTIONS

        switch (options->timestamp_fractions)
        {
        case EL_TS_FRACT_OFF:
            /*
             * we don't add  fractions,  so  simply  return  what  has  been
             * already stored (only seconds)
             */

            return tl;

        case EL_TS_FRACT_MS:
            tl += el_encode_number(ns / 1000000, buf + tl);
            return tl;

        case EL_TS_FRACT_US:
            tl += el_encode_number(ns / 1000, buf + tl);
            return tl;

        case EL_TS_FRACT_NS:
            tl += el_encode_number(ns, buf + tl);
            return tl;

        default:
            /*
             * something  went  somehere  seriously  wrong,  act  like   no
             * timestamp has been configured
             */

            return 0;
        }
#else  /* ENABLE_FRACTIONS */
        return tl;
#endif /* ENABLE_FRACTIONS */
    }
    else
    {
        /*
         * then convert retrieved time into string timestamp
         */

        if (options->timestamp == EL_TS_LONG)
        {
            struct tm   tm;   /* timestamp splitted */
            struct tm  *tmp;  /* timestamp splitted pointer */
            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#   if ENABLE_REENTRANT
            tmp = gmtime_r(&s, &tm);
#   else
            tmp = gmtime(&s);
#   endif

            tl = strftime(buf, 21, "[%Y-%m-%d %H:%M:%S", tmp);
        }
        else
        {
#   ifdef LLONG_MAX
            tl = sprintf(buf, "[%lld", (long long)s);
#   else
            /*
             * if long long is not available, code may be suscible  to  2038
             * bug.  If you are sure your compiler does  support  long  long
             * type, but doesn't define  LLONG_MAX,  define  this  value  to
             * enable long long.
             */

            tl = sprintf(buf, "[%ld", (long)s);
#   endif
        }

        /*
         * if requested, add proper fractions of seconds
         */
#   if ENABLE_FRACTIONS
        switch (options->timestamp_fractions)
        {
        case EL_TS_FRACT_OFF:
            /*
             * we don't add fractions, so simply close opening bracker '['
             */

            buf[tl++] = ']';
            return tl;

        case EL_TS_FRACT_MS:
            tl += sprintf(buf + tl, ".%03ld]", ns / 1000000);
            return tl;

        case EL_TS_FRACT_US:
            tl += sprintf(buf + tl, ".%06ld]", ns / 1000);
            return tl;

        case EL_TS_FRACT_NS:
            tl += sprintf(buf + tl, ".%09ld]", ns);
            return tl;

        default:
            /*
             *  something  went  somehere  seriously  wrong,  act  like   no
             *  timestamp has been configured
             */

            return 0;
        }
#   else  /* ENABLE_FRACTIONS */
        buf[tl++] = ']';
        return tl;
#   endif /* ENABLE_FRACTIONS */
    }

#else  /* ENABLE_TIMESTAMP */
    return 0;
#endif /* ENABLE_TIMESTAMP */
}
