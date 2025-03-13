import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import spi
from esphome.const import (
    CONF_ID,
    CONF_RESET_PIN,
    CONF_IRQ_PIN,
)

CODEOWNERS = ["@SzczepanLeon", "@kubasaw"]

DEPENDENCIES = ["esp32", "spi"]

AUTO_LOAD = ["wmbus_common"]

CONF_RADIO_ID = "radio_id"

radio_ns = cg.esphome_ns.namespace("wmbus_radio")
RadioComponent = radio_ns.class_("Radio", cg.Component)
RadioTransceiver = radio_ns.class_("RadioTransceiver", spi.SPIDevice, cg.Component)


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(RadioComponent),
        cv.GenerateID(CONF_RADIO_ID): cv.declare_id(RadioTransceiver),
        cv.Required(CONF_RESET_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_IRQ_PIN): pins.internal_gpio_input_pin_schema,
    }
).extend(spi.spi_device_schema()).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    cg.add(cg.LineComment("WMBus RadioTransceiver"))

    RADIO = "SX1276"
    config[CONF_RADIO_ID].type = radio_ns.class_(RADIO, RadioTransceiver)
    radio_var = cg.new_Pvariable(config[CONF_RADIO_ID])

    reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
    cg.add(radio_var.set_reset_pin(reset_pin))

    irq_pin = await cg.gpio_pin_expression(config[CONF_IRQ_PIN])
    cg.add(radio_var.set_irq_pin(irq_pin))

    await spi.register_spi_device(radio_var, config)
    await cg.register_component(radio_var, config)

    cg.add(cg.LineComment("WMBus Component"))
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_radio(radio_var))

    await cg.register_component(var, config)
