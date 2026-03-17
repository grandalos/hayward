# ESPHome Hayward PC1002 External Component

External component for ESPHome that integrates Hayward heat pumps using the PC1002 controller over RS485.

## Goal

This repository avoids a custom ESPHome fork. The integration is implemented as an ESPHome `external_component` that works with current upstream ESPHome and runs on the AtomS3 Lite with the M5Stack RS485 base.

## Background

This implementation was built from:

- the local protocol documentation in [`docs`](./docs)
- live captures from a real Hayward installation with the PC1002 panel connected
- analysis of the original fork by cleontin: [github.com/cleontin/esphome](https://github.com/cleontin/esphome)
- the related write-up by Leontin: [leontin.eu/posts/hayward](https://www.leontin.eu/posts/hayward/)

The current code does not reuse the fork directly, but the control model intentionally follows the same protocol idea where it matters.

## Architecture

Main files:
- [`__init__.py`]: ESPHome config schema and entity wiring
- [`hayward.h`]: component classes and public setters
- [`hayward_protocol.h`]: protocol constants, register map, fault descriptors
- [`hayward.cpp`]: UART frame handling, cache, entity publishing, climate control logic


### Runtime model

The component has two roles:

- passive RS485 sniffer for frames already present on the bus
- emulated Modbus device on address `0x02` for controller-driven writes

It does not rely on direct `write single` commands to the heat pump for normal control. Instead:

1. Home Assistant changes a climate, switch, or number entity.
2. The component stages updated holding registers in its local cache.
3. The PC1002 panel reads `0x02`.
4. The component serves the staged register block.
5. The panel and pump propagate the change through the normal protocol flow.

This matches the behavior observed on the real bus and is the key reason the component works without patching ESPHome core.

## Installation

Use the component directly from GitHub:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/grandalos/hayward
      ref: main
    components: [hayward]
```

## Example configuration

Example based on the working AtomS3 Lite + M5Stack RS485 setup from this repository:

```yaml
uart:
  id: my_uart
  baud_rate: 9600
  tx_pin: GPIO06
  rx_pin: GPIO05
  stop_bits: 1
  parity: NONE
  rx_buffer_size: 512

hayward:
  id: hayward_bus
  uart_id: my_uart
  frame_timeout: 15ms
  send_writes: true
  ctrl_climate:
    id: hayward_climate
    name: "Climate"
  diag_power_state:
    name: "Power State"
  diag_mode:
    name: "Mode"
  diag_panel_clock:
    name: "Panel Clock"
  diag_panel_update_flags:
    name: "Panel Update Flags"
  diag_target_temperature:
    name: "Target Temperature"
  diag_silent_active:
    name: "Silent Active"
  diag_silent_schedule_active:
    name: "Silent Schedule Active"
  diag_compressor_running:
    name: "Compressor Running"
  diag_water_pump_active:
    name: "Water Pump Active"
  diag_four_way_valve_active:
    name: "Four-Way Valve Active"
  diag_fan_high_active:
    name: "Fan High Active"
  diag_fan_low_active:
    name: "Fan Low Active"
  diag_defrosting:
    name: "Defrosting"
  diag_power_on_schedule_active:
    name: "Power On Schedule Active"
  diag_power_off_schedule_active:
    name: "Power Off Schedule Active"
  diag_silent_schedule_window:
    name: "Silent Schedule Window"
  diag_power_schedule_window:
    name: "Power Schedule Window"
  diag_power_on_schedule_time:
    name: "Power On Schedule Hour"
    internal: true
  diag_power_off_schedule_time:
    name: "Power Off Schedule Hour"
    internal: true
  diag_silent_schedule_start_hour:
    name: "Silent On Hour"
    internal: true
  diag_silent_schedule_stop_hour:
    name: "Silent Off Hour"
    internal: true
  diag_panel_hour:
    name: "Panel Hour"
    internal: true
  diag_panel_minute:
    name: "Panel Minute"
    internal: true
  diag_panel_second:
    name: "Panel Second"
    internal: true
  diag_power_on_hour:
    name: "Power On Hour"
    internal: true
  diag_power_off_hour:
    name: "Power Off Hour"
    internal: true
  diag_suction_temperature:
    name: "Suction Temperature"
  diag_inlet_temperature:
    name: "Inlet Temperature"
  diag_outlet_temperature:
    name: "Outlet Temperature"
  diag_coil_temperature:
    name: "Coil Temperature"
  diag_ambient_temperature:
    name: "Ambient Temperature"
  diag_exhaust_temperature:
    name: "Exhaust Temperature"
  diag_compressor_current:
    name: "Compressor Current"
  diag_compressor_output_current:
    name: "Compressor Output Current"
  diag_ac_fan_output:
    name: "AC Fan Output"
  diag_super_heat:
    name: "Super Heat"
  diag_target_speed_fan_motor:
    name: "Fan Motor Target Speed"
  diag_over_heat_after_commpen:
    name: "Over Heat After Commpen"
  diag_inverter_plate_ac_voltage:
    name: "Inverter Plate AC Voltage"
  diag_speed_fan_motor_1:
    name: "Fan Motor 1 Speed"
  diag_pressure_sensor:
    name: "Pressure Sensor"
  diag_antifreeze_temperature:
    name: "Antifreeze Temperature"
  diag_switch_flags:
    name: "Switch Flags"
  diag_switch_flags_text:
    name: "Switch Flags Text"
  diag_failure_flags:
    name: "Failure Flags"
  diag_protection_flags:
    name: "Protection Flags"
  diag_inverter_failure_flags:
    name: "Inverter Failure Flags"
  diag_fan_failure_flags:
    name: "Fan Failure Flags"
  diag_error_codes:
    name: "Error Codes"
  diag_error_descriptions:
    name: "Error Descriptions"
  diag_panel_status_flags:
    name: "Panel Status Flags"
  ctrl_silent_active_switch:
    name: "Silent Mode"
  ctrl_silent_schedule_active_switch:
    name: "Silent Schedule Switch"
  ctrl_power_on_schedule_active_switch:
    name: "Power On Schedule Switch"
  ctrl_power_off_schedule_active_switch:
    name: "Power Off Schedule Switch"
  ctrl_silent_schedule_start_hour_number:
    name: "Silent On Hour"
  ctrl_silent_schedule_stop_hour_number:
    name: "Silent Off Hour"
  ctrl_power_on_hour_number:
    name: "Power On Schedule Hour"
  ctrl_power_off_hour_number:
    name: "Power Off Schedule Hour"
```

## Implemented features

Confirmed working on the target installation:

- read-only telemetry from the pump and panel
- `climate` entity
- HVAC mode changes
- target temperature changes
- silent preset
- power and silent schedules
- schedule activation switches
- persistence after restart
- decoded switch flags from `2034`
- decoded fault aggregation from `2074` to `2077`

## Climate state sources

The climate entity is assembled from cached registers:

- current temperature: `2046`
- target temperature: `1013` with fallback to `1135`, `1136`, `1137`
- power state: `2011` with fallback to staged control registers
- mode: `2012` with fallback to staged `1012`
- preset: `1076`
- action: inferred mainly from `2019` and water temperatures

## Logging

Runtime logging is tuned for normal operation:

- actual delivery of staged settings to PC1002 remains visible at `INFO`
- raw frame traffic and cache churn were moved to `VERBOSE`

## Current limitations

- fault decoding is mapped from documentation, but not every alarm has yet been observed live
- some registers from the full PC1002 manual are still not exposed as entities
- panel clock is read correctly from `3015` to `3017`, but writing a real panel time is not currently supported
- the implementation is intentionally tailored to the observed Hayward PC1002 behavior rather than every possible OEM variant
