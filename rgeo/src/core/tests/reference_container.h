/*
 * 2018 Tarpeeksi Hyvae Soft /
 * RallySportED
 *
 * Creates into itself a set of predictable data that can be saved to disk as
 * a valid RallySportED project. The project can then be loaded into RallySportED
 * and tested for validity (see the existing validation tests for examples of
 * its use).
 *
 */

#include "../../core/types.h"

class reference_container_c
{
    typedef u32 element_header_t;   // The element header is a 4-byte value specifying the length of the following data block.
    static_assert(sizeof(element_header_t) == 4, "The element header must be 4 bytes in size.");

public:
    reference_container_c(const std::string &containerFilename, const std::string &manifFilename) :
        fileHandle(kfile_open_file(containerFilename.c_str(), "wb+")),
        manifHandle(kfile_open_file(manifFilename.c_str(), "wb+"))
    {
        // Keep track of a cumulative offset for successive elements in the container,
        // and return a pointer to the beginning of the next data block.
        auto data_block = [&](const uint size)
                          {
                              static uint offs = 0;
                              u8 *const p = (rawData + offs + sizeof(element_header_t));
                              offs += (size + sizeof(element_header_t));
                              return std::pair<uint, u8*>{size, p};
                          };

        // Establish a list of all of the user-accessible elements in the container.
        // Note that the order in which these are entered must match the fixed order
        // in which RGEO expects to find them in the container.
        containerElements = {{"MAASTO",  {data_block(maastoSize)}},
                             {"VARIMAA", {data_block(varimaaSize)}},
                             {"PALAT",   {data_block(palatSize)}},
                             {"ANIMS",   {data_block(animsSize)}},
                             {"TEXT1",   {data_block(text1Size)}},
                             {"KIERROS", {data_block(kierrosSize)}},};

        initialize_container();
        create_manifesto();

        return;
    }
    ~reference_container_c()
    {
        kfile_close_file(fileHandle);
        kfile_close_file(manifHandle);

        delete [] rawData;
    }

    // For indexing into the container's data to modify its bytes. Provide the
    // name of the element whose data you want, and the offset of the particular
    // byte.
    //
    u8& operator()(const std::string &elementName, const uint offs) const
    {
        if (elementName == "ALL")
        {
            k_assert(0, "The 'ALL' element isn't supported for write-enabled container access.");

            static u8 b = 0;
            return b;
        }

        const std::pair<uint, u8*> element = containerElements.at(elementName);
        const uint elemSize = element.first;
        u8 *elemData = element.second;

        k_assert(offs < elemSize, "Tried to access an element byte out of bounds.");

        return elemData[offs];
    }

    u8& byte_at(const std::string &elementName, const uint offs) const
    {
        return this->operator()(elementName, offs);
    }

    // Returns a pointer to the beginning of the given element's data block, with
    // which the bytes of the entire block (and beyond, of course) can be read by
    // the caller.
    //
    const u8* operator()(const std::string &elementName) const
    {
        if (elementName == "ALL")   // Special case, return a (const) pointer to the container's root.
        {
            return rawData;
        }

        const std::pair<uint, u8*> element = containerElements.at(elementName);
        u8 *elemData = element.second;

        return elemData;
    }

    uint element_size(const std::string &elementName) const
    {
        const std::pair<uint, u8*> element = containerElements.at(elementName);
        const uint elemSize = element.first;

        return elemSize;
    }

    uint content_size(void) const
    {
        return dataSize;
    }

    // Returns a pointer to the data, after validating that the data are e.g.
    // formatted correctly for the requirements of the container's file format.
    // Note: Calling this function will alter the container's contents, to the
    // extent that the element headers are rewritten.
    //
    const u8* contents(void) const
    {
        // Insert the correct element headers to the beginning of each element.
        for (const auto e: containerElements)
        {
            const auto dataBlock = e.second;
            const uint blockSize = dataBlock.first;
            u8 *const blockData = dataBlock.second - sizeof(element_header_t);

            // Bounds-check.
            if ((blockData < rawData) ||
                ((blockData + sizeof(element_header_t)) > (rawData + dataSize)))
            {
                continue;
            }

            *(element_header_t*)blockData = blockSize;
        }

        return rawData;
    }

    // Various track parameters.
    const uint trackSideLen = 128;                      // How many tiles per side the track has. RGEO supports tracks of 64 and 128 tiles per side, but no others.
    const uint trackId = (trackSideLen == 128? 4 : 1);  // Which in-game track this represents. We need to pick one that supports the given side length.

    // Files.
    const file_handle_t fileHandle;         // A handle to the file on disk that is to be associated with this container.
    const file_handle_t manifHandle;        // A hande for the manifesto (.$FT) file.

private:
    void initialize_container(void)
    {
        // Pre-fill all of the container with predictable data.
        for (uint i = 0; i < this->dataSize; i++)
        {
            static u8 v = 0;
            this->rawData[i] = v++;
        }

        // Massage the MAASTO data such that it tests all the possible combinations
        // of track heightmap values. (Doesn't test invalid ranges.)
        for (uint i = 0; i < this->element_size("MAASTO") / 2; i++)
        {
            static u8 offsList[3]= {0, 1, 255}; // The three offset types Rally-Sport uses in its heightmaps.
            const u8 offs = offsList[((i / 256) % NUM_ELEMENTS(offsList))]; // For each offset type, produduce all 256 consecutive height values.
            uint height = (i % 256);

            this->byte_at("MAASTO", i*2) = height;
            this->byte_at("MAASTO", i*2+1) = offs;
        }

        // Dump the reference container to disk, for RGEO to load it from.
        kfile_write_byte_array(this->contents(), this->content_size(), this->fileHandle);

        // Keep the file handle open - we'll pass it around as needed to access the
        // exported container on disk. Rewind it for convenience.
        kfile_rewind_file(this->fileHandle);

        return;
    }

    void create_manifesto(void)
    {
        char manifesto[128];
        const int r = snprintf(manifesto, NUM_ELEMENTS(manifesto),  "0 %d %d %d\n"
                                                                    "2 3\n"                 // Set the track to have three props.
                                                                    "5 1 11 21 101 201\n"   // Position the prop.
                                                                    "4 1 1\n"               // Set prop's type.
                                                                    "5 2 12 22 102 202\n"
                                                                    "4 2 2\n"
                                                                    "5 3 13 23 103 203\n"
                                                                    "4 3 3\n"
                                                                    "99\n\n", trackId, 1/*pala id*/, LOADER_MAJOR_VERSION);
        k_assert((r > 0) && (r < NUM_ELEMENTS(manifesto)), "Failed to create reference manifesto data. Have to bail.");

        kfile_write_string(manifesto, manifHandle);
        kfile_rewind_file(manifHandle);

        return;
    }

    const uint dataSize         = 400 * 1024;           // How large the container is, in bytes.
    /// TODO. Verify that the given size of data is sufficient to hold all the elements.
    u8 *const rawData           = new u8[dataSize];     // The container's raw data

    // The sizes of the elements in this container.
    const uint maastoSize       = trackSideLen * trackSideLen * 2;
    const uint varimaaSize      = trackSideLen * trackSideLen;
    const uint palatSize        = kpalat_max_num_palas() * kpalat_pala_width() * kpalat_pala_height();
    const uint animsSize        = palatSize;
    const uint kierrosSize      = 10; /// KIERROS not implemented yet.
    const uint text1Size        = 10; /// TEXT1 not implemented yet.

    // The individual elements (relevant Rally-Sport files) stored in the container.
    std::map<std::string, std::pair<uint, u8*>> containerElements;
};

