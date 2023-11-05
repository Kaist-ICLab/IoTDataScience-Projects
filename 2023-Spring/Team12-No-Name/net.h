#ifndef NET_H_
#define NET_H_

typedef struct gps_t { //type 7 data
  uint32_t type_ms;
  float latitude;
  float longitude;
  uint16_t lat;
  uint16_t lon;
};

constexpr int gps_union_size = sizeof(gps_t);

constexpr int big_union_size = gps_union_size;

typedef union gpsPacket_t {
  gps_t structure;
  byte byteArray[big_union_size] = {0};
};

//Endianness functions
uint16_t htons(uint16_t hostshort) {
  return ((hostshort & 0xff00) >> 8) | ((hostshort & 0x00ff) << 8);
}

uint32_t htonl(uint32_t hostlong) {
  return ((hostlong & 0xff000000) >> 24) | ((hostlong & 0x00ff0000) >> 8) | ((hostlong & 0x0000ff00) << 8) | ((hostlong & 0x000000ff) << 24);
}

float htonf(float value) {
    union {
        float f;
        uint8_t bytes[sizeof(float)];
    } in = {value}, out;

    for (size_t i = 0; i < sizeof(float); ++i) {
        out.bytes[i] = in.bytes[sizeof(float) - i - 1];
    }

    return out.f;
}

#endif  // NET_H_