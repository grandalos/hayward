#include "hayward.h"
#include "hayward_protocol.h"

#include <cmath>
#include <sstream>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome::hayward {

using namespace esphome::hayward::protocol;

static const char *const TAG = "hayward";

static std::string join_strings_(const std::vector<std::string> &parts) {
  if (parts.empty()) {
    
    return "none";
  }
  std::ostringstream out;
  for (size_t i = 0; i < parts.size(); i++) {
    if (i != 0U) {
      out << ", ";
    }
    out << parts[i];
  }
  return out.str();
}

static std::string decode_panel_status_flags_(uint16_t flags) {
  if (flags == 0U) {
    return "idle";
  }

  std::vector<std::string> states;
  if ((flags & FLAG_SETTINGS_APPLYING) != 0U) {
    states.emplace_back("settings_applying");
  }
  if ((flags & FLAG_SETTINGS_UPDATE) != 0U) {
    states.emplace_back("settings_pending");
  }
  if ((flags & FLAG_EXTRA_SETTINGS_UPDATE) != 0U) {
    states.emplace_back("extra_pending");
  }
  if ((flags & FLAG_NEEDS_UPDATES) != 0U) {
    states.emplace_back("needs_updates");
  }

  const uint16_t known_mask =
      FLAG_SETTINGS_APPLYING | FLAG_SETTINGS_UPDATE | FLAG_EXTRA_SETTINGS_UPDATE | FLAG_NEEDS_UPDATES;
  const uint16_t unknown_bits = flags & static_cast<uint16_t>(~known_mask);
  if (unknown_bits != 0U || states.empty()) {
    char raw[16];
    snprintf(raw, sizeof(raw), "raw_0x%04X", flags);
    states.emplace_back(raw);
  }

  std::ostringstream out;
  for (size_t i = 0; i < states.size(); i++) {
    if (i != 0U) {
      out << "+";
    }
    out << states[i];
  }
  return out.str();
}

static std::string decode_switch_flags_(uint16_t flags) {
  std::vector<std::string> active;
  for (const auto &[mask, label] : SWITCH_FLAG_DESCRIPTORS) {
    if ((flags & mask) != 0U) {
      active.emplace_back(label);
    }
  }
  return join_strings_(active);
}

static void collect_faults_(const std::unordered_map<uint16_t, uint16_t> &registers, std::vector<std::string> *codes,
                            std::vector<std::string> *descriptions) {
  for (const auto &descriptor : FAULT_DESCRIPTORS) {
    auto reg = registers.find(descriptor.address);
    if (reg == registers.end()) {
      continue;
    }
    if ((reg->second & descriptor.mask) == 0U) {
      continue;
    }
    codes->emplace_back(descriptor.code);
    descriptions->emplace_back(descriptor.description);
  }
}

void HaywardSwitch::write_state(bool state) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Switch write requested without parent");
    return;
  }
  if (this->parent_->stage_register_write_(this->register_address_, state ? 1U : 0U, "switch control")) {
    this->publish_state(state);
    this->parent_->publish_entities_();
  }
}

void HaywardNumber::control(float value) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Number control requested without parent");
    return;
  }

  const uint16_t register_value = static_cast<uint16_t>(std::lround(value));
  if (this->parent_->stage_register_write_(this->register_address_, register_value, "number control")) {
    this->publish_state(static_cast<float>(register_value));
    this->parent_->publish_entities_();
  }
}

void HaywardClimate::control(const climate::ClimateCall &call) {
  if (this->parent_ == nullptr) {
    ESP_LOGW(TAG, "Climate control requested without parent");
    return;
  }

  bool changed = false;

  if (call.get_mode().has_value()) {
    changed |= this->parent_->apply_climate_mode_(*call.get_mode());
  }

  if (call.get_target_temperature().has_value()) {
    changed |= this->parent_->apply_target_temperature_(*call.get_target_temperature());
  }

  if (call.get_preset().has_value()) {
    changed |= this->parent_->apply_climate_preset_(call.get_preset());
  }

  if (changed) {
    this->parent_->publish_entities_();
  }
}

climate::ClimateTraits HaywardClimate::traits() {
  climate::ClimateTraits traits;
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  traits.set_supported_modes({
      climate::CLIMATE_MODE_OFF,
      climate::CLIMATE_MODE_HEAT,
      climate::CLIMATE_MODE_COOL,
      climate::CLIMATE_MODE_AUTO,
  });
  traits.set_supported_presets({
      climate::CLIMATE_PRESET_NONE,
      climate::CLIMATE_PRESET_ECO,
  });
  traits.set_visual_min_temperature(15.0f);
  traits.set_visual_max_temperature(35.0f);
  traits.set_visual_temperature_step(0.5f);
  return traits;
}

void Hayward::setup() {
  this->check_uart_settings(9600, 1, uart::UART_CONFIG_PARITY_NONE, 8);
  this->set_rx_full_threshold(8);
}

void Hayward::loop() {
  const uint32_t now = millis();

  while (this->available() > 0) {
    uint8_t byte;
    if (!this->read_byte(&byte)) {
      break;
    }
    this->rx_buffer_.push_back(byte);
    this->last_byte_ms_ = now;

    while (this->try_parse_buffer_()) {
    }
  }

  if (!this->rx_buffer_.empty() && now - this->last_byte_ms_ > this->frame_timeout_ms_) {
    ESP_LOGV(TAG, "Dropping partial frame with %zu bytes after timeout", this->rx_buffer_.size());
    this->rx_buffer_.clear();
  }

  this->clear_stale_pending_reads_();
}

