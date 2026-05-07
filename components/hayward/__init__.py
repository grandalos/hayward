import esphome.codegen as cg
from esphome.components import binary_sensor, climate, number, sensor, switch, text_sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_ICON,
    CONF_UNIT_OF_MEASUREMENT,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_CURRENT_AC,
    ICON_FAN,
    ICON_FLASH,
    ICON_POWER,
    ICON_RADIATOR,
    ICON_RESTART_ALERT,
    ICON_THERMOMETER,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_REVOLUTIONS_PER_MINUTE,
    UNIT_VOLT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "button", "climate", "number", "sensor", "switch", "text_sensor"]

CONF_CTRL_CLIMATE = "ctrl_climate"
CONF_FRAME_TIMEOUT = "frame_timeout"
CONF_SEND_WRITES = "send_writes"
CONF_DIAG_POWER_STATE = "diag_power_state"
CONF_DIAG_MODE = "diag_mode"
CONF_DIAG_PANEL_CLOCK = "diag_panel_clock"
CONF_DIAG_SILENT_SCHEDULE_WINDOW = "diag_silent_schedule_window"
CONF_DIAG_POWER_SCHEDULE_WINDOW = "diag_power_schedule_window"
CONF_DIAG_POWER_ON_SCHEDULE_TIME = "diag_power_on_schedule_time"
CONF_DIAG_POWER_OFF_SCHEDULE_TIME = "diag_power_off_schedule_time"
CONF_DIAG_PANEL_UPDATE_FLAGS = "diag_panel_update_flags"
CONF_DIAG_TARGET_TEMPERATURE = "diag_target_temperature"
CONF_DIAG_SILENT_ACTIVE = "diag_silent_active"
CONF_DIAG_SILENT_SCHEDULE_ACTIVE = "diag_silent_schedule_active"
CONF_DIAG_SILENT_SCHEDULE_START_HOUR = "diag_silent_schedule_start_hour"
CONF_DIAG_SILENT_SCHEDULE_STOP_HOUR = "diag_silent_schedule_stop_hour"
CONF_DIAG_SUCTION_TEMPERATURE = "diag_suction_temperature"
CONF_DIAG_INLET_TEMPERATURE = "diag_inlet_temperature"
CONF_DIAG_OUTLET_TEMPERATURE = "diag_outlet_temperature"
CONF_DIAG_COIL_TEMPERATURE = "diag_coil_temperature"
CONF_DIAG_AMBIENT_TEMPERATURE = "diag_ambient_temperature"
CONF_DIAG_EXHAUST_TEMPERATURE = "diag_exhaust_temperature"
CONF_DIAG_COMPRESSOR_CURRENT = "diag_compressor_current"
CONF_DIAG_COMPRESSOR_OUTPUT_CURRENT = "diag_compressor_output_current"
CONF_DIAG_AC_FAN_OUTPUT = "diag_ac_fan_output"
CONF_DIAG_SUPER_HEAT = "diag_super_heat"
CONF_DIAG_TARGET_SPEED_FAN_MOTOR = "diag_target_speed_fan_motor"
CONF_DIAG_OVER_HEAT_AFTER_COMMPEN = "diag_over_heat_after_commpen"
CONF_DIAG_INVERTER_PLATE_AC_VOLTAGE = "diag_inverter_plate_ac_voltage"
CONF_DIAG_SPEED_FAN_MOTOR_1 = "diag_speed_fan_motor_1"
CONF_DIAG_PRESSURE_SENSOR = "diag_pressure_sensor"
CONF_DIAG_SWITCH_FLAGS = "diag_switch_flags"
CONF_DIAG_SWITCH_FLAGS_TEXT = "diag_switch_flags_text"
CONF_DIAG_COMPRESSOR_RUNNING = "diag_compressor_running"
CONF_DIAG_WATER_PUMP_ACTIVE = "diag_water_pump_active"
CONF_DIAG_FOUR_WAY_VALVE_ACTIVE = "diag_four_way_valve_active"
CONF_DIAG_FAN_HIGH_ACTIVE = "diag_fan_high_active"
CONF_DIAG_FAN_LOW_ACTIVE = "diag_fan_low_active"
CONF_DIAG_DEFROSTING = "diag_defrosting"
CONF_DIAG_FAILURE_FLAGS = "diag_failure_flags"
CONF_DIAG_PROTECTION_FLAGS = "diag_protection_flags"
CONF_DIAG_INVERTER_FAILURE_FLAGS = "diag_inverter_failure_flags"
CONF_DIAG_FAN_FAILURE_FLAGS = "diag_fan_failure_flags"
CONF_DIAG_ERROR_CODES = "diag_error_codes"
CONF_DIAG_ERROR_DESCRIPTIONS = "diag_error_descriptions"
CONF_DIAG_POWER_ON_SCHEDULE_ACTIVE = "diag_power_on_schedule_active"
CONF_DIAG_POWER_OFF_SCHEDULE_ACTIVE = "diag_power_off_schedule_active"
CONF_DIAG_ANTIFREEZE_TEMPERATURE = "diag_antifreeze_temperature"
CONF_DIAG_PANEL_STATUS_FLAGS = "diag_panel_status_flags"
CONF_DIAG_PANEL_HOUR = "diag_panel_hour"
CONF_DIAG_PANEL_MINUTE = "diag_panel_minute"
CONF_DIAG_PANEL_SECOND = "diag_panel_second"
CONF_DIAG_POWER_ON_HOUR = "diag_power_on_hour"
CONF_DIAG_POWER_OFF_HOUR = "diag_power_off_hour"
CONF_CTRL_SILENT_ACTIVE_SWITCH = "ctrl_silent_active_switch"
CONF_CTRL_SILENT_SCHEDULE_ACTIVE_SWITCH = "ctrl_silent_schedule_active_switch"
CONF_CTRL_POWER_ON_SCHEDULE_ACTIVE_SWITCH = "ctrl_power_on_schedule_active_switch"
CONF_CTRL_POWER_OFF_SCHEDULE_ACTIVE_SWITCH = "ctrl_power_off_schedule_active_switch"
CONF_CTRL_SILENT_SCHEDULE_START_HOUR_NUMBER = "ctrl_silent_schedule_start_hour_number"
CONF_CTRL_SILENT_SCHEDULE_STOP_HOUR_NUMBER = "ctrl_silent_schedule_stop_hour_number"
CONF_CTRL_POWER_ON_HOUR_NUMBER = "ctrl_power_on_hour_number"
CONF_CTRL_POWER_OFF_HOUR_NUMBER = "ctrl_power_off_hour_number"

