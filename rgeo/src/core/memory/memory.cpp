/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED memory manager
 *
 * A basic memory manager. Allocates a bunch of memory on initialization, then doles
 * it out to callers in small chunks afterwards. Useful to prevent fragmentation and
 * to see how memory is being used by the program, though a limited implementation.
 *
 * Generally, you might access the memory manager through an interface wrapper rather
 * than directly.
 *
 */

#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include "../../core/memory/memory_interface.h"
#include "../../core/memory.h"
#include "../../core/common.h"
#include "../../core/main.h"

/*
 * TODOS:
 *
 * - add dynamic resizing to the memory cache for when it's running short.
 *
 */

static int TOTAL_BYTES_ALLOCATED = 0;
static int TOTAL_BYTES_RELEASED = 0;

static mem_allocation_s ALLOC_TABLE[1024];

static uint MEMORY_CACHE_SIZE = 1 * 1024 * 1024;   // In bytes.

static u8 *MEMORY_CACHE = NULL;
static u8 *NEXT_FREE = NULL;

// Set to 1 to disallow any further allocations from the cache.
static uint CACHE_ALLOC_LOCKED = 0;

static void initialize_cache(void)
{
    DEBUG(("Initializing the memory manager..."));

    // Ideally, we'll only call this function once.
    k_assert(MEMORY_CACHE == NULL, "Calling the memory manager initializer with a non-null memory cache.");

    static_assert(std::is_same<decltype(MEMORY_CACHE), u8*>::value &&
                  sizeof(u8) == 1, "Expected the memory cache to be u8*.");
    MEMORY_CACHE = (u8*)calloc(MEMORY_CACHE_SIZE, 1);
    k_assert(MEMORY_CACHE != NULL, "The memory manager failed to initialize.")

    NEXT_FREE = MEMORY_CACHE;

    return;
}

void* kmem_allocate(const int numBytes, const char *const reason)
{
    k_assert(!CACHE_ALLOC_LOCKED, "Memory allocations are locked, can't add new ones.");
    k_assert(!kmain_program_is_exiting(), "No more memory should be allocated after the program has been asked to terminate.");
    k_assert(numBytes > 0, "Can't allocate sub-byte memory blocks.");

    // Initialize the memory cache the first time the allocator is called.
    if (MEMORY_CACHE == NULL)
    {
        initialize_cache();
    }

    k_assert(NEXT_FREE != NULL, "Didn't have a valid free slot in the memory cache for a new allocation.");
    const u8 *mem = NEXT_FREE;

    k_assert((mem + numBytes) < (MEMORY_CACHE + MEMORY_CACHE_SIZE),
             "Memory allocation would overflow the memory cache size. The cache needs to be made larger.");

    // Find a free spot in the allocation table.
    uint tableIdx = 0;
    bool useOldEntry = false;   // If we decided to use a previously-cleared entry.
    while (1)
    {
        // Take the first unallocated entry. These will always be at the end of
        // the allocation chain, since past allocations are never cleared until
        // the program exits.
        if (ALLOC_TABLE[tableIdx].alloc == NULL)
        {
            break;
        }

        // But also, if there's a released entry of just the right size, we can
        // take that one instead.
        if (ALLOC_TABLE[tableIdx].isUnused &&
            ALLOC_TABLE[tableIdx].numBytes == uint(numBytes))
        {
            useOldEntry = true;
            break;
        }

        tableIdx++;
        k_assert(tableIdx < NUM_ELEMENTS(ALLOC_TABLE),
                 "Memory manager overflowed while looking for room for an allocation.");
    }

    if (useOldEntry)
    {
        ALLOC_TABLE[tableIdx].isReused = true;
    }
    else
    {
        TOTAL_BYTES_ALLOCATED += numBytes;
        NEXT_FREE += numBytes;
    }

    k_assert(strlen(reason) < NUM_ELEMENTS(ALLOC_TABLE[tableIdx].reason),
             "The reason given for an allocation was too long to fit into the string array.");
    strcpy(ALLOC_TABLE[tableIdx].reason, reason);
    ALLOC_TABLE[tableIdx].alloc = mem;
    ALLOC_TABLE[tableIdx].numBytes = numBytes;
    ALLOC_TABLE[tableIdx].isUnused = false;

    k_assert(mem != NULL, "Detected a null return from the memory manager allocator. This should not be the case.");

    return (void*)mem;
}

