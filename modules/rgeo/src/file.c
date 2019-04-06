/*
 * 2018, 2019 Tarpeeksi Hyvae Soft
 * 
 */

#include "common.h"
#include "file.h"

static FILE *FILE_HANDLE_CACHE[5] = {NULL};

/* Returns true if the given handle points to a valid, opened file.*/
int kf_is_valid_handle(const file_handle handle)
{
    return ((handle != KF_INVALID_HANDLE) &&
            (handle < NUM_ELEMENTS(FILE_HANDLE_CACHE)) &&
            (FILE_HANDLE_CACHE[handle] != NULL));
}

static file_handle next_free_handle()
{
    file_handle handle;

    /* Loop until we find an unassigned handle.*/
    handle = 0;
    while (FILE_HANDLE_CACHE[handle] != NULL)
    {
        handle++;
        k_assert((handle != KF_INVALID_HANDLE) &&
                 (handle < NUM_ELEMENTS(FILE_HANDLE_CACHE)), "Couldn't find a free file handle.");
    }

    return handle;
}

int kf_copy_contents(const file_handle srcHandle, const file_handle dstHandle)
{
    DEBUG(("Copying data from file with handle %d to file with handle %d.", srcHandle, dstHandle));

    k_assert((kf_is_valid_handle(srcHandle) && kf_is_valid_handle(dstHandle)),
             "Was asked to copy between two files, but at least one of them was inactive.");

    {
        #define BUFFERSIZE 100
        i32 dataLen = kf_file_size(srcHandle);
        u8 buffer[BUFFERSIZE];

        while (dataLen > 0)
        {
            i32 copyLen = BUFFERSIZE;
            if (copyLen > dataLen) copyLen = dataLen;

            if (fread(&buffer, 1, copyLen, FILE_HANDLE_CACHE[srcHandle]) != copyLen ||
                fwrite(&buffer, 1, copyLen, FILE_HANDLE_CACHE[dstHandle]) != copyLen)
            {
                k_assert(0, "Failed to copy the data.");
                return 0;
            }

            dataLen -= copyLen;
        }
        #undef BUFFERSIZE
    }

    return 1;
}

void kf_read_bytes(u8 *dst, const u32 numBytes, const file_handle handle)
{
    DEBUG(("Was asked to read %lu bytes from file with handle %d.", numBytes, handle));

    k_assert(kf_is_valid_handle(handle), "Was provided an inactive file handle.");

    {
        const size_t r = fread(dst, 1, numBytes, FILE_HANDLE_CACHE[handle]);
        DEBUG(("Read %u bytes from file with handle %u.", r, handle));

        k_assert((r == numBytes), "Failed to read the requested number of bytes.");
    }

    return;
}

file_handle kf_open_file(const char *const filename, const char *const openMode)
{
    DEBUG(("Opening file '%s'.", filename));

    {
        const file_handle handle = next_free_handle();

        FILE_HANDLE_CACHE[handle] = fopen(filename, openMode);
        k_assert((FILE_HANDLE_CACHE[handle] != NULL), "Failed to open the file.");

        DEBUG(("File opened with handle %d.", handle));

        return handle;
    }
}

void kf_close_file(const file_handle handle)
{
    DEBUG(("Closing file with handle %d.", handle));

    assert(kf_is_valid_handle(handle));

    {
        const int r = fclose(FILE_HANDLE_CACHE[handle]);
        assert(r == 0);

        FILE_HANDLE_CACHE[handle] = NULL;
    }

    return;
}

/* Returns the size, in bytes, of the file with the given handle.*/
u32 kf_file_size(const file_handle handle)
{
    DEBUG(("Getting size of file with handle %d.", handle));

    k_assert(kf_is_valid_handle(handle), "Was asked for the file size of an inactive file handle.");

    {
        int r = 0;
        FILE *const f = FILE_HANDLE_CACHE[handle];
        long fileSize = 0;
        const long origPos = ftell(f);

        k_assert((origPos != -1L), "Couldn't find out the size of the given file.");

        /* Find the length of the file by seeking to the end and noting the position,
        * then seek back to where we were.*/
        r = fseek(f, 0L, SEEK_END);
        k_assert((r == 0), "Couldn't find out the size of the given file.");

        fileSize = ftell(f);
        k_assert((fileSize != -1L), "Couldn't find out the size of the given file.");

        r = fseek(f, origPos, SEEK_SET);
        k_assert((r == 0), "Couldn't find out the size of the given file.");

        return (u32)fileSize;
    }
}

/* Seeks to the given byte position. The position is relative to
 * the start of the file.*/
void kf_jump(const u32 pos, const file_handle handle)
{
    k_assert(kf_is_valid_handle(handle), "Was asked for the file size of an inactive file handle.");

    {
        const int r = fseek(FILE_HANDLE_CACHE[handle], pos, SEEK_SET);
        k_assert((r == 0), "Failed to jump to the given position in the file.");
    }

    return;
}

void kf_write_string(const char *const string, const file_handle handle)
{
    DEBUG(("Writing a string to file with handle %u.", handle));

    k_assert(kf_is_valid_handle(handle), "Was asked to write a string to an inactive file handle.");

    {
        const int r = fputs(string, FILE_HANDLE_CACHE[handle]);
        k_assert((r != EOF), "Failed to write the given string to the file.");
    }
}

void kf_write_bytes(const u8 *const src, const u32 len, const file_handle handle)
{
    DEBUG(("Writing %u bytes to file with handle %u.", len, handle));

    k_assert(kf_is_valid_handle(handle), "Was asked to write bytes to an inactive file handle.");

    {
        const size_t r = fwrite(src, 1, len, FILE_HANDLE_CACHE[handle]);
        k_assert((r == len), "Failed to write the given data to the file.");
    }

    return;
}
