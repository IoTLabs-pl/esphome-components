#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../base_sensor.h"

namespace esphome
{
    namespace wmbus_meter
    {
        class RSSISensor : public sensor::Sensor,
                           public BaseSensor
        {
        public:
            void handle_update(Telegram *telegram, ::Meter *meter) override;
        };
    }
}