void Hayward::dump_config() {
  ESP_LOGCONFIG(TAG, "Hayward PC1002 sniffer:");
  ESP_LOGCONFIG(TAG, "  Frame timeout: %u ms", this->frame_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Controller address: 0x%02X (fixed)", this->controller_address_);
  ESP_LOGCONFIG(TAG, "  Broadcast address: 0x%02X (fixed)", this->broadcast_address_);
  ESP_LOGCONFIG(TAG, "  Send writes: %s", YESNO(this->send_writes_));
  LOG_BINARY_SENSOR("  ", "Power State", this->power_state_binary_sensor_);
  LOG_TEXT_SENSOR("  ", "Mode", this->mode_text_sensor_);
  LOG_SENSOR("  ", "Target Temperature", this->target_temperature_sensor_);
  LOG_SENSOR("  ", "Silent Schedule Start Hour", this->silent_schedule_start_hour_sensor_);
  LOG_SENSOR("  ", "Silent Schedule Stop Hour", this->silent_schedule_stop_hour_sensor_);
  LOG_BINARY_SENSOR("  ", "Silent Active", this->silent_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Silent Schedule Active", this->silent_schedule_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Compressor Running", this->compressor_running_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Water Pump Active", this->water_pump_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Four-Way Valve Active", this->four_way_valve_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Fan High Active", this->fan_high_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Fan Low Active", this->fan_low_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Defrosting", this->defrosting_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Power On Schedule Active", this->power_on_schedule_active_binary_sensor_);
  LOG_BINARY_SENSOR("  ", "Power Off Schedule Active", this->power_off_schedule_active_binary_sensor_);
  LOG_SENSOR("  ", "Suction Temperature", this->suction_temperature_sensor_);
  LOG_SENSOR("  ", "Inlet Temperature", this->inlet_temperature_sensor_);
  LOG_SENSOR("  ", "Outlet Temperature", this->outlet_temperature_sensor_);
  LOG_SENSOR("  ", "Coil Temperature", this->coil_temperature_sensor_);
  LOG_SENSOR("  ", "Ambient Temperature", this->ambient_temperature_sensor_);
  LOG_SENSOR("  ", "Exhaust Temperature", this->exhaust_temperature_sensor_);
  LOG_SENSOR("  ", "Compressor Current", this->compressor_current_sensor_);
  LOG_SENSOR("  ", "Compressor Output Current", this->compressor_output_current_sensor_);
  LOG_SENSOR("  ", "AC Fan Output", this->ac_fan_output_sensor_);
  LOG_SENSOR("  ", "Super Heat", this->super_heat_sensor_);
  LOG_SENSOR("  ", "Target Speed Fan Motor", this->target_speed_fan_motor_sensor_);
  LOG_SENSOR("  ", "Over Heat After Commpen", this->over_heat_after_commpen_sensor_);
  LOG_SENSOR("  ", "Inverter Plate AC Voltage", this->inverter_plate_ac_voltage_sensor_);
  LOG_SENSOR("  ", "Speed Fan Motor 1", this->speed_fan_motor_1_sensor_);
  LOG_SENSOR("  ", "Pressure Sensor", this->pressure_sensor_);
  LOG_SENSOR("  ", "Switch Flags", this->switch_flags_sensor_);
  LOG_SENSOR("  ", "Failure Flags", this->failure_flags_sensor_);
  LOG_SENSOR("  ", "Protection Flags", this->protection_flags_sensor_);
  LOG_SENSOR("  ", "Inverter Failure Flags", this->inverter_failure_flags_sensor_);
  LOG_SENSOR("  ", "Fan Failure Flags", this->fan_failure_flags_sensor_);
  LOG_SENSOR("  ", "Antifreeze Temperature", this->antifreeze_temperature_sensor_);
  LOG_SENSOR("  ", "Panel Status Flags", this->panel_status_flags_sensor_);
  LOG_SENSOR("  ", "Panel Hour", this->panel_hour_sensor_);
  LOG_SENSOR("  ", "Panel Minute", this->panel_minute_sensor_);
  LOG_SENSOR("  ", "Panel Second", this->panel_second_sensor_);
  LOG_SENSOR("  ", "Power On Hour", this->power_on_hour_sensor_);
  LOG_SENSOR("  ", "Power Off Hour", this->power_off_hour_sensor_);
  LOG_TEXT_SENSOR("  ", "Panel Clock", this->panel_clock_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Switch Flags Text", this->switch_flags_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Silent Schedule Window", this->silent_schedule_window_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Power Schedule Window", this->power_schedule_window_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Power On Schedule Time", this->power_on_schedule_time_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Power Off Schedule Time", this->power_off_schedule_time_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Panel Update Flags", this->panel_update_flags_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Error Codes", this->error_codes_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Error Descriptions", this->error_descriptions_text_sensor_);
}

float Hayward::get_setup_priority() const { return setup_priority::BUS - 0.5f; }

bool Hayward::stage_register_write_(uint16_t address, uint16_t value, const char *reason) {
  const bool changed = !this->get_register_(address).has_value() || *this->get_register_(address) != value;
  this->update_register_cache_(address, value);
  if (address >= SETTINGS_START && address <= SETTINGS_END) {
    this->pending_settings_update_ = true;
  } else if (address >= EXTRA_SETTINGS_START && address <= EXTRA_SETTINGS_END) {
    this->pending_extra_settings_update_ = true;
  }

  if (!this->send_writes_) {
    ESP_LOGD(TAG, "Staged write only: reg=%u value=0x%04X reason=%s", address, value, reason);
  } else {
    ESP_LOGD(TAG, "Staged emulated controller write: reg=%u value=0x%04X reason=%s", address, value, reason);
  }
  return changed;
}

bool Hayward::send_emulated_read_response_(uint8_t target_address, uint8_t function_code, uint16_t start_address,
                                           const std::vector<uint16_t> &values) {
  std::vector<uint8_t> frame;
  frame.reserve(5 + values.size() * 2);
  frame.push_back(target_address);
  frame.push_back(function_code);
  frame.push_back(static_cast<uint8_t>(values.size() * 2));
  for (const uint16_t value : values) {
    frame.push_back(static_cast<uint8_t>(value >> 8));
    frame.push_back(static_cast<uint8_t>(value & 0xFF));
  }
  const uint16_t crc = this->crc16_(frame.data(), frame.size());
  frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  frame.push_back(static_cast<uint8_t>(crc >> 8));
  this->write_array(frame);
  this->flush();
  ESP_LOGD(TAG, "Served emulated read: addr=0x%02X fc=0x%02X start=%u count=%zu", target_address, function_code,
           start_address, values.size());
  return true;
}

bool Hayward::apply_climate_mode_(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_OFF:
      return this->stage_register_write_(REG_POWER_COMMAND, 0U, "climate mode off") |
             this->stage_register_write_(REG_POWER_COMMAND_2, 0U, "climate mode off");
    case climate::CLIMATE_MODE_COOL: {
      bool changed = false;
      if (auto saved = this->get_register_(REG_SAVED_COOL_TEMPERATURE); saved.has_value()) {
        changed |= this->stage_register_write_(REG_TARGET_TEMPERATURE, *saved, "restore cool target");
      }
      changed |= this->stage_register_write_(REG_MODE_COMMAND, 0U, "climate mode cool");
      changed |= this->stage_register_write_(REG_POWER_COMMAND, 1U, "climate power on");
      changed |= this->stage_register_write_(REG_POWER_COMMAND_2, 1U, "climate power on");
      return changed;
    }
    case climate::CLIMATE_MODE_HEAT: {
      bool changed = false;
      if (auto saved = this->get_register_(REG_SAVED_HEAT_TEMPERATURE); saved.has_value()) {
        changed |= this->stage_register_write_(REG_TARGET_TEMPERATURE, *saved, "restore heat target");
      }
      changed |= this->stage_register_write_(REG_MODE_COMMAND, 1U, "climate mode heat");
      changed |= this->stage_register_write_(REG_POWER_COMMAND, 1U, "climate power on");
      changed |= this->stage_register_write_(REG_POWER_COMMAND_2, 1U, "climate power on");
      return changed;
    }
    case climate::CLIMATE_MODE_AUTO: {
      bool changed = false;
      if (auto saved = this->get_register_(REG_SAVED_AUTO_TEMPERATURE); saved.has_value()) {
        changed |= this->stage_register_write_(REG_TARGET_TEMPERATURE, *saved, "restore auto target");
      }
      changed |= this->stage_register_write_(REG_MODE_COMMAND, 2U, "climate mode auto");
      changed |= this->stage_register_write_(REG_POWER_COMMAND, 1U, "climate power on");
      changed |= this->stage_register_write_(REG_POWER_COMMAND_2, 1U, "climate power on");
      return changed;
    }
    default:
      ESP_LOGW(TAG, "Unsupported climate mode request: %d", mode);
      return false;
  }
}