static uint alloc_table_index_of_pointer(const void *const mem)
{
    uint tableIDx = 0;
    while (ALLOC_TABLE[tableIDx].alloc != mem)
    {
        tableIDx++;
        if (tableIDx >= NUM_ELEMENTS(ALLOC_TABLE))
        {
            k_assert(0, "Couldn't find the requested allocation in the allocation table.");
            goto bail;
        }
    }

    return tableIDx;

    bail:
    throw std::runtime_error("Unexpected code path in the memory manager.");    // We should never reach this, but will if asserts are disabled.
    return 0;
}

// Returns the number of bytes allocated for the memory block starting at the given
// address.
uint kmem_sizeof_allocation(const void *const mem)
{
    if (mem == NULL)
    {
        k_assert(0, "The memory manager was asked for the size of a null allocation.");
        return 0;
    }

    const uint idx = alloc_table_index_of_pointer(mem);

    return ALLOC_TABLE[idx].numBytes;
}

// Mark the given pointer as NULL. Internally, we still retain its pointer and
// will deallocate its memory at the end of the program.
//
void kmem_release(void **mem)
{
    if (*mem == NULL)
    {
        return;
    }

    // Make sure we've been asked to release a valid allocation.
    assert(*mem >= MEMORY_CACHE);
    assert(*mem <= (MEMORY_CACHE + MEMORY_CACHE_SIZE));

    const uint idx = alloc_table_index_of_pointer(*mem);

    // Warn of double deletes.
    if (ALLOC_TABLE[idx].isUnused)
    {
        ERRORI(("Asked to double-delete memory at %p (%s).", *mem, ALLOC_TABLE[idx].reason));
        k_assert(0, "Double-deleting memory.");
    }

    ALLOC_TABLE[idx].isUnused = true;

    if (!ALLOC_TABLE[idx].isReused)
    {
        TOTAL_BYTES_RELEASED += ALLOC_TABLE[idx].numBytes;
    }

    *mem = NULL;

    return;
}

void kmem_lock_cache_alloc(void)
{
    k_assert(CACHE_ALLOC_LOCKED == 0, "Was asked to lock the memory cache, but it had already been locked.");

    CACHE_ALLOC_LOCKED = 1;

    return;
}

void kmem_deallocate_memory_cache(void)
{
    DEBUG(("Releasing the memory cache."));

    k_assert(kmain_program_is_exiting(), "Was asked to release the memory cache before the program had been told to exit.");

    #ifdef RSED_ON_QT // For the dev version.
        DEBUG(("Taking stock of allocations in the memory cache (estimated values)."));
        DEBUG(("Allocated:\t%d KB (%d%% usage)", (TOTAL_BYTES_ALLOCATED / 1024), int((real(TOTAL_BYTES_ALLOCATED) / MEMORY_CACHE_SIZE) * 100)));
        DEBUG(("Released:\t%d KB", (TOTAL_BYTES_RELEASED / 1024)));
        DEBUG(("Balance:\t%d bytes", (TOTAL_BYTES_ALLOCATED - TOTAL_BYTES_RELEASED)));

       /* for (uint i = 0; i < NUM_ELEMENTS(ALLOC_TABLE); i++)
        {
            if ((ALLOC_TABLE[i].alloc != NULL) &&
                !ALLOC_TABLE[i].isUnused)
            {
                DEBUG(("Unreleased memory: %p, %u bytes. Stated purpose: '%s'.", ALLOC_TABLE[i].alloc, ALLOC_TABLE[i].numBytes, ALLOC_TABLE[i].reason));
            }
        }*/

        DEBUG(("(Unreleased allocations will be freed automatically.)"));
    #endif

    free(MEMORY_CACHE);

    return;
}
