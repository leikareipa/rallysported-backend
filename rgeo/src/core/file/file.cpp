/*
 * 2016-2018 Tarpeeksi Hyvae Soft /
 * RallySportED file access
 *
 * A basic C-based file access wrapper.
 *
 */

#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include "../../core/display.h"
#include "../../core/common.h"
#include "../../core/file.h"

// Pre-reserve some room for file handles.
const uint FH_CACHE_SIZE = 15;
static FILE *FILE_HANDLE_CACHE[FH_CACHE_SIZE] = {NULL};

bool is_a_valid_handle(const file_handle_t h)
{
    bool is = bool((h < NUM_ELEMENTS(FILE_HANDLE_CACHE)) &&
                   (FILE_HANDLE_CACHE[h] != NULL));

    return is;
}

FILE* kfile_exposed_file_handle(file_handle_t handle)
{
    k_assert(is_a_valid_handle(handle), "Can't operate on an inactive file handle.");

    return FILE_HANDLE_CACHE[handle];
}

static file_handle_t f_next_free_handle()
{
    // Loop until we find an unassigned handle.
    file_handle_t h = 0;
    while (1)
    {
        if (FILE_HANDLE_CACHE[h] == NULL)
        {
            break;
        }

        h++;
        k_assert(h < NUM_ELEMENTS(FILE_HANDLE_CACHE), "Iterated out of bounds while looking for a free file handle.");
    }

    return h;
}

void kfile_create_directory(const char *const name, const bool warnIfExists)
{
    #ifdef _WIN32
        #define make_dir(name) { const int r = mkdir(name); if (warnIfExists) { k_assert(r == 0, "Failed to create a new directory."); } }
    #else
        #define make_dir(name) { const int r = mkdir(name, 0760); if (warnIfExists) { k_assert((r == 0), "Failed to create a new directory."); } }
    #endif

    make_dir(name);

    return;
}

void kfile_append_contents(const file_handle_t fhSrc, const file_handle_t fhDst)
{
    FILE *const hSrc = kfile_exposed_file_handle(fhSrc);
    FILE *const hDst = kfile_exposed_file_handle(fhDst);
    const u32 bufSize = 100;
    unsigned char buffer[bufSize];
    u32 len = kfile_file_size(fhSrc);

    while (len > bufSize)
    {
        if (fread(&buffer, 1, bufSize, hSrc) != bufSize ||
            fwrite(&buffer, 1, bufSize, hDst) != bufSize)
        {
            return;
        }

        len -= bufSize;
    }

    for (u32 i = 0; i < len; i++)
    {
        if (fread(&buffer, 1, 1, hSrc) != 1 ||
            fwrite(&buffer, 1, 1, hDst) != 1)
        {
            return;
        }
    }

    fflush(hDst);

    return;
}

bool kfile_getline(const file_handle_t handle,
                   char *const dst, const size_t maxLen)
{
    FILE *const h = kfile_exposed_file_handle(handle);

    size_t i = 0;
    while (i < maxLen && h != NULL)
    {
        char nextChar = getc(h);

        if (nextChar == EOF)
        {
            return false;
        }
        else if ((nextChar == '\r') ||
                 (nextChar == '\n'))
        {
            /// Temp workaround for reading text files in binary where newlines are \r\n.
            if (nextChar == '\r')
            {
                getc(h);
            }

            break;
        }

        dst[i++] = nextChar;
    }

    return true;
}

// Write the given byte into the file x times.
//
void kfile_fill(const unsigned char byte, const unsigned long len, const file_handle_t handle)
{
    size_t r = 1;
    for (unsigned long i = 0; (i < len && r == 1); i++)
    {
        r = fwrite(&byte, 1, 1, kfile_exposed_file_handle(handle));
        k_assert(r == 1, "Failed to fill with the given byte.");
    }

    return;
}

void kfile_flush_file(const file_handle_t handle)
{
    const int r = fflush(kfile_exposed_file_handle(handle));
    if (r != 0)
    {
        ERRORI(("Failed to flush a file."));
    }

    return;
}

void kfile_write_string(const char *const str, const file_handle_t handle)
{
    const int r = fputs(str, kfile_exposed_file_handle(handle));
    k_assert(r != EOF, "Failed to write the given string to file.");

    return;
}

void kfile_write_byte_array(const unsigned char *const src, const unsigned long len, const file_handle_t handle)
{
    const size_t r = fwrite(src, 1, len, kfile_exposed_file_handle(handle));
    k_assert(r == len, "Failed to write the given data to file.");

    return;
}

long kfile_position(const file_handle_t handle)
{
    return ftell(kfile_exposed_file_handle(handle));
}

void kfile_read_byte_array(u8 *dst, const size_t numBytes, const file_handle_t handle)
{
    const size_t r = fread(dst, 1, numBytes, kfile_exposed_file_handle(handle));
    k_assert(r == numBytes, "Failed to read bytes from the file.");

    return;
}

// Returns the size in bytes of the file with the given handle.
//
u32 kfile_file_size(const file_handle_t handle)
{
    k_assert(is_a_valid_handle(handle),
             "Was asked for the file size of an invalid handle. Can't provide.");

    int r = 0;
    FILE *const f = FILE_HANDLE_CACHE[handle];
    const long origPos = ftell(f);

    k_assert((origPos != -1l), "Failed to fetch the size of the given file.");

    // Find the length of the file by seeking to the end and noting the position,
    // then seek back to where we were.
    r = fseek(f, 0L, SEEK_END);
    const long size = ftell(f);

    k_assert((r == 0), "Failed to fetch the size of the given file.");
    k_assert((size != -1l), "Failed to fetch the size of the given file.");

    r = fseek(f, origPos, SEEK_SET);

    k_assert((r == 0), "Failed to fetch the size of the given file.");

    return size;
}

// Returns the file's handle, if successfully opened.
//
file_handle_t kfile_open_file(const char *const filename, const char *const mode)
{
    file_handle_t h = f_next_free_handle();

    DEBUG(("Opening file '%s' with handle %u.", filename, h));

    FILE_HANDLE_CACHE[h] = fopen(filename, mode);
    k_assert((FILE_HANDLE_CACHE[h] != NULL), "Failed to open the given file. Is it read-only?");

    return h;
}

// Seek to a delta from the current position.
//
void kfile_jump(const i32 posDelta, const file_handle_t handle)
{
    k_assert(is_a_valid_handle(handle), "Can't operate on an inactive file handle.");

    const int s = fseek(FILE_HANDLE_CACHE[handle], posDelta, SEEK_CUR);
    k_assert((s == 0), "Failed to seek to the given file position.");

    return;
}

void kfile_rewind_file(const file_handle_t handle)
{
    rewind(kfile_exposed_file_handle(handle));

    return;
}

void kfile_seek(const u32 pos, const file_handle_t handle)
{
    const int s = fseek(kfile_exposed_file_handle(handle), pos, SEEK_SET);
    k_assert((s == 0), "Failed to seek to the given file position.");

    return;
}

void kfile_close_file(const file_handle_t handle)
{
    DEBUG(("Closing file with handle %d.", handle));

    const int cl = fclose(kfile_exposed_file_handle(handle));
    k_assert((cl == 0), "Failed to close the given file.");

    k_assert(is_a_valid_handle(handle), "Can't operate on an inactive file handle.");
    FILE_HANDLE_CACHE[handle] = NULL;

    return;
}
