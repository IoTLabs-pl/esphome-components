#pragma once
#include "esphome/core/component.h"

#include "esphome/components/time/real_time_clock.h"

#include "esphome/components/wmbus_radio/component.h"
#include "esphome/components/wmbus_common/meters.h"

namespace esphome
{
    namespace wmbus_meter
    {
        class Meter : public Component
        {
        public:
            void set_meter_params(std::string id, std::string driver, std::string key);
            void set_radio(wmbus_radio::Radio *radio);
            void set_rtc(time::RealTimeClock *rtc);

            void dump_config() override;

            void on_update(std::function<void(Telegram *, ::Meter *)> callback);

        protected:
            std::string key;

            time::RealTimeClock *rtc;

            wmbus_radio::Radio *radio;

            std::shared_ptr<::Meter> meter;

            void handle_packet(wmbus_radio::Packet *packet);
        };
    }
}