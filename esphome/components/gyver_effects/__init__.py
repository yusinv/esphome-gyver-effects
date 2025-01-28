import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.types import AddressableLightEffect
from esphome.components.light.effects import register_addressable_effect
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    CONF_SPEED,
    CONF_HEIGHT,
    CONF_WIDTH,
    CONF_PIXEL_MAPPER,
    CONF_COLOR_PALETTE,
    CONF_UPDATE_INTERVAL,
    CONF_TYPE
)

CONF_SCALE = "scale"
CONF_AMOUNT = "amount"
CONF_FROM_CENTER = "from_center"
CONF_FADE_FRAMES = "fade_frames"
CONF_GEN_SPEED = "generation_speed"



gyver_effects_ns = cg.esphome_ns.namespace("gyver_effects")
BaseGyverLightEffect = gyver_effects_ns.class_("BaseGyverLightEffect", AddressableLightEffect)
FireLightEffect = gyver_effects_ns.class_("FireLightEffect", BaseGyverLightEffect)
PerlinLightEffect = gyver_effects_ns.class_("PerlinLightEffect", BaseGyverLightEffect)
ConfettiLightEffect = gyver_effects_ns.class_("ConfettiLightEffect", BaseGyverLightEffect)
GradientLightEffect = gyver_effects_ns.class_("GradientLightEffect", BaseGyverLightEffect)
ParticlesLightEffect = gyver_effects_ns.class_("ParticlesLightEffect", BaseGyverLightEffect)
ConfettiType = ConfettiLightEffect.enum("ConfettiType",True)
GyverLightSettings = gyver_effects_ns.class_("GyverLightSettings")

MAPPIING_CONFETTI_TYPE = {
    "random": ConfettiType.Random,
    "palette": ConfettiType.Palette,
    "select": ConfettiType.Select
}


MAPPIING_PALETTES = {
    "sunset" : gyver_effects_ns.namespace("PALETTE_SUNSET_REAL"),
    "blurred" : gyver_effects_ns.namespace("PALETTE_DK_BLURRED"),
    "optimus_prime" : gyver_effects_ns.namespace("PALETTE_OPTIMUS_PRIME"),
    "warm" : gyver_effects_ns.namespace("PALETTE_WARM"),
    "cold" : gyver_effects_ns.namespace("PALETTE_COLD"),
    "hot" : gyver_effects_ns.namespace("PALETTE_HOT"),
    "pink" : gyver_effects_ns.namespace("PALETTE_PINK"),
    "comfy" : gyver_effects_ns.namespace("PALETTE_COMFY"),
    "cyberpunk" : gyver_effects_ns.namespace("PALETTE_CYBERPUNK"),
    "girl" : gyver_effects_ns.namespace("PALETTE_GIRL"),
    "xmas" : gyver_effects_ns.namespace("PALETTE_XMAS"),
    "acid" : gyver_effects_ns.namespace("PALETTE_ACID"),
    "blue_smoke" : gyver_effects_ns.namespace("PALETTE_BLUE_SMOKE"),
    "gummy" : gyver_effects_ns.namespace("PALETTE_GUMMY"),
    "leo" : gyver_effects_ns.namespace("PALETTE_LEO"),
    "aurora" : gyver_effects_ns.namespace("PALETTE_AURORA")
    # custom
}

CONFIG_SCHEMA = cv.All(cv.Schema({
    cv.GenerateID(CONF_ID): cv.declare_id(GyverLightSettings),
    cv.Required(CONF_WIDTH): cv.positive_int,
    cv.Required(CONF_HEIGHT): cv.positive_int,
    cv.Required(CONF_PIXEL_MAPPER): cv.returning_lambda,
}).extend(cv.COMPONENT_SCHEMA))


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    pixel_mapper = config[CONF_PIXEL_MAPPER]
    global pixel_mapper_template_
    pixel_mapper_template_ = await cg.process_lambda(
        pixel_mapper,
        [(int, "x"), (int, "y")],
        return_type=cg.int_,
    )
    cg.add(var.set_width(config[CONF_WIDTH]))
    cg.add(var.set_height(config[CONF_HEIGHT]))
    cg.add(var.set_pixel_mapper(pixel_mapper_template_))

