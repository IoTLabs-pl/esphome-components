#pragma once
#include "esphome/components/sensor/sensor.h"

#include "../base_sensor.h"

namespace esphome
{
    namespace wmbus_meter
    {
        class Sensor : public sensor::Sensor, public BaseSensor
        {
        public:
            void set_field_name(std::string field_name);
            void handle_update(Telegram *telegram, ::Meter *meter);

        protected:
            Unit unit_;
            std::string frontend_unit;
        };
    }
}