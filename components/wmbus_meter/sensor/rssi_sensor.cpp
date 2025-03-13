#include "rssi_sensor.h"

namespace esphome
{
    namespace wmbus_meter
    {
        static const char *TAG = "wmbus_meter.rssi_sensor";

        void RSSISensor::handle_update(Telegram *telegram, ::Meter *meter)
        {
            ESP_LOGI(TAG, "Getting value for %s", this->field_name.c_str());
            auto value = telegram->about.rssi_dbm;

            if (std::isnan(value))
            {
                ESP_LOGW(TAG, "Value for %s is NaN", this->field_name.c_str());
                return;
            }
            this->publish_state(value);
        }
    }
}