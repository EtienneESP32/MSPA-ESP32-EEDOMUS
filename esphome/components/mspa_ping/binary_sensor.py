import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID, CONF_IP_ADDRESS

# Namespace declaration
mspa_ping_ns = cg.esphome_ns.namespace("mspa_ping")
MspaPingComponent = mspa_ping_ns.class_(
    "MspaPingComponent", cg.PollingComponent, binary_sensor.BinarySensor
)

CONFIG_SCHEMA = (
    binary_sensor.binary_sensor_schema(MspaPingComponent)
    .extend(
        {
            cv.Required(CONF_IP_ADDRESS): cv.string,
        }
    )
    .extend(cv.polling_component_schema("60s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)
    cg.add(var.set_target_ip(config[CONF_IP_ADDRESS]))
