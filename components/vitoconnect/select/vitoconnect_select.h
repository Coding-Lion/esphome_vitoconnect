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

  protected:
    void control(const std::string& value) override;
    
  private:
    std::map<int, std::string> value_to_option_map_;
    std::map<std::string, int> option_to_value_map_;

};

}  // namespace vitoconnect
}  // namespace esphome 