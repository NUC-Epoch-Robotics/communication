#ifndef CHASSIS_SERIAL_DRIVER__PACKET_HPP_
#define CHASSIS_SERIAL_DRIVER__PACKET_HPP_

#include <algorithm>
#include <cstdint>
#include <vector>

namespace chassis_serial_driver
{
struct ReceivePacket
{
  uint8_t frame_header[2];
  uint8_t linear_x;
  uint8_t linear_y;
  uint8_t linear_z;
  uint8_t angular_x;
  uint8_t angular_y;
  uint8_t angular_z;
  
} __attribute__((packed));

struct SendPacket
{ 
  uint8_t frame_header[2] = {0xAA, 0x55};
  float linear_x;
  float linear_y;
  float linear_z;
  float angular_x;
  float angular_y;
  float angular_z;
  uint8_t frame_end={0xcc};
  
} __attribute__((packed));

inline ReceivePacket fromVector(const std::vector<uint8_t> & data)
{
  ReceivePacket packet;
  std::copy(data.begin(), data.end(), reinterpret_cast<uint8_t *>(&packet));
  return packet;
}

inline std::vector<uint8_t> toVector(const SendPacket & data)
{
  std::vector<uint8_t> packet(sizeof(SendPacket));
  std::copy(
    reinterpret_cast<const uint8_t *>(&data),
    reinterpret_cast<const uint8_t *>(&data) + sizeof(SendPacket), packet.begin());
  return packet;
}


}  // namespace chassis_serial_driver

#endif  // RM_SERIAL_DRIVER__PACKET_HPP_
