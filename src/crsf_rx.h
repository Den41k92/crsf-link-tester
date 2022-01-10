

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define CRSF_BAUDRATE	420000

enum { CRSF_SYNC_BYTE = 0xC8 };

enum { CRSF_FRAME_SIZE_MAX = 64 }; // 62 bytes frame plus 2 bytes frame header(<length><type>)
enum { CRSF_PAYLOAD_SIZE_MAX = CRSF_FRAME_SIZE_MAX - 6 };

typedef enum {
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    // Extended Header Frames, range: 0x28 to 0x96
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
    CRSF_FRAMETYPE_COMMAND = 0x32,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C,  // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
    CRSF_FRAMETYPE_DISPLAYPORT_CMD = 0x7D, // displayport control command
} crsf_frame_type_e;

enum {
    CRSF_DISPLAYPORT_SUBCMD_UPDATE = 0x01, // transmit displayport buffer to remote
    CRSF_DISPLAYPORT_SUBCMD_CLEAR = 0X02, // clear client screen
    CRSF_DISPLAYPORT_SUBCMD_OPEN = 0x03,  // client request to open cms menu
    CRSF_DISPLAYPORT_SUBCMD_CLOSE = 0x04,  // client request to close cms menu
    CRSF_DISPLAYPORT_SUBCMD_POLL = 0x05,  // client request to poll/refresh cms menu
};

enum {
    CRSF_DISPLAYPORT_OPEN_ROWS_OFFSET = 1,
    CRSF_DISPLAYPORT_OPEN_COLS_OFFSET = 2,
};

enum {
    CRSF_FRAME_GPS_PAYLOAD_SIZE = 15,
    CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE = 8,
    CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE = 10,
    CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE = 22, // 11 bits per channel * 16 channels = 22 bytes.
    CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE = 6,
};

enum {
    CRSF_FRAME_LENGTH_ADDRESS = 1, // length of ADDRESS field
    CRSF_FRAME_LENGTH_FRAMELENGTH = 1, // length of FRAMELENGTH field
    CRSF_FRAME_LENGTH_TYPE = 1, // length of TYPE field
    CRSF_FRAME_LENGTH_CRC = 1, // length of CRC field
    CRSF_FRAME_LENGTH_TYPE_CRC = 2, // length of TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_EXT_TYPE_CRC = 4, // length of Extended Dest/Origin, TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_NON_PAYLOAD = 4, // combined length of all fields except payload
};

enum {
    CRSF_FRAME_TX_MSP_FRAME_SIZE = 58,
    CRSF_FRAME_RX_MSP_FRAME_SIZE = 8,
    CRSF_FRAME_ORIGIN_DEST_SIZE = 2,
};

typedef struct crsf_payload_link_statistics_t {
	uint8_t uplink_RSSI_1;
	uint8_t uplink_RSSI_2;
	uint8_t uplink_Link_quality;
	int8_t uplink_SNR;
	uint8_t active_antenna;
	uint8_t rf_Mode;
	uint8_t uplink_TX_Power;
	uint8_t downlink_RSSI;
	uint8_t downlink_Link_quality;
	int8_t downlink_SNR;
};



// void CRSF_RX_begin(crsf_telemetry_aux_t * telemetry = nullptr);
void CRSF_RX_begin();
void CRSF_RX_onReceiveLinkStatistics(void (*callback)(crsf_payload_link_statistics_t * link_info));
void CRSF_RX_loop();
int CRSF_txPowerToMilliwatts(uint8_t n);
