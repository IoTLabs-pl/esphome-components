#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>

#define WMBUS_PREAMBLE_SIZE (3)
#define WMBUS_MODE_C_PREAMBLE (0x54)

namespace esphome
{
    namespace wmbus_radio
    {
        struct Packet
        {
            std::vector<uint8_t> data;
            int8_t rssi;
        };

        size_t packetSize(uint8_t l_field);
        size_t get_packet_size(std::vector<unsigned char> &preamble);
    }
} // namespace esphome
