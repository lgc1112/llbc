/**
 * @file    TimerScheduler.cpp
 * @author  Longwei Lai<lailongwei@126.com>
 * @date    2013/12/01
 * @version 1.0
 *
 * @brief
 */

#include "llbc/common/Export.h"
#include "llbc/common/BeforeIncl.h"

#include "llbc/core/os/OS_Time.h"

#include "llbc/core/timer/Timer.h"
#include "llbc/core/timer/TimerData.h"

#include "llbc/core/timer/TimerScheduler.h"

__LLBC_INTERNAL_NS_BEGIN

static LLBC_NS LLBC_TimerScheduler *__g_entryThreadTimerScheduler = NULL;

__LLBC_INTERNAL_NS_END

__LLBC_NS_BEGIN

LLBC_TimerScheduler::LLBC_TimerScheduler()
: _maxTimerId(0)
, _enabled(true)
, _destroyed(false)
{
}

LLBC_TimerScheduler::~LLBC_TimerScheduler()
{
    _destroyed = true;

    const size_t size = _heap.GetSize();
    const _Heap::Container &elems = _heap.GetData();
    for (size_t i = 1; i <= size; i++)
    {
        LLBC_TimerData *data = const_cast<LLBC_TimerData *>(elems[i]);
        if (data->validate)
        {
            data->validate = false;
            data->cancelling = true;
            data->timer->OnCancel();

            if (--data->refCount == 0)
                LLBC_Delete(data);
        }
    }
}

void LLBC_TimerScheduler::CreateEntryThreadScheduler()
{
    LLBC_TimerScheduler *&scheduler = 
        LLBC_INTERNAL_NS __g_entryThreadTimerScheduler;
    if (!scheduler)
    {
        scheduler = new LLBC_TimerScheduler;
    }
}

void LLBC_TimerScheduler::DestroyEntryThreadScheduler()
{
    LLBC_XDelete(LLBC_INTERNAL_NS __g_entryThreadTimerScheduler);
}

LLBC_TimerScheduler::_This *LLBC_TimerScheduler::GetEntryThreadScheduler()
{
    return LLBC_INTERNAL_NS __g_entryThreadTimerScheduler;
}

LLBC_TimerScheduler::_This *LLBC_TimerScheduler::GetCurrentThreadScheduler()
{
    __LLBC_LibTls *tls = __LLBC_GetLibTls();
    return reinterpret_cast<_This *>(tls->coreTls.timerScheduler);
}

void LLBC_TimerScheduler::Update()
{
    if (UNLIKELY(!_enabled))
        return;

    LLBC_TimerData *data;
    uint64 now = LLBC_GetMilliSeconds();
    while (_heap.FindTop(data) == LLBC_OK)
    {
        if (now < data->handle)
            break;

        _heap.DeleteTop();
        if (!data->validate)
        {
            if (--data->refCount == 0)
                LLBC_Delete(data);

            continue;
        }

        data->timeouting = true;

        bool reSchedule = true;
        LLBC_Timer *timer = data->timer;
#if LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
        uint64 pseudoNow = now;
        while (pseudoNow >= data->handle)
#endif // LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
        {
            ++data->repeatTimes;
            timer->OnTimeout();

            // Cancel() or Schedule() called.
            if (!data->validate)
            {
                reSchedule = false;
#if LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
                break;
#endif // LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
            }

#if LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
            if (data->period == 0)
                break;

            if (UNLIKELY(pseudoNow < data->period))
                break;

            pseudoNow -= data->period;
#endif // LLBC_CFG_CORE_TIMER_STRICT_SCHEDULE
        }

        if (reSchedule)
        {
            data->timeouting = false;

            uint64 delay = (data->period != 0) ? (now - data->handle) % data->period : 0;
            data->handle = now + data->period - delay;

            _heap.Insert(data);
        }
        else
        {
            if (--data->refCount == 0)
                LLBC_Delete(data);
        }
    }
}

bool LLBC_TimerScheduler::IsEnabled() const
{
    return _enabled;
}

void LLBC_TimerScheduler::SetEnabled(bool enabled)
{
    _enabled = enabled;
}

size_t LLBC_TimerScheduler::GetTimerCount() const
{
    return _heap.GetSize();
}

bool LLBC_TimerScheduler::IsDstroyed() const
{
    return _destroyed;
}

int LLBC_TimerScheduler::Schedule(LLBC_Timer *timer, uint64 dueTime, uint64 period)
{
    if (UNLIKELY(_destroyed))
        return LLBC_ERROR_INVALID;

    LLBC_TimerData *data = new LLBC_TimerData;
    ::memset(data, 0, sizeof(LLBC_TimerData));
    data->handle = LLBC_GetMilliSeconds() + dueTime;
    data->timerId = ++ _maxTimerId;
    data->dueTime = dueTime;
    data->period = period;
    // data->repeatTimes = 0;
    data->timer = timer;
    data->validate = true;
    // data->timeouting = false;
    // data->cancelling = false;
    data->refCount = 2;

    if (timer->_timerData)
    {
        if (--timer->_timerData->refCount == 0)
            LLBC_Delete(timer->_timerData);
    }

    timer->_timerData = data;
    _heap.Insert(data);

    return LLBC_OK;
}

int LLBC_TimerScheduler::Cancel(LLBC_Timer *timer)
{
    if (UNLIKELY(_destroyed))
        return LLBC_ERROR_INVALID;

    LLBC_TimerData *data = timer->_timerData;
    ASSERT(data->timer == timer && 
        "Timer manager internal error, LLBC_TimerData::timer != argument: timer!");

    data->validate = false;
    data->cancelling = true;
    timer->OnCancel();
    data->cancelling = false;

    if (data->timeouting)
        return LLBC_OK;

    if (static_cast<sint64>(data->handle) - 
            LLBC_GetMilliSeconds() >= LLBC_CFG_CORE_TIMER_LONG_TIMEOUT_TIME)
    {
        int delElemRet = _heap.DeleteElem(data);
        ASSERT(delElemRet == LLBC_OK &&
            "Timer manager internal error, Could not found timer data when Cancel long timeout timer!");
        if (--data->refCount == 0)
            LLBC_Delete(data);
    }

    return LLBC_OK;
}

void LLBC_TimerScheduler::CancelAll()
{
    if (UNLIKELY(_destroyed))
        return;

    const size_t size = _heap.GetSize();
    _Heap::Container copyElems(_heap.GetData());
    for (size_t i = 1; i <= size; i++)
    {
        LLBC_TimerData *data = copyElems[i];
        if (UNLIKELY(!data->validate))
            return;

        data->timer->Cancel();
    }
}

__LLBC_NS_END

#include "llbc/common/AfterIncl.h"