bool Hayward::apply_target_temperature_(float target_temperature) {
  const uint16_t raw_target = static_cast<uint16_t>(std::lround(target_temperature * 10.0f));
  bool changed = this->stage_register_write_(REG_TARGET_TEMPERATURE, raw_target, "target temperature");

  climate::ClimateMode active_mode = climate::CLIMATE_MODE_OFF;
  if (this->hayward_climate_ != nullptr) {
    active_mode = this->hayward_climate_->mode;
  }

  switch (active_mode) {
    case climate::CLIMATE_MODE_COOL:
      changed |= this->stage_register_write_(REG_SAVED_COOL_TEMPERATURE, raw_target, "save cool target");
      break;
    case climate::CLIMATE_MODE_HEAT:
      changed |= this->stage_register_write_(REG_SAVED_HEAT_TEMPERATURE, raw_target, "save heat target");
      break;
    case climate::CLIMATE_MODE_AUTO:
      changed |= this->stage_register_write_(REG_SAVED_AUTO_TEMPERATURE, raw_target, "save auto target");
      break;
    default:
      break;
  }
  return changed;
}

bool Hayward::apply_climate_preset_(const optional<climate::ClimatePreset> &preset) {
  if (!preset.has_value()) {
    return false;
  }

  switch (*preset) {
    case climate::CLIMATE_PRESET_NONE:
      return this->stage_register_write_(REG_SILENT_ACTIVE, 0U, "preset none");
    case climate::CLIMATE_PRESET_ECO:
      return this->stage_register_write_(REG_SILENT_ACTIVE, 1U, "preset eco");
    default:
      ESP_LOGW(TAG, "Unsupported climate preset request: %d", *preset);
      return false;
  }
}

bool Hayward::try_parse_buffer_() {
  if (this->rx_buffer_.size() < 5) {
    return false;
  }

  auto detected_length = this->detect_frame_length_();
  if (!detected_length.has_value()) {
    return false;
  }

  const size_t frame_len = *detected_length;
  if (this->rx_buffer_.size() < frame_len) {
    return false;
  }

  if (!this->validate_crc_(this->rx_buffer_.data(), frame_len)) {
    ESP_LOGW(TAG, "CRC mismatch while decoding frame, dropping one byte to resync");
    this->rx_buffer_.erase(this->rx_buffer_.begin());
    return true;
  }

  std::vector<uint8_t> frame(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);
  this->rx_buffer_.erase(this->rx_buffer_.begin(), this->rx_buffer_.begin() + frame_len);
  this->handle_frame_(frame);
  return !this->rx_buffer_.empty();
}

optional<size_t> Hayward::detect_frame_length_() const {
  if (this->rx_buffer_.size() < 2) {
    return {};
  }

  const auto &buf = this->rx_buffer_;
  const uint8_t address = buf[0];
  const uint8_t function_code = buf[1];

  auto try_len = [&](size_t len) -> optional<size_t> {
    if (buf.size() >= len && this->validate_crc_(buf.data(), len)) {
      return len;
    }
    return {};
  };

  if ((function_code & 0x80U) != 0U) {
    return try_len(5);
  }

  switch (function_code) {
    case FC_READ_HOLDING:
    case FC_READ_INPUT: {
      auto pending = this->pending_reads_.find(address);
      if (pending != this->pending_reads_.end()) {
        const size_t response_len = 5 + pending->second.register_count * 2;
        if (auto len = try_len(response_len); len.has_value()) {
          return len;
        }
      }

      if (auto len = try_len(8); len.has_value()) {
        return len;
      }

      if (buf.size() >= 3) {
        const size_t response_len = 5 + buf[2];
        if (auto len = try_len(response_len); len.has_value()) {
          return len;
        }
      }
      return {};
    }
    case FC_WRITE_SINGLE:
      return try_len(8);
    case FC_WRITE_MULTIPLE:
      if (auto len = try_len(8); len.has_value()) {
        return len;
      }
      if (buf.size() >= 7) {
        const size_t request_len = 9 + buf[6];
        if (auto len = try_len(request_len); len.has_value()) {
          return len;
        }
      }
      return {};
    default:
      return {};
  }
}

bool Hayward::validate_crc_(const uint8_t *data, size_t len) const {
  if (len < 4) {
    return false;
  }
  const uint16_t remote_crc = static_cast<uint16_t>(data[len - 2]) | (static_cast<uint16_t>(data[len - 1]) << 8);
  return this->crc16_(data, len - 2) == remote_crc;
}

