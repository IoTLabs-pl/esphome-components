#include "sensor.h"

namespace esphome
{
    namespace wmbus_meter
    {
        static const char *TAG = "wmbus_meter.sensor";

        void Sensor::set_field_name(std::string field_name)
        {
            std::string name;
            extractUnit(field_name, &name, &(this->unit_));

            BaseSensor::set_field_name(name);

            if (this->get_unit_of_measurement().empty())
            {
                this->frontend_unit = unitToStringHR(this->unit_);
                this->set_unit_of_measurement(this->frontend_unit.c_str());
            }
        }

        void Sensor::handle_update(Telegram *telegram, ::Meter *meter)
        {
            ESP_LOGI(TAG, "Getting value for %s", this->field_name.c_str());
            auto value = meter->getNumericValue(this->field_name, this->unit_);

            if (std::isnan(value))
            {
                ESP_LOGW(TAG, "Value for %s is NaN", this->field_name.c_str());
                return;
            }
            this->publish_state(value);
        }
    }
}
