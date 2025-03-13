#pragma once

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"

#include "esphome/core/component.h"
#include "esphome/core/gpio.h"
#include "esphome/core/helpers.h"

#include "esphome/components/spi/spi.h"

#include "transceiver.h"
#include "version.h"

namespace esphome
{
  namespace wmbus_radio
  {
    struct Packet
    {
      std::vector<uint8_t> data;
      int8_t rssi;
    };

    class Radio : public Component
    {
    public:
      void set_radio(RadioTransceiver *radio) { this->radio = radio; };

      void setup() override;
      void loop() override;
      void receive_frame();

      void add_on_packet_callback(std::function<void(Packet*)> &&callback);

    protected:
      static void wakeup_receiver_task_from_isr(TaskHandle_t *arg);
      static void receiver_task(Radio *arg);

      RadioTransceiver *radio{nullptr};
      TaskHandle_t receiver_task_handle_{nullptr};
      QueueHandle_t packet_queue_{nullptr};

      CallbackManager<void(Packet*)> packet_callback_manager_;
    };
  } // namespace wmbus
} // namespace esphome