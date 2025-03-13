#pragma once
#include "esphome/core/component.h"
#include "esphome/components/wmbus_common/Telegram.h"

#include "wmbus_meter.h"

namespace esphome
{
    namespace wmbus_meter
    {
        class BaseSensor : public Parented<Meter>, public Component
        {
        public:
            void set_field_name(std::string field_name);
            virtual void handle_update(Telegram *telegram, ::Meter *meter) = 0;
            void set_parent(Meter *parent);
            void dump_config() override;

        protected:
            Meter *meter;
            std::string field_name;
        };
    }
}