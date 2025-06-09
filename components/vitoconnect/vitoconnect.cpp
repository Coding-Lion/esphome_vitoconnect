/*
  optolink.cpp - Connect Viessmann heating devices via Optolink to ESPhome

  Copyright (C) 2023  Philipp Danner

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "vitoconnect.h"

namespace esphome {
namespace vitoconnect {

static const char *TAG = "vitoconnect";

void VitoConnect::setup() {

    this->check_uart_settings(4800, 2, uart::UART_CONFIG_PARITY_EVEN, 8);

    ESP_LOGD(TAG, "Starting optolink with protocol: %s", this->protocol.c_str());
    if (this->protocol.compare("P300") == 0) {
        _optolink = new OptolinkP300(this);
    } else if (this->protocol.compare("KW") == 0) {
        _optolink = new OptolinkKW(this);
    } else {
      ESP_LOGW(TAG, "Unknown protocol.");
    }

    // optimize datapoint list
    _datapoints.shrink_to_fit();

    if (_optolink) {

      // add onData and onError callbacks
      _optolink->onData(&VitoConnect::_onData);
      _optolink->onError(&VitoConnect::_onError);
      
      // set initial state
      _optolink->begin();

    } else {
      ESP_LOGW(TAG, "Not able to initialize VitoConnect");
    }
}

void VitoConnect::register_datapoint(Datapoint *datapoint) {
    ESP_LOGD(TAG, "Adding datapoint with address %x and length %d", datapoint->getAddress(), datapoint->getLength());
    this->_datapoints.push_back(datapoint);
}

void VitoConnect::loop() {
    _optolink->loop();
}

void VitoConnect::update() {
  // This will be called every "update_interval" milliseconds.
  ESP_LOGD(TAG, "Schedule sensor update");
  
  for (Datapoint* dp : this->_datapoints) {
      CbArg* arg = new CbArg(this, dp);   
      if (_optolink->read(dp->getAddress(), dp->getLength(), reinterpret_cast<void*>(arg))) {
      } else {
          delete arg;
      }
  }
}

void VitoConnect::_onData(uint8_t* data, uint8_t len, void* arg) {
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  cbArg->dp->decode(data, len, cbArg->dp);
  delete cbArg;
}

void VitoConnect::_onError(uint8_t error, void* arg) {
  ESP_LOGD(TAG, "Error received: %d", error);
  CbArg* cbArg = reinterpret_cast<CbArg*>(arg);
  if (cbArg->v->_onErrorCb) cbArg->v->_onErrorCb(error, cbArg->dp);
  delete cbArg;
}

bool VitoConnect::write_datapoint(Datapoint* datapoint, uint8_t* data, uint8_t length) {
  if (!_optolink) {
    ESP_LOGW(TAG, "Optolink not initialized");
    return false;
  }
  
  if (length != datapoint->getLength()) {
    ESP_LOGW(TAG, "Write data length %d doesn't match datapoint length %d", length, datapoint->getLength());
    return false;
  }
  
  // Create a temporary copy of the data to ensure it stays valid
  uint8_t* temp_data = new uint8_t[length];
  memcpy(temp_data, data, length);
  
  ESP_LOGD(TAG, "write_datapoint called with data[0] = 0x%02X, temp_data[0] = 0x%02X", data[0], temp_data[0]);
  
  CbArg* arg = new CbArg(this, datapoint);
  bool success = _optolink->write(datapoint->getAddress(), length, temp_data, reinterpret_cast<void*>(arg));
  
  // Clean up temp_data - it's been copied by OptolinkDP constructor
  delete[] temp_data;
  
  if (!success) {
    delete arg;
    ESP_LOGW(TAG, "Failed to enqueue write request for address 0x%04X", datapoint->getAddress());
  } else {
    ESP_LOGD(TAG, "Write request enqueued for address 0x%04X, length %d", datapoint->getAddress(), length);
  }
  
  return success;
}

}  // namespace vitoconnect
}  // namespace esphome
