#include "wmbus_meter.h"

namespace esphome
{
    namespace wmbus_meter
    {
        static const char *TAG = "wmbus_meter";

        void Meter::set_meter_params(std::string id, std::string driver, std::string key)
        {
            MeterInfo meter_info;
            meter_info.parse("ESPHome", driver, id + ",", key);

            this->meter = createMeter(&meter_info);
        }
        void Meter::set_radio(wmbus_radio::Radio *radio)
        {
            this->radio = radio;
            radio->add_on_packet_callback([this](wmbus_radio::Packet *packet)
                                          { this->handle_packet(packet); });
        }
        void Meter::set_rtc(time::RealTimeClock *rtc) { this->rtc = rtc; }

        void Meter::dump_config()
        {
            std::vector<AddressExpression> address_expressions = this->meter->addressExpressions();
            std::string id = address_expressions.size() > 0 ? address_expressions[0].id : "unknown";
            std::string driver = this->meter->driverName().str();
            MeterKeys *keys = this->meter->meterKeys();
            std::string key = keys->hasConfidentialityKey() ? format_hex(keys->confidentiality_key) : "not-encrypted";

            ESP_LOGCONFIG(TAG, "wM-Bus Meter:");
            ESP_LOGCONFIG(TAG, "  ID: %s", id.c_str());
            ESP_LOGCONFIG(TAG, "  Driver: %s", driver.c_str());
            ESP_LOGCONFIG(TAG, "  Key: %s", key.c_str());
        }

        void Meter::handle_packet(wmbus_radio::Packet *packet)
        {
            auto about = AboutTelegram("ESPHome wM-Bus", packet->rssi, FrameType::WMBUS, this->rtc->timestamp_now());

            std::vector<Address> adresses;
            bool id_match;
            Telegram telegram;

            if (this->meter->handleTelegram(about, packet->data, false, &adresses, &id_match, &telegram))
                ESP_LOGD("wmbus_meter", "Telegram handled successfully");
            else
                ESP_LOGI("wmbus_meter", "Telegram not handled");
        }

        void Meter::on_update(std::function<void(Telegram *, ::Meter *)> callback)
        {
            this->meter->onUpdate(callback);
        }

    }
}
