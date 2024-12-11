import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID, CONF_PIN


hunter_roam_ns = cg.esphome_ns.namespace('hunter_roam')
HunterRoam = hunter_roam_ns.class_('HunterRoam', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HunterRoam),
    cv.Required(CONF_PIN): pins.gpio_output_pin_schema,
})

def to_code(config):
    # Create a new instance of the HunterRoam class and pass the pin to it
    
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_pin(config[CONF_PIN]['number']))

    yield cg.register_component(var, config)