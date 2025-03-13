#include "text_sensor.h"

namespace esphome
{
    namespace wmbus_meter
    {
        static const char *TAG = "wmbus_meter.text_sensor";

        void TextSensor::handle_update(Telegram *telegram, ::Meter *meter)
        {
            ESP_LOGI(TAG, "Getting value for %s", this->field_name.c_str());
            auto value = meter->getMyStringValue(this->field_name);

            if (value.empty())
            {
                ESP_LOGW(TAG, "Value for %s is empty", this->field_name.c_str());
                return;
            }
            this->publish_state(value);
        }
    } // namespace wmbus_meter
} // namespace esphome

