#include "component.h"

#include "freertos/task.h"
#include "freertos/queue.h"

#include "decode3of6.h"
#include "packets.h"
#include "wmbusmeters.h"

#define ASSERT(expr, expected, before_exit)                       \
  {                                                               \
    auto result = (expr);                                         \
    if (!!result != expected)                                     \
    {                                                             \
      ESP_LOGE(TAG, "Assertion failed: %s -> %d", #expr, result); \
      before_exit;                                                \
      return;                                                     \
    }                                                             \
  }

#define ASSERT_SETUP(expr) ASSERT(expr, 1, this->mark_failed())

namespace esphome
{
  namespace wmbus_radio
  {
    static const char *TAG = "wmbus";

    void Radio::setup()
    {
      ASSERT_SETUP(this->packet_queue_ = xQueueCreate(10, sizeof(Packet *)));

      ASSERT_SETUP(xTaskCreate(
          (TaskFunction_t)this->receiver_task,
          "radio_recv",
          8 * 1024,
          this,
          12,
          &(this->receiver_task_handle_)));

      ESP_LOGE(TAG, "Receiver task created");
      ESP_LOGE(TAG, "Receiver task handle: %p", this->receiver_task_handle_);

      this->radio->attach_data_interrupt(Radio::wakeup_receiver_task_from_isr, &(this->receiver_task_handle_));
    }

    void Radio::loop()
    {
      Packet *p;
      if (xQueueReceive(this->packet_queue_, &p, 0) != pdTRUE)
        return;

      std::shared_ptr<Packet> packet(p);

      ESP_LOGI(TAG, "Have data from radio ...");

      if (packet->data[0] != WMBUS_MODE_C_PREAMBLE)
      {
        auto decoded = decode3of6(packet->data);
        if (decoded.has_value())
        {
          packet->data = *decoded;
        }
      }

      ESP_LOGI(TAG, "%s", format_hex(packet->data).c_str());
      ESP_LOGI(TAG, "RSSI: %d", packet->rssi);

      removeAnyDLLCRCs(packet->data);
      int dummy;
      if (checkWMBusFrame(packet->data, (size_t *)&dummy, &dummy, &dummy, false) != FrameStatus::FullFrame)
      {
        ESP_LOGE(TAG, "Frame check failed");
        return;
      }

      this->packet_callback_manager_.call(packet.get());
    }

    void Radio::wakeup_receiver_task_from_isr(TaskHandle_t *arg)
    {
      BaseType_t xHigherPriorityTaskWoken;
      vTaskNotifyGiveFromISR(*arg, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    void Radio::receive_frame()
    {
      ESP_LOGD(TAG, "RX Mode reboot");
      this->radio->restart_rx();

      if (!ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(60000)))
      {
        ESP_LOGD(TAG, "Radio interrupt timeout");
        return;
      }

      std::vector<uint8_t> frame_data(WMBUS_PREAMBLE_SIZE);

      if (!this->radio->read_in_task(frame_data.data(), WMBUS_PREAMBLE_SIZE))
      {
        ESP_LOGD(TAG, "Failed to read preamble");
        return;
      }

      auto total_length = get_packet_size(frame_data);

      if (total_length == 0)
      {
        ESP_LOGD(TAG, "Cannot decode preamble");
        return;
      }

      frame_data.resize(total_length);

      if (!this->radio->read_in_task(frame_data.data() + WMBUS_PREAMBLE_SIZE, total_length - WMBUS_PREAMBLE_SIZE))
      {
        ESP_LOGW(TAG, "Failed to read data");
        return;
      }

      auto packet = new Packet{
          .data = frame_data,
          .rssi = radio->get_rssi(),
      };

      if (xQueueSend(this->packet_queue_, &packet, 0) != pdTRUE)
      {
        ESP_LOGW(TAG, "Queue send failed");
        delete packet;
      }
      else
      {
        ESP_LOGI(TAG, "Queue items: %zu", uxQueueMessagesWaiting(this->packet_queue_));
        ESP_LOGI(TAG, "Queue send success");
      }
    }

    void Radio::receiver_task(Radio *arg)
    {
      ESP_LOGE(TAG, "Hello from radio task!");

      while (true)
        arg->receive_frame();
    }

    void Radio::add_on_packet_callback(std::function<void(Packet *)> &&callback)
    {
      this->packet_callback_manager_.add(std::move(callback));
    }

  } // namespace wmbus
} // namespace esphome
