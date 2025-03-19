#pragma once

#include "esphome/core/automation.h"
#include "component.h"
#include "packet.h"

namespace esphome
{
    namespace wmbus_radio
    {
        class PacketTrigger : public Trigger<Packet *>
        {
        public:
            explicit PacketTrigger(wmbus_radio::Radio *radio)
            {
                radio->add_packet_handler([this](Packet *packet)
                                          { this->trigger(packet); 
                                            return false; });
            }
        };
    }
}