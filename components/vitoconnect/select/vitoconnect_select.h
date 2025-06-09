#pragma once

#include "esphome/components/select/select.h"
#include "../vitoconnect_datapoint.h"
#include "../vitoconnect_optolink.h"
#include <map>

namespace esphome {
namespace vitoconnect {

class VitoConnect;  // Forward declaration

class OPTOLINKSelect : public select::Select, public Datapoint {

  public:
    OPTOLINKSelect();
    ~OPTOLINKSelect();

    void set_parent(VitoConnect* parent) { parent_ = parent; }

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, const std::string& data);

    void add_mapping(int key, const std::string& value);
    void set_map(std::map<std::string, std::string> *mapping);

  protected:
    void control(const std::string& value) override;
    
  private:
    std::map<std::string, std::string> *mapping_ = nullptr;
    VitoConnect* parent_ = nullptr;
    uint8_t write_buffer_[MAX_DP_LENGTH];  // Buffer to store data for writing
    void datapoint_value_changed(const std::string& value);
    void datapoint_value_changed(uint8_t value);

};

}  // namespace vitoconnect
}  // namespace esphome 