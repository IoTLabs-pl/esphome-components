#pragma once

#include "esphome/components/wmbus_common/Telegram.h"

/** Extra  definitions not exposed by Telegram.h*/
FrameStatus checkWMBusFrame(std::vector<uchar> &data, size_t *frame_length, int *payload_len_out, int *payload_offset, bool only_test);
void removeAnyDLLCRCs(std::vector<uchar> &payload);