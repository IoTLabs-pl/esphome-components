#include "packets.h"

#include "decode3of6.h"

#define WMBUS_BLOCK_A_PREAMBLE (0xCD)
#define WMBUS_BLOCK_B_PREAMBLE (0x3D)

namespace esphome
{
    namespace wmbus_radio
    {
        size_t packetSize(uint8_t l_field)
        {
            // The 2 first blocks contains 25 bytes when excluding CRC and the L-field
            // The other blocks contains 16 bytes when excluding the CRC-fields
            // Less than 26 (15 + 10)
            auto nrBlocks = l_field < 26 ? 2 : (l_field - 26) / 16 + 3;

            // Add all extra fields, excluding the CRC fields + 2 CRC bytes for each block
            auto nrBytes = l_field + 1 + 2 * nrBlocks;

            return nrBytes;
        }

        size_t get_packet_size(std::vector<unsigned char> &preamble)
        {
            if (preamble.size() == WMBUS_PREAMBLE_SIZE)
                if (preamble[0] == WMBUS_MODE_C_PREAMBLE)
                {
                    switch (preamble[1])
                    {
                    case WMBUS_BLOCK_A_PREAMBLE:
                        return 2 + packetSize(preamble[2]);
                    case WMBUS_BLOCK_B_PREAMBLE:
                        return 2 + 1 + packetSize(preamble[2]);
                    }
                }
                else
                {
                    auto decoded = decode3of6(preamble);
                    if (decoded.has_value())
                        return encoded_size(packetSize((*decoded)[0]));
                }

            return 0;
        }
    }
}