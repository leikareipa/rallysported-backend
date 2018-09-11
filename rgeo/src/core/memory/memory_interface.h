/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * A basic interface to a basic memory manager.
 *
 * Note that the memory allocated by this interface object doesn't get released
 * when the object goes out of scope. It'll remain until program exit, unless
 * specifically freed before then.
 *
 */

#ifndef HEAP_MEM_H
#define HEAP_MEM_H

#include "../../core/display.h"
#include "../../core/common.h"
#include "../../core/memory.h"
#include "../../core/types.h"

/*
 * TODOS:
 *
 * - syntax gets a bit too cumbersome in more involved use.
 *
 */

// A basic interface for a custom memory manager.
template <typename T>
struct heap_bytes_s
{
    heap_bytes_s(void)
    {
        data = nullptr;
    }

    heap_bytes_s(const uint size, const char *const reason = nullptr)
    {
        alloc(size, reason);

        return;
    }

    T& operator[](const uint offs) const
    {
        k_assert_optional((data != nullptr), "Tried to access null heap memory.");
        k_assert_optional((offs < this->size()), "Accessing heap memory out of bounds with [].");

        return data[offs];
    }

    // Raw pointer to the beginning of the data. For when you want to operate on the
    // raw data as an array.
    //
    T* ptr(void) const
    {
        k_assert_optional(data != nullptr, "Tried to access null heap memory.");

        return data;
    }

    // For e.g. memset() and the like; run the length value through this function
    // to verify that it's not out of bounds.
    //
    uint up_to(const uint size)
    {
        k_assert((size <= this->size()), "Possible memory access out of bounds.");

        return size;
    }

    // Manually assign the data pointer to memory, without allocating it.
    //
    void point_to(T *const newPtr, const int size)
    {
        k_assert(newPtr != nullptr, "Assigning a null pointer.");
        k_assert(data == nullptr, "Can't assign a pointer to a non-null memory object.");
        k_assert(size > 0, "Can't assign with data sizes less than 1.");

        data = newPtr;
        dataSize = size;
        alias = true;

        return;
    }

    void alloc(const int size, const char *const reason = nullptr)
    {
        data = (T*)kmem_allocate(sizeof(T) * size, (reason == nullptr? "(No reason given.)" : reason));
        dataSize = size;

        return;
    }

    void release_memory(void)
    {
        if (data == nullptr)
        {
            DEBUG(("Was called to release a null memory object. Ignoring this."));
            return;
        }

        if (!alias)
        {
            kmem_release((void**)&data);
            dataSize = 0;
        }
        else
        {
            DEBUG(("Marking aliased memory to null instead of releasing it."));
            data = nullptr;
            dataSize = 0;
        }

        return;
    }

    uint size(void) const
    {
        return dataSize;
    }

    bool is_null(void) const
    {
        return !bool(data);
    }

private:
    T *data = nullptr;
    uint dataSize = 0;
    bool alias = false; // Set to true if the data pointer was assigned from outside rather than allocated specifically for this memory object.
};

#endif
