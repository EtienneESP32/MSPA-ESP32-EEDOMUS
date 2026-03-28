import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart

mspa_ns = cg.esphome_ns.namespace("mspa")
MSPAUartComponent = mspa_ns.class_("MSPAUartComponent", cg.Component)

DEPENDENCIES = ["uart"]

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MSPAUartComponent),
    cv.Required("uart_spa"): cv.use_id(uart.UARTComponent),
    cv.Required("uart_kbd"): cv.use_id(uart.UARTComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    spa = await cg.get_variable(config["uart_spa"])
    kbd = await cg.get_variable(config["uart_kbd"])
    var = cg.new_Pvariable(config[CONF_ID], spa, kbd)
    await cg.register_component(var, config)
