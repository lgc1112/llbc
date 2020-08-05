// The MIT License (MIT)

// Copyright (c) 2013 lailongwei<lailongwei@126.com>
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of 
// this software and associated documentation files (the "Software"), to deal in 
// the Software without restriction, including without limitation the rights to 
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
// the Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all 
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifdef __LLBC_CORE_TIMER_TIMER_H__

__LLBC_NS_BEGIN

template <typename ObjType>
LLBC_Timer::LLBC_Timer(ObjType *obj,
                       void(ObjType::*timeoutMeth)(LLBC_Timer *),
                       void(ObjType::*cancelMeth)(LLBC_Timer *),
                       Scheduler *scheduler)
: _scheduler(scheduler ? scheduler : reinterpret_cast<Scheduler *>(__LLBC_GetLibTls()->coreTls.timerScheduler))
, _timerData(NULL)

, _data(NULL)
, _timeoutDeleg(obj && timeoutMeth ? new LLBC_Delegate1<void, ObjType, LLBC_Timer *>(obj, timeoutMeth) : NULL)
, _cancelDeleg(obj && cancelMeth ? new LLBC_Delegate1<void, ObjType, LLBC_Timer *>(obj, cancelMeth) : NULL)
{
}

template <typename ObjectType>
inline void LLBC_Timer::SetTimeoutHandler(ObjectType *object, void ( ObjectType::*timeoutMeth)(LLBC_Timer *))
{
    typedef LLBC_Delegate1<void, ObjectType, LLBC_Timer *> __TimeoutMethDeleg;

    if (UNLIKELY(object == NULL || timeoutMeth == NULL))
        SetTimeoutHandler(static_cast<LLBC_IDelegate1<void, LLBC_Timer *> *>(NULL));
    else
        SetTimeoutHandler(LLBC_New2(__TimeoutMethDeleg, object, timeoutMeth));
}

template <typename ObjectType>
void LLBC_Timer::SetCancelHandler(ObjectType *object, void ( ObjectType::*cancelMeth)(LLBC_Timer *))
{
    typedef LLBC_Delegate1<void, ObjectType, LLBC_Timer *> __CancelMethDeleg;

    if (UNLIKELY(object == NULL || cancelMeth == NULL))
        SetCancelHandler(static_cast<LLBC_IDelegate1<void, LLBC_Timer *> *>(NULL));
    else
        SetCancelHandler(LLBC_New2(__CancelMethDeleg, object, cancelMeth));
}

__LLBC_NS_END

#endif // __LLBC_CORE_TIMER_TIMER_H__
