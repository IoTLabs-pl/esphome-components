import esphome.config_validation as cv
from esphome.components import sensor

from .. import wmbus_meter_ns
from ..base_sensor import BASE_SCHEMA, CONF_FIELD, register_meter, BaseSensor


RegularSensor = wmbus_meter_ns.class_("Sensor", BaseSensor, sensor.Sensor)
RSSISensor = wmbus_meter_ns.class_("RSSISensor", BaseSensor, sensor.Sensor)


RegularSensorSchema = BASE_SCHEMA.extend(sensor.sensor_schema(RegularSensor))

RSSISensorSchema = BASE_SCHEMA.extend({cv.Required(CONF_FIELD): "rssi"}).extend(
    sensor.sensor_schema(
        RSSISensor,
        device_class=sensor.DEVICE_CLASS_SIGNAL_STRENGTH,
    )
)

CONFIG_SCHEMA = cv.Any(RSSISensorSchema, RegularSensorSchema)


async def to_code(config):
    sensor_ = await sensor.new_sensor(config)
    await register_meter(sensor_, config)