uint16_t Hayward::crc16_(const uint8_t *data, size_t len) const {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if ((crc & 0x0001U) != 0U) {
        crc = (crc >> 1) ^ 0xA001U;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

void Hayward::handle_frame_(const std::vector<uint8_t> &frame) {
  if (frame.size() < 4) {
    return;
  }

  const uint8_t address = frame[0];
  const uint8_t function_code = frame[1];

  ESP_LOGV(TAG, "Frame rx: %s", this->format_frame_(frame).c_str());

  if ((function_code & 0x80U) != 0U) {
    return;
  }

  switch (function_code) {
    case FC_READ_HOLDING:
    case FC_READ_INPUT:
      if (frame.size() == 8) {
        this->handle_read_request_(frame);
      } else {
        this->handle_read_response_(frame);
      }
      break;
    case FC_WRITE_SINGLE:
      if (address == this->broadcast_address_ || address == this->controller_address_) {
        this->handle_write_single_request_(frame);
      }
      break;
    case FC_WRITE_MULTIPLE:
      if (frame.size() > 8 && (address == this->broadcast_address_ || address == this->controller_address_)) {
        this->handle_write_multiple_request_(frame);
      }
      break;
    default:
      break;
  }
}

void Hayward::handle_read_request_(const std::vector<uint8_t> &frame) {
  PendingRead request{};
  request.address = frame[0];
  request.function_code = frame[1];
  request.start_address = static_cast<uint16_t>(frame[2] << 8) | frame[3];
  request.register_count = static_cast<uint16_t>(frame[4] << 8) | frame[5];
  request.timestamp_ms = millis();
  this->pending_reads_[request.address] = request;
  ESP_LOGV(TAG, "Read request addr=0x%02X fc=0x%02X start=%u count=%u", request.address, request.function_code,
           request.start_address, request.register_count);
  if (this->send_writes_ && request.address == this->controller_address_ &&
      request.function_code == FC_READ_HOLDING) {
    this->maybe_respond_to_controller_read_(request);
  }
}

void Hayward::handle_read_response_(const std::vector<uint8_t> &frame) {
  const uint8_t address = frame[0];
  auto pending = this->pending_reads_.find(address);
  if (pending == this->pending_reads_.end()) {
    ESP_LOGV(TAG, "Read response from 0x%02X without pending request, ignoring", address);
    return;
  }

  const uint8_t byte_count = frame[2];
  if (byte_count % 2 != 0) {
    this->pending_reads_.erase(pending);
    return;
  }

  const uint16_t register_count = byte_count / 2;
  ESP_LOGV(TAG, "Read response addr=0x%02X start=%u count=%u", address, pending->second.start_address, register_count);
  for (uint16_t index = 0; index < register_count; index++) {
    const size_t offset = 3 + index * 2;
    const uint16_t value = static_cast<uint16_t>(frame[offset] << 8) | frame[offset + 1];
    this->update_register_cache_(pending->second.start_address + index, value);
  }

  this->pending_reads_.erase(pending);
  this->publish_entities_();
}

void Hayward::handle_write_single_request_(const std::vector<uint8_t> &frame) {
  const uint16_t address = static_cast<uint16_t>(frame[2] << 8) | frame[3];
  const uint16_t value = static_cast<uint16_t>(frame[4] << 8) | frame[5];
  ESP_LOGV(TAG, "Write single addr=0x%02X reg=%u value=0x%04X (%u)", frame[0], address, value, value);
  this->update_register_cache_(address, value);
  this->publish_entities_();
}

void Hayward::handle_write_multiple_request_(const std::vector<uint8_t> &frame) {
  const uint16_t start_address = static_cast<uint16_t>(frame[2] << 8) | frame[3];
  const uint16_t register_count = static_cast<uint16_t>(frame[4] << 8) | frame[5];
  const uint8_t byte_count = frame[6];
  if (byte_count != register_count * 2) {
    return;
  }

  ESP_LOGV(TAG, "Write multiple addr=0x%02X start=%u count=%u", frame[0], start_address, register_count);

  for (uint16_t index = 0; index < register_count; index++) {
    const size_t offset = 7 + index * 2;
    const uint16_t value = static_cast<uint16_t>(frame[offset] << 8) | frame[offset + 1];
    this->update_register_cache_(start_address + index, value);
  }

  this->publish_entities_();
}

void Hayward::clear_stale_pending_reads_() {
  const uint32_t now = millis();
  std::vector<uint8_t> stale_addresses;
  for (const auto &[address, request] : this->pending_reads_) {
    if (now - request.timestamp_ms > 250) {
      ESP_LOGV(TAG, "Pending read timed out addr=0x%02X start=%u count=%u", address, request.start_address,
               request.register_count);
      stale_addresses.push_back(address);
    }
  }
  for (const auto address : stale_addresses) {
    this->pending_reads_.erase(address);
  }
}

void Hayward::update_register_cache_(uint16_t address, uint16_t value) {
  auto existing = this->registers_.find(address);
  if (existing == this->registers_.end()) {
    ESP_LOGV(TAG, "Cache new reg=%u value=0x%04X (%u)", address, value, value);
  } else if (existing->second != value) {
    ESP_LOGV(TAG, "Cache update reg=%u old=0x%04X new=0x%04X", address, existing->second, value);
  }
  this->registers_[address] = value;
  this->update_block_tracking_(address);
  if (address == REG_PANEL_HOUR || address == REG_PANEL_MINUTE || address == REG_PANEL_SECOND) {
    this->update_clock_seed_();
  }
}

optional<uint16_t> Hayward::get_register_(uint16_t address) const {
  auto it = this->registers_.find(address);
  if (it == this->registers_.end()) {
    return {};
  }
  return it->second;
}

void Hayward::publish_entities_() {
  this->publish_temperature_sensor_(this->target_temperature_sensor_, REG_TARGET_TEMPERATURE);
  this->publish_scaled_sensor_(this->silent_schedule_start_hour_sensor_, REG_SILENT_SCHEDULE_START_HOUR, 1.0f);
  this->publish_scaled_sensor_(this->silent_schedule_stop_hour_sensor_, REG_SILENT_SCHEDULE_STOP_HOUR, 1.0f);
  this->publish_temperature_sensor_(this->suction_temperature_sensor_, REG_SUCTION_TEMPERATURE);
  this->publish_temperature_sensor_(this->inlet_temperature_sensor_, REG_INLET_TEMPERATURE);
  this->publish_temperature_sensor_(this->outlet_temperature_sensor_, REG_OUTLET_TEMPERATURE);
  this->publish_temperature_sensor_(this->coil_temperature_sensor_, REG_COIL_TEMPERATURE);
  this->publish_temperature_sensor_(this->ambient_temperature_sensor_, REG_AMBIENT_TEMPERATURE);
  this->publish_temperature_sensor_(this->exhaust_temperature_sensor_, REG_EXHAUST_TEMPERATURE);
  this->publish_scaled_sensor_(this->compressor_current_sensor_, REG_COMPRESSOR_CURRENT, 0.1f);
  this->publish_scaled_sensor_(this->compressor_output_current_sensor_, REG_COMPRESSOR_OUTPUT_CURRENT, 0.1f);
  this->publish_scaled_sensor_(this->ac_fan_output_sensor_, REG_AC_FAN_OUTPUT, 1.0f);
  this->publish_temperature_sensor_(this->super_heat_sensor_, REG_SUPER_HEAT);
  this->publish_scaled_sensor_(this->target_speed_fan_motor_sensor_, REG_TARGET_SPEED_FAN_MOTOR, 1.0f);
  this->publish_temperature_sensor_(this->over_heat_after_commpen_sensor_, REG_OVER_HEAT_AFTER_COMMPEN);
  this->publish_scaled_sensor_(this->inverter_plate_ac_voltage_sensor_, REG_INVERTER_PLATE_AC_VOLTAGE, 1.0f);
  this->publish_scaled_sensor_(this->speed_fan_motor_1_sensor_, REG_SPEED_FAN_MOTOR_1, 1.0f);
  this->publish_scaled_sensor_(this->pressure_sensor_, REG_PRESSURE_SENSOR, 0.1f);
  this->publish_scaled_sensor_(this->switch_flags_sensor_, REG_SWITCH_FLAGS, 1.0f);
  this->publish_scaled_sensor_(this->failure_flags_sensor_, REG_FAILURE_FLAGS, 1.0f);
  this->publish_scaled_sensor_(this->protection_flags_sensor_, REG_PROTECTION_FLAGS, 1.0f);
  this->publish_scaled_sensor_(this->inverter_failure_flags_sensor_, REG_INVERTER_FAILURE_FLAGS, 1.0f);
  this->publish_scaled_sensor_(this->fan_failure_flags_sensor_, REG_FAN_FAILURE_FLAGS, 1.0f);
  this->publish_temperature_sensor_(this->antifreeze_temperature_sensor_, REG_ANTIFREEZE_TEMPERATURE);
  this->publish_scaled_sensor_(this->panel_status_flags_sensor_, REG_PANEL_STATUS_FLAGS, 1.0f);
  this->publish_bcd_sensor_(this->panel_hour_sensor_, REG_PANEL_HOUR);
  this->publish_bcd_sensor_(this->panel_minute_sensor_, REG_PANEL_MINUTE);
  this->publish_bcd_sensor_(this->panel_second_sensor_, REG_PANEL_SECOND);
  this->publish_scaled_sensor_(this->power_on_hour_sensor_, REG_POWER_ON_HOUR, 1.0f);
  this->publish_scaled_sensor_(this->power_off_hour_sensor_, REG_POWER_OFF_HOUR, 1.0f);

  if (auto power = this->get_register_(REG_POWER_STATUS); power.has_value()) {
    this->publish_binary_sensor_(this->power_state_binary_sensor_, *power != 0U);
  } else if (auto power = this->get_register_(REG_POWER_COMMAND); power.has_value()) {
    this->publish_binary_sensor_(this->power_state_binary_sensor_, *power != 0U);
  }

  this->publish_toggle_entities_(REG_SILENT_ACTIVE, this->silent_active_binary_sensor_, this->silent_active_switch_);
  this->publish_toggle_entities_(REG_SILENT_SCHEDULE_ACTIVE, this->silent_schedule_active_binary_sensor_,
                                 this->silent_schedule_active_switch_);

  if (auto outputs = this->get_register_(REG_OUTPUT_FLAGS); outputs.has_value()) {
    this->publish_binary_sensor_(this->compressor_running_binary_sensor_, (*outputs & 0x0001U) != 0U);
    this->publish_binary_sensor_(this->water_pump_active_binary_sensor_, (*outputs & 0x0002U) != 0U);
    this->publish_binary_sensor_(this->four_way_valve_active_binary_sensor_, (*outputs & 0x0004U) != 0U);
    this->publish_binary_sensor_(this->fan_high_active_binary_sensor_, (*outputs & 0x0008U) != 0U);
    this->publish_binary_sensor_(this->fan_low_active_binary_sensor_, (*outputs & 0x0010U) != 0U);
  }

  if (auto failures = this->get_register_(REG_FAILURE_FLAGS); failures.has_value()) {
    this->publish_binary_sensor_(this->defrosting_binary_sensor_, (*failures & 0x8000U) != 0U);
  }

  if (auto switch_flags = this->get_register_(REG_SWITCH_FLAGS); switch_flags.has_value()) {
    this->publish_text_sensor_(this->switch_flags_text_sensor_, decode_switch_flags_(*switch_flags));
  }

  std::vector<std::string> error_codes;
  std::vector<std::string> error_descriptions;
  collect_faults_(this->registers_, &error_codes, &error_descriptions);
  this->publish_text_sensor_(this->error_codes_text_sensor_, join_strings_(error_codes));
  this->publish_text_sensor_(this->error_descriptions_text_sensor_, join_strings_(error_descriptions));

  this->publish_toggle_entities_(REG_POWER_ON_SCHEDULE_ACTIVE, this->power_on_schedule_active_binary_sensor_,
                                 this->power_on_schedule_active_switch_);
  this->publish_toggle_entities_(REG_POWER_OFF_SCHEDULE_ACTIVE, this->power_off_schedule_active_binary_sensor_,
                                 this->power_off_schedule_active_switch_);

  if (auto hour = this->get_decoded_bcd_register_(REG_PANEL_HOUR); hour.has_value()) {
    auto minute = this->get_decoded_bcd_register_(REG_PANEL_MINUTE);
    auto second = this->get_decoded_bcd_register_(REG_PANEL_SECOND);
    if (minute.has_value() && second.has_value()) {
      char buffer[16];
      snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", *hour, *minute, *second);
      this->publish_text_sensor_(this->panel_clock_text_sensor_, buffer);
    }
  }

  this->publish_hour_value_(REG_SILENT_SCHEDULE_START_HOUR, this->silent_schedule_start_hour_sensor_,
                            this->silent_schedule_start_hour_number_, nullptr);
  this->publish_hour_value_(REG_SILENT_SCHEDULE_STOP_HOUR, this->silent_schedule_stop_hour_sensor_,
                            this->silent_schedule_stop_hour_number_, nullptr);
  this->publish_schedule_window_(REG_SILENT_SCHEDULE_START_HOUR, REG_SILENT_SCHEDULE_STOP_HOUR,
                                 this->silent_schedule_window_text_sensor_);
  this->publish_hour_value_(REG_POWER_ON_HOUR, this->power_on_hour_sensor_, this->power_on_hour_number_,
                            this->power_on_schedule_time_text_sensor_);
  this->publish_hour_value_(REG_POWER_OFF_HOUR, this->power_off_hour_sensor_, this->power_off_hour_number_,
                            this->power_off_schedule_time_text_sensor_);
  this->publish_schedule_window_(REG_POWER_ON_HOUR, REG_POWER_OFF_HOUR, this->power_schedule_window_text_sensor_);

  if (auto flags = this->get_register_(REG_PANEL_STATUS_FLAGS); flags.has_value()) {
    this->publish_text_sensor_(this->panel_update_flags_text_sensor_, decode_panel_status_flags_(*flags));
  }

  this->publish_climate_();
  this->publish_mode_text_();
}

void Hayward::publish_temperature_sensor_(sensor::Sensor *sensor, uint16_t address) {
  if (sensor == nullptr) {
    return;
  }
  auto value = this->get_register_(address);
  if (!value.has_value() || *value == 0xFFFFU) {
    return;
  }

  const float converted = static_cast<float>(static_cast<int16_t>(*value)) / 10.0f;
  if (!sensor->has_state() || std::fabs(sensor->state - converted) > 0.05f) {
    sensor->publish_state(converted);
  }
}

void Hayward::publish_scaled_sensor_(sensor::Sensor *sensor, uint16_t address, float scale) {
  if (sensor == nullptr) {
    return;
  }
  auto value = this->get_register_(address);
  if (!value.has_value()) {
    return;
  }

  const float converted = static_cast<float>(*value) * scale;
  if (!sensor->has_state() || std::fabs(sensor->state - converted) > 0.05f) {
    sensor->publish_state(converted);
  }
}

void Hayward::publish_bcd_sensor_(sensor::Sensor *sensor, uint16_t address) {
  if (sensor == nullptr) {
    return;
  }
  auto value = this->get_register_(address);
  if (!value.has_value()) {
    return;
  }

  const float converted = static_cast<float>(this->decode_bcd_(*value));
  if (!sensor->has_state() || std::fabs(sensor->state - converted) > 0.05f) {
    sensor->publish_state(converted);
  }
}

void Hayward::publish_binary_sensor_(binary_sensor::BinarySensor *sensor, bool value) {
  if (sensor == nullptr) {
    return;
  }
  if (!sensor->has_state() || sensor->state != value) {
    sensor->publish_state(value);
  }
}

void Hayward::publish_text_sensor_(text_sensor::TextSensor *sensor, const std::string &value) {
  if (sensor == nullptr) {
    return;
  }
  if (sensor->state != value) {
    sensor->publish_state(value);
  }
}

void Hayward::publish_switch_(switch_::Switch *sw, bool value) {
  if (sw == nullptr) {
    return;
  }
  if (sw->state != value) {
    sw->publish_state(value);
  }
}

void Hayward::publish_number_(number::Number *num, float value) {
  if (num == nullptr) {
    return;
  }
  if (std::isnan(num->state) || std::fabs(num->state - value) > 0.05f) {
    num->publish_state(value);
  }
}

void Hayward::publish_toggle_entities_(uint16_t address, binary_sensor::BinarySensor *binary_sensor, switch_::Switch *sw) {
  if (auto value = this->get_register_(address); value.has_value()) {
    const bool enabled = *value != 0U;
    this->publish_binary_sensor_(binary_sensor, enabled);
    this->publish_switch_(sw, enabled);
  }
}

void Hayward::publish_hour_value_(uint16_t address, sensor::Sensor *sensor, number::Number *number_entity,
                                  text_sensor::TextSensor *time_sensor) {
  if (auto hour = this->get_register_(address); hour.has_value()) {
    this->publish_scaled_sensor_(sensor, address, 1.0f);
    this->publish_number_(number_entity, static_cast<float>(*hour));
    this->publish_text_sensor_(time_sensor, this->format_hour_(*hour));
  }
}

void Hayward::publish_schedule_window_(uint16_t start_address, uint16_t stop_address, text_sensor::TextSensor *window_sensor) {
  auto start_hour = this->get_register_(start_address);
  auto stop_hour = this->get_register_(stop_address);
  if (!start_hour.has_value() || !stop_hour.has_value()) {
    return;
  }

  this->publish_text_sensor_(window_sensor, this->format_hour_(*start_hour) + "-" + this->format_hour_(*stop_hour));
}

void Hayward::publish_mode_text_() {
  if (this->mode_text_sensor_ == nullptr) {
    return;
  }

  optional<uint16_t> mode_register = this->get_register_(REG_MODE_STATUS);
  if (!mode_register.has_value()) {
    mode_register = this->get_register_(REG_MODE_COMMAND);
  }
  if (!mode_register.has_value()) {
    mode_register = this->get_register_(REG_CAPABILITY_MODE);
  }
  if (!mode_register.has_value()) {
    return;
  }

  switch (*mode_register) {
    case 0:
      this->publish_text_sensor_(this->mode_text_sensor_, "cool");
      break;
    case 1:
      this->publish_text_sensor_(this->mode_text_sensor_, "heat");
      break;
    case 2:
      this->publish_text_sensor_(this->mode_text_sensor_, "auto");
      break;
    default:
      this->publish_text_sensor_(this->mode_text_sensor_, "unknown");
      break;
  }
}

uint16_t Hayward::decode_bcd_(uint16_t value) const {
  uint16_t decoded = 0;
  uint16_t multiplier = 1;
  while (value > 0U) {
    decoded += (value & 0x0FU) * multiplier;
    multiplier *= 10U;
    value >>= 4U;
  }
  return decoded;
}

optional<uint16_t> Hayward::get_decoded_bcd_register_(uint16_t address) const {
  auto value = this->get_register_(address);
  if (!value.has_value()) {
    return {};
  }
  return this->decode_bcd_(*value);
}

std::string Hayward::format_hour_(uint16_t hour) const {
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%02u:00", hour);
  return buffer;
}

void Hayward::publish_climate_() {
  if (this->hayward_climate_ == nullptr) {
    return;
  }

  bool changed = false;

  if (auto inlet = this->get_register_(REG_INLET_TEMPERATURE); inlet.has_value()) {
    const float current_temperature = static_cast<float>(static_cast<int16_t>(*inlet)) / 10.0f;
    if (std::isnan(this->hayward_climate_->current_temperature) ||
        std::fabs(this->hayward_climate_->current_temperature - current_temperature) > 0.05f) {
      this->hayward_climate_->current_temperature = current_temperature;
      changed = true;
    }
  }

  const bool use_staged_control_state =
      !this->send_writes_ || this->pending_settings_update_ || this->pending_extra_settings_update_;
  climate::ClimateMode new_mode = climate::CLIMATE_MODE_OFF;
  bool have_mode = false;

  optional<uint16_t> power_register;
  if (use_staged_control_state) {
    power_register = this->get_register_(REG_POWER_COMMAND);
  }
  if (!power_register.has_value()) {
    power_register = this->get_register_(REG_POWER_STATUS);
  }

  if (power_register.has_value() && *power_register == 0U) {
    new_mode = climate::CLIMATE_MODE_OFF;
    have_mode = true;
  } else {
    optional<uint16_t> mode_register;
    if (use_staged_control_state) {
      mode_register = this->get_register_(REG_MODE_COMMAND);
    }
    if (!mode_register.has_value()) {
      mode_register = this->get_register_(REG_MODE_STATUS);
    }
    if (!mode_register.has_value()) {
      mode_register = this->get_register_(REG_MODE_COMMAND);
    }
    if (!mode_register.has_value()) {
      mode_register = this->get_register_(REG_CAPABILITY_MODE);
    }
    if (mode_register.has_value()) {
      have_mode = true;
      switch (*mode_register) {
        case 0:
          new_mode = climate::CLIMATE_MODE_COOL;
          break;
        case 1:
          new_mode = climate::CLIMATE_MODE_HEAT;
          break;
        case 2:
          new_mode = climate::CLIMATE_MODE_AUTO;
          break;
        default:
          new_mode = climate::CLIMATE_MODE_OFF;
          break;
      }
    }
  }

  if (have_mode && this->hayward_climate_->mode != new_mode) {
    this->hayward_climate_->mode = new_mode;
    changed = true;
  }

  optional<uint16_t> target_register = this->get_register_(REG_TARGET_TEMPERATURE);
  if (!target_register.has_value()) {
    switch (new_mode) {
      case climate::CLIMATE_MODE_COOL:
        target_register = this->get_register_(REG_SAVED_COOL_TEMPERATURE);
        break;
      case climate::CLIMATE_MODE_HEAT:
        target_register = this->get_register_(REG_SAVED_HEAT_TEMPERATURE);
        break;
      case climate::CLIMATE_MODE_AUTO:
        target_register = this->get_register_(REG_SAVED_AUTO_TEMPERATURE);
        break;
      default:
        break;
    }
  }
  if (!target_register.has_value()) {
    target_register = this->get_register_(REG_SAVED_HEAT_TEMPERATURE);
  }
  if (!target_register.has_value()) {
    target_register = this->get_register_(REG_SAVED_COOL_TEMPERATURE);
  }
  if (!target_register.has_value()) {
    target_register = this->get_register_(REG_SAVED_AUTO_TEMPERATURE);
  }
  if (target_register.has_value()) {
    const float target_temperature = static_cast<float>(static_cast<int16_t>(*target_register)) / 10.0f;
    if (std::isnan(this->hayward_climate_->target_temperature) ||
        std::fabs(this->hayward_climate_->target_temperature - target_temperature) > 0.05f) {
      this->hayward_climate_->target_temperature = target_temperature;
      changed = true;
    }
  } else if (std::isnan(this->hayward_climate_->target_temperature) &&
             !std::isnan(this->hayward_climate_->current_temperature)) {
    // Keep the climate entity operable even before the settings registers appear on the bus.
    const float fallback_target = clamp(this->hayward_climate_->current_temperature, 15.0f, 35.0f);
    this->hayward_climate_->target_temperature = fallback_target;
    changed = true;
  }

  climate::ClimateAction new_action = climate::CLIMATE_ACTION_OFF;
  bool compressor_active = false;
  if (auto outputs = this->get_register_(REG_OUTPUT_FLAGS); outputs.has_value()) {
    compressor_active = (*outputs & 0x0001U) != 0U;
  }
  if (!compressor_active) {
    if (new_mode == climate::CLIMATE_MODE_OFF) {
      new_action = climate::CLIMATE_ACTION_OFF;
    } else {
      new_action = climate::CLIMATE_ACTION_IDLE;
    }
  } else {
    auto inlet = this->get_register_(REG_INLET_TEMPERATURE);
    auto outlet = this->get_register_(REG_OUTLET_TEMPERATURE);
    if (new_mode == climate::CLIMATE_MODE_HEAT) {
      new_action = climate::CLIMATE_ACTION_HEATING;
    } else if (new_mode == climate::CLIMATE_MODE_COOL) {
      new_action = climate::CLIMATE_ACTION_COOLING;
    } else if (new_mode == climate::CLIMATE_MODE_AUTO && inlet.has_value() && outlet.has_value()) {
      new_action = (*outlet > *inlet) ? climate::CLIMATE_ACTION_HEATING : climate::CLIMATE_ACTION_COOLING;
    } else {
      new_action = climate::CLIMATE_ACTION_IDLE;
    }
  }

  if (this->hayward_climate_->action != new_action) {
    this->hayward_climate_->action = new_action;
    changed = true;
  }

  optional<climate::ClimatePreset> new_preset = climate::CLIMATE_PRESET_NONE;
  if (auto silent = this->get_register_(REG_SILENT_ACTIVE); silent.has_value() && *silent != 0U) {
    new_preset = climate::CLIMATE_PRESET_ECO;
  }
  if (this->hayward_climate_->preset != new_preset) {
    this->hayward_climate_->preset = new_preset;
    changed = true;
  }

  if (changed) {
    this->hayward_climate_->publish_state();
  }
}

bool Hayward::maybe_respond_to_controller_read_(const PendingRead &request) {
  const bool supported_range =
      (request.start_address == STATUS_START && (request.register_count == STATUS_COUNT || request.register_count == 30U)) ||
      (request.start_address == SETTINGS_START && request.register_count == SETTINGS_COUNT) ||
      (request.start_address == EXTRA_SETTINGS_START && request.register_count == EXTRA_SETTINGS_COUNT);
  if (!supported_range) {
    ESP_LOGV(TAG, "Ignoring controller read outside emulated ranges: start=%u count=%u", request.start_address,
             request.register_count);
    return false;
  }

  auto values = this->build_register_block_(request.start_address, request.register_count);
  if (values.empty()) {
    ESP_LOGW(TAG, "Cannot emulate read for start=%u count=%u", request.start_address, request.register_count);
    return false;
  }

  if (request.start_address == SETTINGS_START) {
    if (this->pending_settings_update_) {
      ESP_LOGI(TAG, "Delivered staged settings to PC1002: start=%u count=%u", request.start_address,
               request.register_count);
    }
    this->pending_settings_update_ = false;
    this->settings_applying_until_ms_ = millis() + 3000U;
  } else if (request.start_address == EXTRA_SETTINGS_START) {
    if (this->pending_extra_settings_update_) {
      ESP_LOGI(TAG, "Delivered staged extra settings to PC1002: start=%u count=%u", request.start_address,
               request.register_count);
    }
    this->pending_extra_settings_update_ = false;
    this->extra_settings_applying_until_ms_ = millis() + 3000U;
  }

  return this->send_emulated_read_response_(request.address, request.function_code, request.start_address, values);
}

std::vector<uint16_t> Hayward::build_register_block_(uint16_t start_address, uint16_t register_count) {
  std::vector<uint16_t> values;
  values.reserve(register_count);
  for (uint16_t index = 0; index < register_count; index++) {
    const uint16_t address = start_address + index;
    if (start_address == STATUS_START) {
      values.push_back(this->build_status_register_(address));
      continue;
    }

    auto value = this->get_register_(address);
    values.push_back(value.value_or(0U));
  }
  return values;
}

uint16_t Hayward::build_status_register_(uint16_t address) {
  if (address == REG_STATUS_ADDRESS) {
    return STATUS_START;
  }

  if (address == REG_PANEL_STATUS_FLAGS) {
    if (this->pending_settings_update_) {
      return FLAG_SETTINGS_UPDATE;
    }
    if (this->pending_extra_settings_update_) {
      return FLAG_EXTRA_SETTINGS_UPDATE;
    }
    const uint32_t now = millis();
    if (now < this->settings_applying_until_ms_) {
      return FLAG_SETTINGS_APPLYING;
    }
    if (now < this->extra_settings_applying_until_ms_) {
      return FLAG_EXTRA_SETTINGS_UPDATE;
    }
    if (!this->has_settings_snapshot_ || !this->has_extra_settings_snapshot_) {
      return FLAG_NEEDS_UPDATES;
    }
    return 0U;
  }

  if (address == REG_PANEL_HOUR || address == REG_PANEL_MINUTE || address == REG_PANEL_SECOND) {
    if (this->clock_seed_timestamp_ms_ != 0U) {
      const uint32_t elapsed_seconds = (millis() - this->clock_seed_timestamp_ms_) / 1000U;
      uint32_t hours = this->decode_bcd_(this->clock_seed_hour_bcd_);
      uint32_t minutes = this->decode_bcd_(this->clock_seed_minute_bcd_);
      uint32_t seconds = this->decode_bcd_(this->clock_seed_second_bcd_);
      uint32_t total_seconds = hours * 3600U + minutes * 60U + seconds + elapsed_seconds;
      total_seconds %= 24U * 3600U;
      const uint16_t current_hour = total_seconds / 3600U;
      const uint16_t current_minute = (total_seconds % 3600U) / 60U;
      const uint16_t current_second = total_seconds % 60U;
      auto encode_bcd = [](uint16_t value) -> uint16_t { return static_cast<uint16_t>(((value / 10U) << 4U) | (value % 10U)); };
      if (address == REG_PANEL_HOUR) {
        return encode_bcd(current_hour);
      }
      if (address == REG_PANEL_MINUTE) {
        return encode_bcd(current_minute);
      }
      return encode_bcd(current_second);
    }
  }

  if (auto value = this->get_register_(address); value.has_value()) {
    return *value;
  }

  if (address >= STATUS_START && address <= REG_STATUS_ADDRESS) {
    const uint16_t diagnostic_address = 2001 + (address - STATUS_START);
    if (auto value = this->get_register_(diagnostic_address); value.has_value()) {
      return *value;
    }
  }

  return 0U;
}

void Hayward::update_block_tracking_(uint16_t address) {
  if (address >= SETTINGS_START && address <= SETTINGS_END) {
    this->has_settings_snapshot_ = true;
  } else if (address >= EXTRA_SETTINGS_START && address <= EXTRA_SETTINGS_END) {
    this->has_extra_settings_snapshot_ = true;
  } else if (address >= STATUS_START && address <= STATUS_END) {
    this->has_status_snapshot_ = true;
  }
}

void Hayward::update_clock_seed_() {
  auto hour = this->get_register_(REG_PANEL_HOUR);
  auto minute = this->get_register_(REG_PANEL_MINUTE);
  auto second = this->get_register_(REG_PANEL_SECOND);
  if (!hour.has_value() || !minute.has_value() || !second.has_value()) {
    return;
  }
  this->clock_seed_hour_bcd_ = *hour;
  this->clock_seed_minute_bcd_ = *minute;
  this->clock_seed_second_bcd_ = *second;
  this->clock_seed_timestamp_ms_ = millis();
}

std::string Hayward::format_frame_(const std::vector<uint8_t> &frame) const {
  std::ostringstream stream;
  stream << "addr=0x" << format_hex(frame[0]) << " fc=0x" << format_hex(frame[1]) << " len=" << frame.size()
         << " data=" << format_hex_pretty(frame.data(), frame.size());
  return stream.str();
}

}  // namespace esphome::hayward
