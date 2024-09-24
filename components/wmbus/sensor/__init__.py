import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.log import Fore, color
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    CONF_KEY,
    CONF_NAME,
    UNIT_DECIBEL_MILLIWATT,
    DEVICE_CLASS_SIGNAL_STRENGTH,
    STATE_CLASS_MEASUREMENT,
    UNIT_CUBIC_METER,
    DEVICE_CLASS_WATER,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_KILOWATT_HOURS,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    STATE_CLASS_TOTAL,
    UNIT_KILOWATT,
    UNIT_EMPTY,
    DEVICE_CLASS_EMPTY,
    UNIT_CELSIUS,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_GAS,
    UNIT_VOLT,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    UNIT_SECOND,
    DEVICE_CLASS_TIMESTAMP,
    CONF_UNIT_OF_MEASUREMENT,
)

AUTO_LOAD = ["wmbus"]

CONF_METER_ID = "meter_id"
CONF_LISTENER_ID = "listener_id"
CONF_WMBUS_ID = "wmbus_id"
CONF_FIELD = "field"
CONF_SENSORS = 'sensors'

from .. import (
    WMBusComponent,
    wmbus_ns
)

CODEOWNERS = ["@SzczepanLeon"]

WMBusListener = wmbus_ns.class_('WMBusListener')

def my_key(value):
    value = cv.string_strict(value)
    parts = [value[i : i + 2] for i in range(0, len(value), 2)]
    if (len(parts) != 16) and (len(parts) != 0):
        raise cv.Invalid("Key must consist of 16 hexadecimal numbers")
    parts_int = []
    if any(len(part) != 2 for part in parts):
        raise cv.Invalid("Key must be format XX")
    for part in parts:
        try:
            parts_int.append(int(part, 16))
        except ValueError:
            # pylint: disable=raise-missing-from
            raise cv.Invalid("Key must be hex values from 00 to FF")
    return "".join(f"{part:02X}" for part in parts_int)


SENSOR_SCHEMA = sensor.sensor_schema(
    #
).extend(
    {
        cv.Optional(CONF_FIELD, default=""): cv.string_strict,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_LISTENER_ID): cv.declare_id(WMBusListener),
        cv.GenerateID(CONF_WMBUS_ID): cv.use_id(WMBusComponent),
        cv.Optional(CONF_METER_ID, default=0): cv.hex_int,
        cv.Optional(CONF_TYPE, default=""): cv.string_strict,
        cv.Optional(CONF_KEY, default=""): my_key,
        cv.Optional(CONF_SENSORS): cv.ensure_list(SENSOR_SCHEMA),
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    if config[CONF_TYPE]:
        cg.add_platformio_option("build_src_filter", [f"+<**/wmbus/driver_{config[CONF_TYPE].lower()}.cpp>"])
    var = cg.new_Pvariable(config[CONF_LISTENER_ID],
                           config[CONF_METER_ID],
                           config[CONF_TYPE].lower(),
                           config[CONF_KEY])
    if config[CONF_METER_ID]:
        for s in config.get(CONF_SENSORS, []):
            if CONF_UNIT_OF_MEASUREMENT not in s:
                print(color(Fore.RED, f"unit_of_measurement not defined for sensor '{s[CONF_NAME]}'!"))
                exit()
            if (s[CONF_FIELD]):
                field = s[CONF_FIELD].lower()
            else:
                field = s[CONF_NAME].lower()
            unit = s[CONF_UNIT_OF_MEASUREMENT]
            sens = await sensor.new_sensor(s)
            cg.add(var.add_sensor(field, unit, sens))
        wmbus = await cg.get_variable(config[CONF_WMBUS_ID])
        cg.add(wmbus.register_wmbus_listener(var))
