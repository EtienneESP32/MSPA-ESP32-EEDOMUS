#pragma once

#include "esphome/core/component.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/network/util.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "errno.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mspa_ping {

static const char *const TAG = "mspa_ping";

class MspaPingComponent : public PollingComponent, public binary_sensor::BinarySensor {
 public:
  void set_target_ip(const std::string &ip) { ip_str_ = ip; }

  void setup() override {
    ESP_LOGCONFIG(TAG, "Setting up Mspa Ping Radar for %s", ip_str_.c_str());
    // On initialise à false par sécurité
    this->publish_state(false);
  }

  void update() override {
    if (!network::is_connected()) {
      this->publish_state(false);
      return;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(ip_str_.c_str());
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(53); // Test DNS Port

    int sock =  ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        this->publish_state(false);
        return;
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    ::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    int err = ::connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGW(TAG, "Internet presence test failed (TCP 53) to %s", ip_str_.c_str());
        this->publish_state(false);
    } else {
        ESP_LOGD(TAG, "Internet presence confirmed");
        this->publish_state(true);
    }
    ::close(sock);
  }

  void dump_config() override {
    ESP_LOGCONFIG(TAG, "Mspa Ping Radar:");
    ESP_LOGCONFIG(TAG, "  Target IP: %s", ip_str_.c_str());
    LOG_BINARY_SENSOR("  ", "Binary Sensor", this);
  }

 protected:
  std::string ip_str_;
};

}  // namespace mspa_ping
}  // namespace esphome