hayward_ns = cg.esphome_ns.namespace("hayward")
Hayward = hayward_ns.class_("Hayward", cg.Component, uart.UARTDevice)
HaywardClimate = hayward_ns.class_("HaywardClimate", climate.Climate, cg.Component)
HaywardSwitch = hayward_ns.class_("HaywardSwitch", switch.Switch, cg.Component)
HaywardNumber = hayward_ns.class_("HaywardNumber", number.Number, cg.Component)
def diagnostic_temperature_schema(icon=ICON_THERMOMETER):
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        icon=icon,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


def diagnostic_raw_schema(icon):
    return sensor.sensor_schema(
        accuracy_decimals=0,
        icon=icon,
        state_class=STATE_CLASS_MEASUREMENT,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    )


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Hayward),
            cv.Optional(CONF_FRAME_TIMEOUT, default="15ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_SEND_WRITES, default=False): cv.boolean,
            cv.Optional(CONF_DIAG_POWER_STATE): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_POWER,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon=ICON_POWER,
            ),
            cv.Optional(CONF_DIAG_MODE): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon=ICON_RADIATOR,
            ),
            cv.Optional(CONF_CTRL_CLIMATE): climate.climate_schema(
                HaywardClimate,
                icon="mdi:pool-thermometer",
            ),
            cv.Optional(CONF_DIAG_PANEL_CLOCK): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:clock-digital",
            ),
            cv.Optional(CONF_DIAG_SILENT_SCHEDULE_WINDOW): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:calendar-clock",
            ),
            cv.Optional(CONF_DIAG_POWER_SCHEDULE_WINDOW): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:calendar-clock",
            ),
            cv.Optional(CONF_DIAG_POWER_ON_SCHEDULE_TIME): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:clock-start",
            ),
            cv.Optional(CONF_DIAG_POWER_OFF_SCHEDULE_TIME): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:clock-end",
            ),
            cv.Optional(CONF_DIAG_PANEL_UPDATE_FLAGS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:update",
            ),
            cv.Optional(CONF_DIAG_SWITCH_FLAGS_TEXT): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:connection",
            ),
            cv.Optional(CONF_DIAG_ERROR_CODES): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:alert-circle-outline",
            ),
            cv.Optional(CONF_DIAG_ERROR_DESCRIPTIONS): text_sensor.text_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:text-box-search-outline",
            ),
            cv.Optional(CONF_DIAG_TARGET_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                icon=ICON_THERMOMETER,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DIAG_SILENT_SCHEDULE_START_HOUR): diagnostic_raw_schema("mdi:clock-start"),
            cv.Optional(CONF_DIAG_SILENT_SCHEDULE_STOP_HOUR): diagnostic_raw_schema("mdi:clock-end"),
            cv.Optional(CONF_DIAG_SILENT_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:volume-off",
            ),
            cv.Optional(CONF_DIAG_SILENT_SCHEDULE_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:calendar-clock",
            ),
            cv.Optional(CONF_DIAG_COMPRESSOR_RUNNING): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:engine",
            ),
            cv.Optional(CONF_DIAG_WATER_PUMP_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:pump",
            ),
            cv.Optional(CONF_DIAG_FOUR_WAY_VALVE_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:swap-horizontal",
            ),
            cv.Optional(CONF_DIAG_FAN_HIGH_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:fan-speed-3",
            ),
            cv.Optional(CONF_DIAG_FAN_LOW_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:fan-speed-1",
            ),
            cv.Optional(CONF_DIAG_DEFROSTING): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:snowflake-melt",
            ),
            cv.Optional(CONF_DIAG_POWER_ON_SCHEDULE_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:power-on",
            ),
            cv.Optional(CONF_DIAG_POWER_OFF_SCHEDULE_ACTIVE): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                icon="mdi:power-off",
            ),
            cv.Optional(CONF_DIAG_SUCTION_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_INLET_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_OUTLET_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_COIL_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_AMBIENT_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_EXHAUST_TEMPERATURE): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_COMPRESSOR_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_CURRENT_AC,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_COMPRESSOR_OUTPUT_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                icon=ICON_CURRENT_AC,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_AC_FAN_OUTPUT): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_FAN,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_SUPER_HEAT): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_TARGET_SPEED_FAN_MOTOR): sensor.sensor_schema(
                unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
                icon=ICON_FAN,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_OVER_HEAT_AFTER_COMMPEN): diagnostic_temperature_schema(),
            cv.Optional(CONF_DIAG_INVERTER_PLATE_AC_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                icon=ICON_FLASH,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_SPEED_FAN_MOTOR_1): sensor.sensor_schema(
                unit_of_measurement=UNIT_REVOLUTIONS_PER_MINUTE,
                icon=ICON_FAN,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_PRESSURE_SENSOR): sensor.sensor_schema(
                unit_of_measurement="bar",
                icon="mdi:gauge",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
            cv.Optional(CONF_DIAG_SWITCH_FLAGS): diagnostic_raw_schema("mdi:connection"),
            cv.Optional(CONF_DIAG_FAILURE_FLAGS): diagnostic_raw_schema("mdi:alert-outline"),
            cv.Optional(CONF_DIAG_PROTECTION_FLAGS): diagnostic_raw_schema("mdi:shield-alert-outline"),
            cv.Optional(CONF_DIAG_INVERTER_FAILURE_FLAGS): diagnostic_raw_schema("mdi:lightning-bolt-outline"),
            cv.Optional(CONF_DIAG_FAN_FAILURE_FLAGS): diagnostic_raw_schema("mdi:fan-alert"),
            cv.Optional(CONF_DIAG_ANTIFREEZE_TEMPERATURE): diagnostic_temperature_schema(icon="mdi:snowflake-thermometer"),
            cv.Optional(CONF_DIAG_PANEL_STATUS_FLAGS): diagnostic_raw_schema("mdi:toggle-switch-outline"),
            cv.Optional(CONF_DIAG_PANEL_HOUR): diagnostic_raw_schema("mdi:clock-outline"),
            cv.Optional(CONF_DIAG_PANEL_MINUTE): diagnostic_raw_schema("mdi:clock-outline"),
            cv.Optional(CONF_DIAG_PANEL_SECOND): diagnostic_raw_schema("mdi:clock-outline"),
            cv.Optional(CONF_DIAG_POWER_ON_HOUR): diagnostic_raw_schema("mdi:clock-start"),
            cv.Optional(CONF_DIAG_POWER_OFF_HOUR): diagnostic_raw_schema("mdi:clock-end"),
            cv.Optional(CONF_CTRL_SILENT_ACTIVE_SWITCH): switch.switch_schema(
                HaywardSwitch,
                icon="mdi:volume-off",
            ),
            cv.Optional(CONF_CTRL_SILENT_SCHEDULE_ACTIVE_SWITCH): switch.switch_schema(
                HaywardSwitch,
                icon="mdi:calendar-clock",
            ),
            cv.Optional(CONF_CTRL_POWER_ON_SCHEDULE_ACTIVE_SWITCH): switch.switch_schema(
                HaywardSwitch,
                icon="mdi:power-on",
            ),
            cv.Optional(CONF_CTRL_POWER_OFF_SCHEDULE_ACTIVE_SWITCH): switch.switch_schema(
                HaywardSwitch,
                icon="mdi:power-off",
            ),
            cv.Optional(CONF_CTRL_SILENT_SCHEDULE_START_HOUR_NUMBER): number.number_schema(
                HaywardNumber,
                icon="mdi:clock-start",
            ),
            cv.Optional(CONF_CTRL_SILENT_SCHEDULE_STOP_HOUR_NUMBER): number.number_schema(
                HaywardNumber,
                icon="mdi:clock-end",
            ),
            cv.Optional(CONF_CTRL_POWER_ON_HOUR_NUMBER): number.number_schema(
                HaywardNumber,
                icon="mdi:clock-start",
            ),
            cv.Optional(CONF_CTRL_POWER_OFF_HOUR_NUMBER): number.number_schema(
                HaywardNumber,
                icon="mdi:clock-end",
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_frame_timeout_ms(config[CONF_FRAME_TIMEOUT].total_milliseconds))
    cg.add(var.set_send_writes(config[CONF_SEND_WRITES]))

    sensor_setters = {
        CONF_DIAG_TARGET_TEMPERATURE: "set_target_temperature_sensor",
        CONF_DIAG_SILENT_SCHEDULE_START_HOUR: "set_silent_schedule_start_hour_sensor",
        CONF_DIAG_SILENT_SCHEDULE_STOP_HOUR: "set_silent_schedule_stop_hour_sensor",
        CONF_DIAG_SUCTION_TEMPERATURE: "set_suction_temperature_sensor",
        CONF_DIAG_INLET_TEMPERATURE: "set_inlet_temperature_sensor",
        CONF_DIAG_OUTLET_TEMPERATURE: "set_outlet_temperature_sensor",
        CONF_DIAG_COIL_TEMPERATURE: "set_coil_temperature_sensor",
        CONF_DIAG_AMBIENT_TEMPERATURE: "set_ambient_temperature_sensor",
        CONF_DIAG_EXHAUST_TEMPERATURE: "set_exhaust_temperature_sensor",
        CONF_DIAG_COMPRESSOR_CURRENT: "set_compressor_current_sensor",
        CONF_DIAG_COMPRESSOR_OUTPUT_CURRENT: "set_compressor_output_current_sensor",
        CONF_DIAG_AC_FAN_OUTPUT: "set_ac_fan_output_sensor",
        CONF_DIAG_SUPER_HEAT: "set_super_heat_sensor",
        CONF_DIAG_TARGET_SPEED_FAN_MOTOR: "set_target_speed_fan_motor_sensor",
        CONF_DIAG_OVER_HEAT_AFTER_COMMPEN: "set_over_heat_after_commpen_sensor",
        CONF_DIAG_INVERTER_PLATE_AC_VOLTAGE: "set_inverter_plate_ac_voltage_sensor",
        CONF_DIAG_SPEED_FAN_MOTOR_1: "set_speed_fan_motor_1_sensor",
        CONF_DIAG_PRESSURE_SENSOR: "set_pressure_sensor",
        CONF_DIAG_SWITCH_FLAGS: "set_switch_flags_sensor",
        CONF_DIAG_FAILURE_FLAGS: "set_failure_flags_sensor",
        CONF_DIAG_PROTECTION_FLAGS: "set_protection_flags_sensor",
        CONF_DIAG_INVERTER_FAILURE_FLAGS: "set_inverter_failure_flags_sensor",
        CONF_DIAG_FAN_FAILURE_FLAGS: "set_fan_failure_flags_sensor",
        CONF_DIAG_ANTIFREEZE_TEMPERATURE: "set_antifreeze_temperature_sensor",
        CONF_DIAG_PANEL_STATUS_FLAGS: "set_panel_status_flags_sensor",
        CONF_DIAG_PANEL_HOUR: "set_panel_hour_sensor",
        CONF_DIAG_PANEL_MINUTE: "set_panel_minute_sensor",
        CONF_DIAG_PANEL_SECOND: "set_panel_second_sensor",
        CONF_DIAG_POWER_ON_HOUR: "set_power_on_hour_sensor",
        CONF_DIAG_POWER_OFF_HOUR: "set_power_off_hour_sensor",
    }
    for key, setter in sensor_setters.items():
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, setter)(sens))

    if CONF_DIAG_POWER_STATE in config:
        power_state = await binary_sensor.new_binary_sensor(config[CONF_DIAG_POWER_STATE])
        cg.add(var.set_power_state_binary_sensor(power_state))

    if CONF_DIAG_SILENT_ACTIVE in config:
        silent = await binary_sensor.new_binary_sensor(config[CONF_DIAG_SILENT_ACTIVE])
        cg.add(var.set_silent_active_binary_sensor(silent))

    if CONF_DIAG_SILENT_SCHEDULE_ACTIVE in config:
        silent_schedule = await binary_sensor.new_binary_sensor(config[CONF_DIAG_SILENT_SCHEDULE_ACTIVE])
        cg.add(var.set_silent_schedule_active_binary_sensor(silent_schedule))

    if CONF_DIAG_COMPRESSOR_RUNNING in config:
        running = await binary_sensor.new_binary_sensor(config[CONF_DIAG_COMPRESSOR_RUNNING])
        cg.add(var.set_compressor_running_binary_sensor(running))

    if CONF_DIAG_WATER_PUMP_ACTIVE in config:
        pump = await binary_sensor.new_binary_sensor(config[CONF_DIAG_WATER_PUMP_ACTIVE])
        cg.add(var.set_water_pump_active_binary_sensor(pump))

    if CONF_DIAG_FOUR_WAY_VALVE_ACTIVE in config:
        valve = await binary_sensor.new_binary_sensor(config[CONF_DIAG_FOUR_WAY_VALVE_ACTIVE])
        cg.add(var.set_four_way_valve_active_binary_sensor(valve))

    if CONF_DIAG_FAN_HIGH_ACTIVE in config:
        fan_high = await binary_sensor.new_binary_sensor(config[CONF_DIAG_FAN_HIGH_ACTIVE])
        cg.add(var.set_fan_high_active_binary_sensor(fan_high))

    if CONF_DIAG_FAN_LOW_ACTIVE in config:
        fan_low = await binary_sensor.new_binary_sensor(config[CONF_DIAG_FAN_LOW_ACTIVE])
        cg.add(var.set_fan_low_active_binary_sensor(fan_low))

    if CONF_DIAG_DEFROSTING in config:
        defrost = await binary_sensor.new_binary_sensor(config[CONF_DIAG_DEFROSTING])
        cg.add(var.set_defrosting_binary_sensor(defrost))

    if CONF_DIAG_POWER_ON_SCHEDULE_ACTIVE in config:
        power_on_schedule = await binary_sensor.new_binary_sensor(config[CONF_DIAG_POWER_ON_SCHEDULE_ACTIVE])
        cg.add(var.set_power_on_schedule_active_binary_sensor(power_on_schedule))

    if CONF_DIAG_POWER_OFF_SCHEDULE_ACTIVE in config:
        power_off_schedule = await binary_sensor.new_binary_sensor(config[CONF_DIAG_POWER_OFF_SCHEDULE_ACTIVE])
        cg.add(var.set_power_off_schedule_active_binary_sensor(power_off_schedule))

    switch_setters = {
        CONF_CTRL_SILENT_ACTIVE_SWITCH: ("set_silent_active_switch", 1076),
        CONF_CTRL_SILENT_SCHEDULE_ACTIVE_SWITCH: ("set_silent_schedule_active_switch", 1072),
        CONF_CTRL_POWER_ON_SCHEDULE_ACTIVE_SWITCH: ("set_power_on_schedule_active_switch", 1158),
        CONF_CTRL_POWER_OFF_SCHEDULE_ACTIVE_SWITCH: ("set_power_off_schedule_active_switch", 1159),
    }
    for key, (setter, register_address) in switch_setters.items():
        if key in config:
            sw = cg.new_Pvariable(config[key][CONF_ID])
            await cg.register_component(sw, config[key])
            await switch.register_switch(sw, config[key])
            cg.add(sw.set_register_address(register_address))
            cg.add(getattr(var, setter)(sw))

    number_setters = {
        CONF_CTRL_SILENT_SCHEDULE_START_HOUR_NUMBER: ("set_silent_schedule_start_hour_number", 1068),
        CONF_CTRL_SILENT_SCHEDULE_STOP_HOUR_NUMBER: ("set_silent_schedule_stop_hour_number", 1069),
        CONF_CTRL_POWER_ON_HOUR_NUMBER: ("set_power_on_hour_number", 1150),
        CONF_CTRL_POWER_OFF_HOUR_NUMBER: ("set_power_off_hour_number", 1152),
    }
    for key, (setter, register_address) in number_setters.items():
        if key in config:
            num = cg.new_Pvariable(config[key][CONF_ID])
            await cg.register_component(num, config[key])
            await number.register_number(num, config[key], min_value=0, max_value=23, step=1)
            cg.add(num.set_register_address(register_address))
            cg.add(getattr(var, setter)(num))

    if CONF_DIAG_MODE in config:
        mode = await text_sensor.new_text_sensor(config[CONF_DIAG_MODE])
        cg.add(var.set_mode_text_sensor(mode))

    if CONF_CTRL_CLIMATE in config:
        climate_var = cg.new_Pvariable(config[CONF_CTRL_CLIMATE][CONF_ID])
        await cg.register_component(climate_var, config[CONF_CTRL_CLIMATE])
        await climate.register_climate(climate_var, config[CONF_CTRL_CLIMATE])
        cg.add(var.set_hayward_climate(climate_var))

    if CONF_DIAG_PANEL_CLOCK in config:
        panel_clock = await text_sensor.new_text_sensor(config[CONF_DIAG_PANEL_CLOCK])
        cg.add(var.set_panel_clock_text_sensor(panel_clock))

    if CONF_DIAG_SILENT_SCHEDULE_WINDOW in config:
        silent_window = await text_sensor.new_text_sensor(config[CONF_DIAG_SILENT_SCHEDULE_WINDOW])
        cg.add(var.set_silent_schedule_window_text_sensor(silent_window))

    if CONF_DIAG_POWER_SCHEDULE_WINDOW in config:
        power_window = await text_sensor.new_text_sensor(config[CONF_DIAG_POWER_SCHEDULE_WINDOW])
        cg.add(var.set_power_schedule_window_text_sensor(power_window))

    if CONF_DIAG_POWER_ON_SCHEDULE_TIME in config:
        power_on_time = await text_sensor.new_text_sensor(config[CONF_DIAG_POWER_ON_SCHEDULE_TIME])
        cg.add(var.set_power_on_schedule_time_text_sensor(power_on_time))

    if CONF_DIAG_POWER_OFF_SCHEDULE_TIME in config:
        power_off_time = await text_sensor.new_text_sensor(config[CONF_DIAG_POWER_OFF_SCHEDULE_TIME])
        cg.add(var.set_power_off_schedule_time_text_sensor(power_off_time))

    if CONF_DIAG_PANEL_UPDATE_FLAGS in config:
        update_flags = await text_sensor.new_text_sensor(config[CONF_DIAG_PANEL_UPDATE_FLAGS])
        cg.add(var.set_panel_update_flags_text_sensor(update_flags))

    if CONF_DIAG_SWITCH_FLAGS_TEXT in config:
        switch_flags_text = await text_sensor.new_text_sensor(config[CONF_DIAG_SWITCH_FLAGS_TEXT])
        cg.add(var.set_switch_flags_text_sensor(switch_flags_text))

    if CONF_DIAG_ERROR_CODES in config:
        error_codes = await text_sensor.new_text_sensor(config[CONF_DIAG_ERROR_CODES])
        cg.add(var.set_error_codes_text_sensor(error_codes))

    if CONF_DIAG_ERROR_DESCRIPTIONS in config:
        error_descriptions = await text_sensor.new_text_sensor(config[CONF_DIAG_ERROR_DESCRIPTIONS])
        cg.add(var.set_error_descriptions_text_sensor(error_descriptions))
