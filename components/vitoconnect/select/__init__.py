import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID, CONF_NAME, CONF_ADDRESS, CONF_MAP
from .. import vitoconnect_ns, VitoConnect, CONF_VITOCONNECT_ID

DEPENDENCIES = ["vitoconnect"]

CONF_BYTES = "bytes"

OPTOLINKSelect = vitoconnect_ns.class_("OPTOLINKSelect", select.Select)

def validate_mapping(value):
    """Validate the mapping format: "key -> value" """
    if not isinstance(value, list):
        raise cv.Invalid("Map must be a list of mappings")
    
    result = {}
    for item in value:
        if not isinstance(item, str):
            raise cv.Invalid("Each mapping must be a string")
        
        if " -> " not in item:
            raise cv.Invalid("Each mapping must be in format 'key -> value'")
        
        key_str, val_str = item.split(" -> ", 1)
        try:
            key = int(key_str.strip())
        except ValueError:
            raise cv.Invalid(f"Key '{key_str}' must be an integer")
        
        result[key] = val_str.strip()
    
    return result

CONFIG_SCHEMA = select.select_schema(OPTOLINKSelect).extend({
    cv.GenerateID(CONF_VITOCONNECT_ID): cv.use_id(VitoConnect),
    cv.Required(CONF_ADDRESS): cv.uint16_t,
    cv.Required(CONF_BYTES): cv.uint8_t,
    cv.Required(CONF_MAP): validate_mapping,
})

async def to_code(config):
    var = await select.new_select(config, options=list(config[CONF_MAP].values()))

    # Add configuration to datapoint
    cg.add(var.setAddress(config[CONF_ADDRESS]))
    cg.add(var.setLength(config[CONF_BYTES]))
    
    # Set up the value mapping
    for key, value in config[CONF_MAP].items():
        cg.add(var.add_mapping(key, value))

    # Add select to component hub (VitoConnect)
    hub = await cg.get_variable(config[CONF_VITOCONNECT_ID])
    cg.add(hub.register_datapoint(var)) 