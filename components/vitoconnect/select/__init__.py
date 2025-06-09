import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID, CONF_NAME, CONF_ADDRESS, CONF_FROM, CONF_TO
from .. import vitoconnect_ns, VitoConnect, CONF_VITOCONNECT_ID

DEPENDENCIES = ["vitoconnect"]

CONF_BYTES = "bytes"
CONF_MAP = "map"

OPTOLINKSelect = vitoconnect_ns.class_("OPTOLINKSelect", select.Select)

def validate_mapping(value):
    """Validate the mapping format: "key -> value" """
    if not isinstance(value, dict):
        value = cv.string(value)
        if " -> " not in value:
            raise cv.Invalid("Mapping must contain ' -> '")
        a, b = value.split(" -> ", 1)
        value = {CONF_FROM: a.strip(), CONF_TO: b.strip()}

    return cv.Schema(
        {cv.Required(CONF_FROM): cv.string, cv.Required(CONF_TO): cv.string}
    )(value)

CONFIG_SCHEMA = select.select_schema(OPTOLINKSelect).extend({
    cv.GenerateID(CONF_VITOCONNECT_ID): cv.use_id(VitoConnect),
    cv.Required(CONF_ADDRESS): cv.uint16_t,
    cv.Required(CONF_BYTES): cv.uint8_t,
    cv.Required(CONF_MAP): cv.ensure_list(validate_mapping),
})

async def to_code(config):
    # Extract options from the mapping
    options = [item[CONF_TO] for item in config[CONF_MAP]]
    var = await select.new_select(config, options=options)

    # Add configuration to datapoint
    cg.add(var.setAddress(config[CONF_ADDRESS]))
    cg.add(var.setLength(config[CONF_BYTES]))
    
    # Set up the value mapping using the add_mapping method
    for item in config[CONF_MAP]:
        try:
            key = int(item[CONF_FROM])
            value = item[CONF_TO]
            cg.add(var.add_mapping(key, value))
        except ValueError:
            raise cv.Invalid(f"Key '{item[CONF_FROM]}' must be an integer")

    # Add select to component hub (VitoConnect)
    hub = await cg.get_variable(config[CONF_VITOCONNECT_ID])
    cg.add(hub.register_datapoint(var)) 