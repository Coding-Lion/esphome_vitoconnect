#include "vitoconnect_select.h"
#include "esphome/core/log.h"

static const char *const TAG = "vitoconnect.select";

namespace esphome {
namespace vitoconnect {

OPTOLINKSelect::OPTOLINKSelect(){
  // empty
}

OPTOLINKSelect::~OPTOLINKSelect() {
  // empty
}

void OPTOLINKSelect::add_mapping(int key, const std::string& value) {
  value_to_option_map_[key] = value;
  option_to_value_map_[value] = key;
}

void OPTOLINKSelect::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);

  if (!dp) dp = this;

  if (_length == 1) {
    uint8_t raw_value = data[0];
    
    auto it = value_to_option_map_.find(raw_value);
    if (it != value_to_option_map_.end()) {
      publish_state(it->second);
      ESP_LOGD(TAG, "Decoded value %d to option '%s'", raw_value, it->second.c_str());
    } else {
      ESP_LOGW(TAG, "Unknown value %d received", raw_value);
    }
  } else {
    ESP_LOGW(TAG, "Select component only supports 1 byte length, got %d", _length);
  }
}

void OPTOLINKSelect::encode(uint8_t* raw, uint8_t length, void* data) {
  std::string value = *reinterpret_cast<std::string*>(data);
  encode(raw, length, value);
}

void OPTOLINKSelect::encode(uint8_t* raw, uint8_t length, const std::string& data) {
  assert(length >= _length);

  if (_length == 1) {
    auto it = option_to_value_map_.find(data);
    if (it != option_to_value_map_.end()) {
      raw[0] = static_cast<uint8_t>(it->second);
      ESP_LOGD(TAG, "Encoded option '%s' to value %d", data.c_str(), it->second);
    } else {
      ESP_LOGW(TAG, "Unknown option '%s' for encoding", data.c_str());
    }
  } else {
    ESP_LOGW(TAG, "Select component only supports 1 byte length, got %d", _length);
  }
}

void OPTOLINKSelect::control(const std::string& value) {
  // This method is called when the select value is changed via ESPHome
  // The vitoconnect component should handle writing the value to the device
  auto it = option_to_value_map_.find(value);
  if (it != option_to_value_map_.end()) {
    ESP_LOGD(TAG, "Select control called with option '%s' (value %d)", value.c_str(), it->second);
    // The actual writing to the device would be handled by the parent component
    // when it detects this datapoint has been updated
    publish_state(value);
  } else {
    ESP_LOGW(TAG, "Invalid option '%s' selected", value.c_str());
  }
}

}  // namespace vitoconnect
}  // namespace esphome 