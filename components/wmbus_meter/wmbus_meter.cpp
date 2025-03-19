#include "wmbus_meter.h"

namespace esphome
{
    namespace wmbus_meter
    {
        static const char *TAG = "wmbus_meter";

        void Meter::set_meter_params(std::string id, std::string driver, std::string key)
        {
            MeterInfo meter_info;
            meter_info.parse(driver + '-' + id, driver, id + ",", key);

            this->meter = createMeter(&meter_info);
        }
        void Meter::set_radio(wmbus_radio::Radio *radio)
        {
            this->radio = radio;
            radio->add_packet_handler([this](wmbus_radio::Packet *packet)
                                      { return this->handle_packet(packet); });
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

        bool Meter::handle_packet(wmbus_radio::Packet *packet)
        {
            auto about = AboutTelegram(App.get_friendly_name(), packet->rssi, FrameType::WMBUS, this->rtc->timestamp_now());

            std::vector<Address> adresses;
            bool id_match;
            auto telegram = std::make_unique<Telegram>();

            if (this->meter->handleTelegram(about, packet->data, false, &adresses, &id_match, telegram.get()))
            {
                this->last_telegram = std::move(telegram);
                this->defer([this]()
                            { this->on_telegram_callback_manager(); });
                return true;
            }

            return false;
        }

        std::string Meter::as_json(bool pretty_print)
        {
            std::string json;
            this->meter->printMeter(this->last_telegram.get(), nullptr, nullptr, '\t', &json, nullptr, nullptr, nullptr, pretty_print);
            return json;
        }

        optional<std::string> Meter::get_string_field(std::string field_name)
        {
            auto field_info = this->meter->findFieldInfo(field_name, Quantity::Text);
            if (field_info)
                return this->meter->getStringValue(field_info);

            return {};
        }

        optional<float> Meter::get_numeric_field(std::string field_name)
        {
            // RSSI is not handled by meter but by telegram :/
            if (field_name == "rssi_dbm")
                return this->last_telegram->about.rssi_dbm;

            std::string name;
            Unit unit;
            extractUnit(field_name, &name, &unit);

            auto value = this->meter->getNumericValue(name, unit);

            if (!std::isnan(value))
                return value;

            return {};
        }

        void Meter::on_telegram(std::function<void()> &&callback)
        {
            this->on_telegram_callback_manager.add(std::move(callback));
        }

    }
}
