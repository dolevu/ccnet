/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <event.h>
#include <sys/time.h>
#include <glib.h>

#include "utils.h"
#include "timer.h"

#ifdef WIN32
#include "net.h"
#endif

struct CcnetTimer
{
    struct event   event;
    struct timeval tv;
    TimerCB        func;
    void          *user_data;
    uint8_t        inCallback;
};

static void
timer_callback (int fd, short event, void *vtimer)
{
    int more;
    struct CcnetTimer *timer = vtimer;

    timer->inCallback = 1;
    more = (*timer->func) (timer->user_data);
    timer->inCallback = 0;

    if (more)
        evtimer_add (&timer->event, &timer->tv);
    else
        ccnet_timer_free (&timer);
}

void
ccnet_timer_free (CcnetTimer **ptimer)
{
    CcnetTimer *timer;

    /* zero out the argument passed in */
    g_return_if_fail (ptimer);

    timer = *ptimer;
    *ptimer = NULL;

    /* destroy the timer directly or via the command queue */
    if (timer && !timer->inCallback)
    {
        event_del (&timer->event);
        g_free (timer);
    }
}


CcnetTimer*
ccnet_timer_new (TimerCB         func,
                 void           *user_data,
                 uint64_t        interval_milliseconds)
{
    CcnetTimer *timer = g_new0 (CcnetTimer, 1);

    timer->tv = timeval_from_msec (interval_milliseconds);
    timer->func = func;
    timer->user_data = user_data;

    evtimer_set (&timer->event, timer_callback, timer);
    evtimer_add (&timer->event, &timer->tv);

    return timer;
}
