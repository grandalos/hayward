#pragma once

#include <cstdint>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/climate/climate.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"

namespace esphome::hayward {

class Hayward;

class HaywardSwitch;
class HaywardNumber;

class HaywardClimate : public climate::Climate, public Component {
 public:
  void set_parent(Hayward *parent) { this->parent_ = parent; }
  void control(const climate::ClimateCall &call) override;
  climate::ClimateTraits traits() override;

 protected:
  Hayward *parent_{nullptr};
};

class HaywardSwitch : public switch_::Switch, public Component {
 public:
  void set_parent(Hayward *parent) { this->parent_ = parent; }
  void set_register_address(uint16_t register_address) { this->register_address_ = register_address; }

 protected:
  void write_state(bool state) override;

  Hayward *parent_{nullptr};
  uint16_t register_address_{0};
};

class HaywardNumber : public number::Number, public Component {
 public:
  void set_parent(Hayward *parent) { this->parent_ = parent; }
  void set_register_address(uint16_t register_address) { this->register_address_ = register_address; }

 protected:
  void control(float value) override;

  Hayward *parent_{nullptr};
  uint16_t register_address_{0};
};

class Hayward : public Component, public uart::UARTDevice {
  friend class HaywardClimate;
  friend class HaywardSwitch;
  friend class HaywardNumber;

 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_frame_timeout_ms(uint32_t frame_timeout_ms) { this->frame_timeout_ms_ = frame_timeout_ms; }
  void set_send_writes(bool send_writes) { this->send_writes_ = send_writes; }

