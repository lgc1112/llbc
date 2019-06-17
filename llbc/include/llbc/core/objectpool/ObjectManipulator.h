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

#ifndef __LLBC_CORE_OBJECT_POOL_OBJECT_MANIPULATOR_H__
#define __LLBC_CORE_OBJECT_POOL_OBJECT_MANIPULATOR_H__

#include "llbc/common/Common.h"

__LLBC_NS_BEGIN

/**
 * \brief The pool object manipulator.
 */
class LLBC_ObjectManipulator
{
public:
    /**
     * New object in giving memory.
     */
    template <typename ObjectType>
    static void New(void *mem);

    /**
     * Delete object.
     */
    template <typename ObjectType>
    static void Delete(void *obj);

    /**
     * Reset object.
     */
    template <typename ObjectType>
    static bool Reset(void *obj);

private:
    /**
     * Reset object, this method is called when object has clear function.
     */
    template <typename ObjectType, void (ObjectType::*)()>
    struct serializable_type;
    template <typename ObjectType>
    static bool ResetObj(void *obj, serializable_type<ObjectType, &ObjectType::clear> *);

    /**
     * Reset object, default method.
     */
    template <typename ObjectType>
    static bool ResetObj(void *obj, ...);
};

__LLBC_NS_END

#include "llbc/core/objectpool/ObjectManipulatorImpl.h"

#endif // !__LLBC_CORE_OBJECT_POOL_OBJECT_MANIPULATOR_H__
