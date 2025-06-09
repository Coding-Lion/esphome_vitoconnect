#pragma once

#include "esphome/components/select/select.h"
#include "../vitoconnect_datapoint.h"
#include <map>

namespace esphome {
namespace vitoconnect {

class OPTOLINKSelect : public select::Select, public Datapoint {

  public:
    OPTOLINKSelect();
    ~OPTOLINKSelect();

    void decode(uint8_t* data, uint8_t length, Datapoint* dp = nullptr) override;
    void encode(uint8_t* raw, uint8_t length, void* data) override;
    void encode(uint8_t* raw, uint8_t length, const std::string& data);

    void add_mapping(int key, const std::string& value);
    void set_map(std::map<std::string, std::string> *mapping);

  protected:
    void control(const std::string& value) override;
    
  private:
    std::map<std::string, std::string> *mapping_ = nullptr;
    void datapoint_value_changed(const std::string& value);
    void datapoint_value_changed(uint8_t value);

};

}  // namespace vitoconnect
}  // namespace esphome 