  void set_power_state_binary_sensor(binary_sensor::BinarySensor *sensor) { this->power_state_binary_sensor_ = sensor; }
  void set_silent_active_binary_sensor(binary_sensor::BinarySensor *sensor) { this->silent_active_binary_sensor_ = sensor; }
  void set_silent_schedule_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->silent_schedule_active_binary_sensor_ = sensor;
  }
  void set_silent_schedule_start_hour_sensor(sensor::Sensor *sensor) { this->silent_schedule_start_hour_sensor_ = sensor; }
  void set_silent_schedule_stop_hour_sensor(sensor::Sensor *sensor) { this->silent_schedule_stop_hour_sensor_ = sensor; }
  void set_compressor_running_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->compressor_running_binary_sensor_ = sensor;
  }
  void set_water_pump_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->water_pump_active_binary_sensor_ = sensor;
  }
  void set_four_way_valve_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->four_way_valve_active_binary_sensor_ = sensor;
  }
  void set_fan_high_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->fan_high_active_binary_sensor_ = sensor;
  }
  void set_fan_low_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->fan_low_active_binary_sensor_ = sensor;
  }
  void set_defrosting_binary_sensor(binary_sensor::BinarySensor *sensor) { this->defrosting_binary_sensor_ = sensor; }
  void set_power_on_schedule_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->power_on_schedule_active_binary_sensor_ = sensor;
  }
  void set_power_off_schedule_active_binary_sensor(binary_sensor::BinarySensor *sensor) {
    this->power_off_schedule_active_binary_sensor_ = sensor;
  }
  void set_mode_text_sensor(text_sensor::TextSensor *sensor) { this->mode_text_sensor_ = sensor; }
  void set_panel_clock_text_sensor(text_sensor::TextSensor *sensor) { this->panel_clock_text_sensor_ = sensor; }
  void set_switch_flags_text_sensor(text_sensor::TextSensor *sensor) { this->switch_flags_text_sensor_ = sensor; }
  void set_silent_schedule_window_text_sensor(text_sensor::TextSensor *sensor) {
    this->silent_schedule_window_text_sensor_ = sensor;
  }
  void set_power_schedule_window_text_sensor(text_sensor::TextSensor *sensor) {
    this->power_schedule_window_text_sensor_ = sensor;
  }
  void set_power_on_schedule_time_text_sensor(text_sensor::TextSensor *sensor) {
    this->power_on_schedule_time_text_sensor_ = sensor;
  }
  void set_power_off_schedule_time_text_sensor(text_sensor::TextSensor *sensor) {
    this->power_off_schedule_time_text_sensor_ = sensor;
  }
  void set_panel_update_flags_text_sensor(text_sensor::TextSensor *sensor) {
    this->panel_update_flags_text_sensor_ = sensor;
  }
  void set_error_codes_text_sensor(text_sensor::TextSensor *sensor) { this->error_codes_text_sensor_ = sensor; }
  void set_error_descriptions_text_sensor(text_sensor::TextSensor *sensor) {
    this->error_descriptions_text_sensor_ = sensor;
  }
  void set_hayward_climate(HaywardClimate *climate) {
    this->hayward_climate_ = climate;
    if (this->hayward_climate_ != nullptr) {
      this->hayward_climate_->set_parent(this);
    }
  }
  void set_silent_active_switch(HaywardSwitch *sw) {
    this->silent_active_switch_ = sw;
    if (this->silent_active_switch_ != nullptr) {
      this->silent_active_switch_->set_parent(this);
    }
  }
  void set_silent_schedule_active_switch(HaywardSwitch *sw) {
    this->silent_schedule_active_switch_ = sw;
    if (this->silent_schedule_active_switch_ != nullptr) {
      this->silent_schedule_active_switch_->set_parent(this);
    }
  }
  void set_power_on_schedule_active_switch(HaywardSwitch *sw) {
    this->power_on_schedule_active_switch_ = sw;
    if (this->power_on_schedule_active_switch_ != nullptr) {
      this->power_on_schedule_active_switch_->set_parent(this);
    }
  }
  void set_power_off_schedule_active_switch(HaywardSwitch *sw) {
    this->power_off_schedule_active_switch_ = sw;
    if (this->power_off_schedule_active_switch_ != nullptr) {
      this->power_off_schedule_active_switch_->set_parent(this);
    }
  }
  void set_silent_schedule_start_hour_number(HaywardNumber *num) {
    this->silent_schedule_start_hour_number_ = num;
    if (this->silent_schedule_start_hour_number_ != nullptr) {
      this->silent_schedule_start_hour_number_->set_parent(this);
    }
  }
  void set_silent_schedule_stop_hour_number(HaywardNumber *num) {
    this->silent_schedule_stop_hour_number_ = num;
    if (this->silent_schedule_stop_hour_number_ != nullptr) {
      this->silent_schedule_stop_hour_number_->set_parent(this);
    }
  }
  void set_power_on_hour_number(HaywardNumber *num) {
    this->power_on_hour_number_ = num;
    if (this->power_on_hour_number_ != nullptr) {
      this->power_on_hour_number_->set_parent(this);
    }
  }
  void set_power_off_hour_number(HaywardNumber *num) {
    this->power_off_hour_number_ = num;
    if (this->power_off_hour_number_ != nullptr) {
      this->power_off_hour_number_->set_parent(this);
    }
  }

  void set_target_temperature_sensor(sensor::Sensor *sensor) { this->target_temperature_sensor_ = sensor; }
  void set_power_sensor(sensor::Sensor *sensor) { this->power_sensor_ = sensor; }
  void set_suction_temperature_sensor(sensor::Sensor *sensor) { this->suction_temperature_sensor_ = sensor; }
  void set_inlet_temperature_sensor(sensor::Sensor *sensor) { this->inlet_temperature_sensor_ = sensor; }
  void set_outlet_temperature_sensor(sensor::Sensor *sensor) { this->outlet_temperature_sensor_ = sensor; }
  void set_coil_temperature_sensor(sensor::Sensor *sensor) { this->coil_temperature_sensor_ = sensor; }
  void set_ambient_temperature_sensor(sensor::Sensor *sensor) { this->ambient_temperature_sensor_ = sensor; }
  void set_exhaust_temperature_sensor(sensor::Sensor *sensor) { this->exhaust_temperature_sensor_ = sensor; }
  void set_compressor_current_sensor(sensor::Sensor *sensor) { this->compressor_current_sensor_ = sensor; }
  void set_compressor_output_current_sensor(sensor::Sensor *sensor) { this->compressor_output_current_sensor_ = sensor; }
  void set_ac_fan_output_sensor(sensor::Sensor *sensor) { this->ac_fan_output_sensor_ = sensor; }
  void set_super_heat_sensor(sensor::Sensor *sensor) { this->super_heat_sensor_ = sensor; }
  void set_target_speed_fan_motor_sensor(sensor::Sensor *sensor) { this->target_speed_fan_motor_sensor_ = sensor; }
  void set_over_heat_after_commpen_sensor(sensor::Sensor *sensor) { this->over_heat_after_commpen_sensor_ = sensor; }
  void set_inverter_plate_ac_voltage_sensor(sensor::Sensor *sensor) { this->inverter_plate_ac_voltage_sensor_ = sensor; }
  void set_speed_fan_motor_1_sensor(sensor::Sensor *sensor) { this->speed_fan_motor_1_sensor_ = sensor; }
  void set_pressure_sensor(sensor::Sensor *sensor) { this->pressure_sensor_ = sensor; }
  void set_switch_flags_sensor(sensor::Sensor *sensor) { this->switch_flags_sensor_ = sensor; }
  void set_failure_flags_sensor(sensor::Sensor *sensor) { this->failure_flags_sensor_ = sensor; }
  void set_protection_flags_sensor(sensor::Sensor *sensor) { this->protection_flags_sensor_ = sensor; }
  void set_inverter_failure_flags_sensor(sensor::Sensor *sensor) { this->inverter_failure_flags_sensor_ = sensor; }
  void set_fan_failure_flags_sensor(sensor::Sensor *sensor) { this->fan_failure_flags_sensor_ = sensor; }
  void set_antifreeze_temperature_sensor(sensor::Sensor *sensor) { this->antifreeze_temperature_sensor_ = sensor; }
  void set_panel_status_flags_sensor(sensor::Sensor *sensor) { this->panel_status_flags_sensor_ = sensor; }
  void set_panel_hour_sensor(sensor::Sensor *sensor) { this->panel_hour_sensor_ = sensor; }
  void set_panel_minute_sensor(sensor::Sensor *sensor) { this->panel_minute_sensor_ = sensor; }
  void set_panel_second_sensor(sensor::Sensor *sensor) { this->panel_second_sensor_ = sensor; }
  void set_power_on_hour_sensor(sensor::Sensor *sensor) { this->power_on_hour_sensor_ = sensor; }
  void set_power_off_hour_sensor(sensor::Sensor *sensor) { this->power_off_hour_sensor_ = sensor; }

 protected:
  struct PendingRead {
    uint8_t address;
    uint8_t function_code;
    uint16_t start_address;
    uint16_t register_count;
    uint32_t timestamp_ms;
  };

  bool try_parse_buffer_();
  optional<size_t> detect_frame_length_() const;
  bool validate_crc_(const uint8_t *data, size_t len) const;
  uint16_t crc16_(const uint8_t *data, size_t len) const;

  void handle_frame_(const std::vector<uint8_t> &frame);
  void handle_read_request_(const std::vector<uint8_t> &frame);
  void handle_read_response_(const std::vector<uint8_t> &frame);
  void handle_write_single_request_(const std::vector<uint8_t> &frame);
  void handle_write_multiple_request_(const std::vector<uint8_t> &frame);
  std::string format_frame_(const std::vector<uint8_t> &frame) const;

  void clear_stale_pending_reads_();
  void update_register_cache_(uint16_t address, uint16_t value);
  optional<uint16_t> get_register_(uint16_t address) const;
  void publish_entities_();
  void publish_climate_();
  bool stage_register_write_(uint16_t address, uint16_t value, const char *reason);
  bool send_emulated_read_response_(uint8_t target_address, uint8_t function_code, uint16_t start_address,
                                    const std::vector<uint16_t> &values);
  bool maybe_respond_to_controller_read_(const PendingRead &request);
  std::vector<uint16_t> build_register_block_(uint16_t start_address, uint16_t register_count);
  uint16_t build_status_register_(uint16_t address);
  void update_block_tracking_(uint16_t address);
  void update_clock_seed_();
  bool apply_climate_mode_(climate::ClimateMode mode);
  bool apply_target_temperature_(float target_temperature);
  bool apply_climate_preset_(const optional<climate::ClimatePreset> &preset);

  void publish_temperature_sensor_(sensor::Sensor *sensor, uint16_t address);
  void publish_scaled_sensor_(sensor::Sensor *sensor, uint16_t address, float scale);
  void publish_power_sensor_(sensor::Sensor *sensor);
  void publish_bcd_sensor_(sensor::Sensor *sensor, uint16_t address);
  void publish_binary_sensor_(binary_sensor::BinarySensor *sensor, bool value);
  void publish_text_sensor_(text_sensor::TextSensor *sensor, const std::string &value);
  void publish_switch_(switch_::Switch *sw, bool value);
  void publish_number_(number::Number *num, float value);
  void publish_toggle_entities_(uint16_t address, binary_sensor::BinarySensor *binary_sensor, switch_::Switch *sw);
  void publish_hour_value_(uint16_t address, sensor::Sensor *sensor, number::Number *number_entity,
                           text_sensor::TextSensor *time_sensor);
  void publish_schedule_window_(uint16_t start_address, uint16_t stop_address, text_sensor::TextSensor *window_sensor);
  void publish_mode_text_();
  uint16_t decode_bcd_(uint16_t value) const;
  optional<uint16_t> get_decoded_bcd_register_(uint16_t address) const;
  std::string format_hour_(uint16_t hour) const;

  std::vector<uint8_t> rx_buffer_;
  std::unordered_map<uint16_t, uint16_t> registers_;
  std::unordered_map<uint8_t, PendingRead> pending_reads_;

  uint32_t last_byte_ms_{0};
  uint32_t frame_timeout_ms_{15};
  uint8_t controller_address_{0x02};
  uint8_t broadcast_address_{0x00};
  bool send_writes_{false};
  bool has_settings_snapshot_{false};
  bool has_extra_settings_snapshot_{false};
  bool has_status_snapshot_{false};
  bool pending_settings_update_{false};
  bool pending_extra_settings_update_{false};
  uint32_t settings_applying_until_ms_{0};
  uint32_t extra_settings_applying_until_ms_{0};
  uint32_t clock_seed_timestamp_ms_{0};
  uint16_t clock_seed_hour_bcd_{0};
  uint16_t clock_seed_minute_bcd_{0};
  uint16_t clock_seed_second_bcd_{0};

  binary_sensor::BinarySensor *power_state_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *silent_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *silent_schedule_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *compressor_running_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *water_pump_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *four_way_valve_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fan_high_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *fan_low_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *defrosting_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *power_on_schedule_active_binary_sensor_{nullptr};
  binary_sensor::BinarySensor *power_off_schedule_active_binary_sensor_{nullptr};
  text_sensor::TextSensor *mode_text_sensor_{nullptr};
  text_sensor::TextSensor *panel_clock_text_sensor_{nullptr};
  text_sensor::TextSensor *switch_flags_text_sensor_{nullptr};
  text_sensor::TextSensor *silent_schedule_window_text_sensor_{nullptr};
  text_sensor::TextSensor *power_schedule_window_text_sensor_{nullptr};
  text_sensor::TextSensor *power_on_schedule_time_text_sensor_{nullptr};
  text_sensor::TextSensor *power_off_schedule_time_text_sensor_{nullptr};
  text_sensor::TextSensor *panel_update_flags_text_sensor_{nullptr};
  text_sensor::TextSensor *error_codes_text_sensor_{nullptr};
  text_sensor::TextSensor *error_descriptions_text_sensor_{nullptr};
  HaywardClimate *hayward_climate_{nullptr};
  HaywardSwitch *silent_active_switch_{nullptr};
  HaywardSwitch *silent_schedule_active_switch_{nullptr};
  HaywardSwitch *power_on_schedule_active_switch_{nullptr};
  HaywardSwitch *power_off_schedule_active_switch_{nullptr};
  HaywardNumber *silent_schedule_start_hour_number_{nullptr};
  HaywardNumber *silent_schedule_stop_hour_number_{nullptr};
  HaywardNumber *power_on_hour_number_{nullptr};
  HaywardNumber *power_off_hour_number_{nullptr};

  sensor::Sensor *target_temperature_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *silent_schedule_start_hour_sensor_{nullptr};
  sensor::Sensor *silent_schedule_stop_hour_sensor_{nullptr};
  sensor::Sensor *suction_temperature_sensor_{nullptr};
  sensor::Sensor *inlet_temperature_sensor_{nullptr};
  sensor::Sensor *outlet_temperature_sensor_{nullptr};
  sensor::Sensor *coil_temperature_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *exhaust_temperature_sensor_{nullptr};
  sensor::Sensor *compressor_current_sensor_{nullptr};
  sensor::Sensor *compressor_output_current_sensor_{nullptr};
  sensor::Sensor *ac_fan_output_sensor_{nullptr};
  sensor::Sensor *super_heat_sensor_{nullptr};
  sensor::Sensor *target_speed_fan_motor_sensor_{nullptr};
  sensor::Sensor *over_heat_after_commpen_sensor_{nullptr};
  sensor::Sensor *inverter_plate_ac_voltage_sensor_{nullptr};
  sensor::Sensor *speed_fan_motor_1_sensor_{nullptr};
  sensor::Sensor *pressure_sensor_{nullptr};
  sensor::Sensor *switch_flags_sensor_{nullptr};
  sensor::Sensor *failure_flags_sensor_{nullptr};
  sensor::Sensor *protection_flags_sensor_{nullptr};
  sensor::Sensor *inverter_failure_flags_sensor_{nullptr};
  sensor::Sensor *fan_failure_flags_sensor_{nullptr};
  sensor::Sensor *antifreeze_temperature_sensor_{nullptr};
  sensor::Sensor *panel_status_flags_sensor_{nullptr};
  sensor::Sensor *panel_hour_sensor_{nullptr};
  sensor::Sensor *panel_minute_sensor_{nullptr};
  sensor::Sensor *panel_second_sensor_{nullptr};
  sensor::Sensor *power_on_hour_sensor_{nullptr};
  sensor::Sensor *power_off_hour_sensor_{nullptr};
};

}  // namespace esphome::hayward