BASE_EFFECT_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_ID): cv.use_id(GyverLightSettings),
})


@register_addressable_effect(
    "fire",
    FireLightEffect,
    "Fire",
    BASE_EFFECT_SCHEMA.extend(cv.Schema({
        cv.Optional(CONF_SPEED, default=10): cv.uint8_t
    })).schema,
)
async def fire_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_settings(await cg.get_variable(config[CONF_ID])))
    return var

@register_addressable_effect(
    "perlin",
    PerlinLightEffect,
    "Perlin",
    BASE_EFFECT_SCHEMA.extend(cv.Schema({
        cv.Optional(CONF_SPEED, default=10): cv.uint8_t,
        cv.Optional(CONF_SCALE, default=100): cv.uint8_t,
        cv.Optional(CONF_COLOR_PALETTE, default="sunset"): cv.enum(MAPPIING_PALETTES),
    })).schema,
)
async def perlin_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_scale(config[CONF_SCALE]))
    cg.add(var.set_palette(config[CONF_COLOR_PALETTE]))
    cg.add(var.set_settings(await cg.get_variable(config[CONF_ID])))
    return var

@register_addressable_effect(
    "confetti",
    ConfettiLightEffect,
    "Confetti",
    BASE_EFFECT_SCHEMA.extend(cv.Schema({
        cv.Optional(CONF_FADE_FRAMES, default=30): cv.uint8_t,
        cv.Optional(CONF_GEN_SPEED, default=5.0): cv.float_range(0,255),
        cv.Optional(CONF_COLOR_PALETTE): cv.enum(MAPPIING_PALETTES),
        cv.Optional(CONF_UPDATE_INTERVAL, default='16ms'): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_TYPE,default="random"): cv.enum(MAPPIING_CONFETTI_TYPE)
        #todo palette is required if type == palette
    })).schema,
)
async def confetti_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_fade_frames(config[CONF_FADE_FRAMES]))
    cg.add(var.set_gen_speed(config[CONF_GEN_SPEED]))
    if palette := config.get(CONF_COLOR_PALETTE):
        cg.add(var.set_palette(palette))
    cg.add(var.set_type(config[CONF_TYPE]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(var.set_settings(await cg.get_variable(config[CONF_ID])))
    return var

@register_addressable_effect(
    "gradient",
    GradientLightEffect,
    "Gradient",
    BASE_EFFECT_SCHEMA.extend(cv.Schema({
        cv.Optional(CONF_SPEED, default=10): cv.uint8_t,
        cv.Optional(CONF_AMOUNT, default=100): cv.uint8_t,
        cv.Optional(CONF_COLOR_PALETTE, default="sunset"): cv.enum(MAPPIING_PALETTES),
        cv.Optional(CONF_UPDATE_INTERVAL, default='16ms'): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_FROM_CENTER,default=False): cv.boolean,
        cv.Optional(CONF_SCALE, default=127): cv.uint8_t
    })).schema,
)
async def gradient_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_from_center(config[CONF_FROM_CENTER]))
    cg.add(var.set_palette(config[CONF_COLOR_PALETTE]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(var.set_settings(await cg.get_variable(config[CONF_ID])))
    return var

@register_addressable_effect(
    "particles",
    ParticlesLightEffect,
    "Particles",
    BASE_EFFECT_SCHEMA.extend(cv.Schema({
        cv.Optional(CONF_SPEED, default=10): cv.uint8_t,
        cv.Optional(CONF_AMOUNT, default=100): cv.uint8_t,
        cv.Optional(CONF_COLOR_PALETTE, default="sunset"): cv.enum(MAPPIING_PALETTES),
        cv.Optional(CONF_UPDATE_INTERVAL, default='16ms'): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_FROM_CENTER,default=False): cv.boolean,
        cv.Optional(CONF_SCALE, default=127): cv.uint8_t
    })).schema,
)
async def gradient_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_speed(config[CONF_SPEED]))
    cg.add(var.set_from_center(config[CONF_FROM_CENTER]))
    cg.add(var.set_palette(config[CONF_COLOR_PALETTE]))
    cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))
    cg.add(var.set_settings(await cg.get_variable(config[CONF_ID])))
    return var