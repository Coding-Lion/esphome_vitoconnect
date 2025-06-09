#include "vitoconnect_select.h"
#include "../vitoconnect.h"
#include "esphome/core/log.h"

static const char *const TAG = "vitoconnect.select";

namespace esphome {
namespace vitoconnect {

OPTOLINKSelect::OPTOLINKSelect(){
  // Initialize write buffer to zero
  memset(write_buffer_, 0, sizeof(write_buffer_));
}

OPTOLINKSelect::~OPTOLINKSelect() {
  // empty
}

void OPTOLINKSelect::add_mapping(int key, const std::string& value) {
  // This method is called during setup to build the mapping
  // We'll need to convert this to the string-to-string mapping format
  if (!mapping_) {
    mapping_ = new std::map<std::string, std::string>();
  }
  std::string key_str = std::to_string(key);
  (*mapping_)[key_str] = value;
  
  // Update the select options
  std::vector<std::string> options;
  for (auto &it : *mapping_) {
    options.push_back(it.second);
  }
  traits.set_options(options);
}

void OPTOLINKSelect::set_map(std::map<std::string, std::string> *mapping) {
  mapping_ = mapping;
  std::vector<std::string> options;
  for (auto &it : *mapping) {
    options.push_back(it.second);
  }
  traits.set_options(options);
}

void OPTOLINKSelect::decode(uint8_t* data, uint8_t length, Datapoint* dp) {
  assert(length >= _length);

  if (!dp) dp = this;

  if (_length == 1) {
    uint8_t raw_value = data[0];
    datapoint_value_changed(raw_value);
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

  if (_length == 1 && mapping_) {
    // Find the key for this value
    for (auto it = mapping_->begin(); it != mapping_->end(); ++it) {
      if (it->second == data) {
        // Convert string to integer safely without exceptions
        char* endptr;
        long key_long = strtol(it->first.c_str(), &endptr, 10);
        if (*endptr == '\0' && key_long >= 0 && key_long <= 255) {
          int key = static_cast<int>(key_long);
          raw[0] = static_cast<uint8_t>(key);
          ESP_LOGD(TAG, "Encoded option '%s' to value %d, raw[0] = 0x%02X", data.c_str(), key, raw[0]);
          return;
        } else {
          ESP_LOGW(TAG, "Invalid key '%s' for encoding", it->first.c_str());
        }
      }
    }
    ESP_LOGW(TAG, "Unknown option '%s' for encoding", data.c_str());
  } else {
    ESP_LOGW(TAG, "Select component only supports 1 byte length, got %d", _length);
  }
}

void OPTOLINKSelect::control(const std::string& value) {
  if (!mapping_) {
    ESP_LOGW(TAG, "No mapping configured for select");
    return;
  }

  if (!parent_) {
    ESP_LOGW(TAG, "No parent component configured for select");
    return;
  }

  for (auto it = mapping_->begin(); it != mapping_->end(); ++it) {
    if (it->second == value) {
      ESP_LOGI(TAG, "Control of select %s to value %s", get_name().c_str(), it->first.c_str());
      
      // Encode the value to the member buffer
      encode(write_buffer_, 1, value);
      ESP_LOGD(TAG, "After encode, write_buffer_[0] = 0x%02X", write_buffer_[0]);
      
      // Write to device via parent component  
      bool success = parent_->write_datapoint(this, write_buffer_, 1);
      
      if (success) {
        ESP_LOGD(TAG, "Write request sent successfully");
        // Don't publish state immediately - wait for confirmation from device
        // The state will be updated when the device responds with the written value
      } else {
        ESP_LOGW(TAG, "Failed to send write request");
      }
      
      break;
    }
  }
}

void OPTOLINKSelect::datapoint_value_changed(const std::string& value) {
  if (!mapping_) {
    ESP_LOGW(TAG, "No mapping configured for select");
    return;
  }

  auto pos = mapping_->find(value);
  if (pos == mapping_->end()) {
    ESP_LOGE(TAG, "Value %s not found in select %s", value.c_str(), get_name().c_str());
  } else {
    publish_state(pos->second);
  }
}

void OPTOLINKSelect::datapoint_value_changed(uint8_t value) {
  std::string key = std::to_string(value);
  datapoint_value_changed(key);
}

}  // namespace vitoconnect
}  // namespace esphome 