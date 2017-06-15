#ifndef _ESP8266MQTTMESH_H_
#define _ESP8266MQTTMESH_H_

#if  ! defined(ESP8266MESHMQTT_DISABLE_OTA)
    //By default we support OTA
    #if ! defined(MQTT_MAX_PACKET_SIZE) || MQTT_MAX_PACKET_SIZE < (1024+128)
        #error "Must define MQTT_MAX_PACKET_SIZE >= 1152"
    #endif
    #define HAS_OTA 1
#else
    #define HAS_OTA 0
#endif

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "cbuf.h"
#include <AsyncMqttClient.h>
#include <FS.h>
#include <functional>

#define TOPIC_LEN 64

#define DEBUG_EXTRA         0x10000000
#define DEBUG_MSG           0x00000001
#define DEBUG_MSG_EXTRA     (DEBUG_EXTRA | DEBUG_MSG)
#define DEBUG_WIFI          0x00000002
#define DEBUG_WIFI_EXTRA    (DEBUG_EXTRA | DEBUG_WIFI)
#define DEBUG_MQTT          0x00000004
#define DEBUG_MQTT_EXTRA    (DEBUG_EXTRA | DEBUG_MQTT)
#define DEBUG_OTA           0x00000008
#define DEBUG_OTA_EXTRA     (DEBUG_EXTRA | DEBUG_OTA)
#define DEBUG_TIMING        0x00000010
#define DEBUG_TIMING_EXTRA  (DEBUG_EXTRA | DEBUG_TIMING)
#define DEBUG_FS            0x00000020
#define DEBUG_FS_EXTRA      (DEBUG_EXTRA | DEBUG_OTA)
#define DEBUG_ALL           0xFFFFFFFF
#define DEBUG_NONE          0x00000000

#ifndef ESP8266_NUM_CLIENTS
  #define ESP8266_NUM_CLIENTS 4
#endif

typedef struct {
    unsigned int len;
    byte         md5[16];
} ota_info_t;

typedef struct {
    char bssid[19];
    int  ssid_idx;
    int  rssi;
} ap_t;

class ESP8266MQTTMesh {

private:
    unsigned int firmware_id;
    const char   *firmware_ver;
    const char   **networks;
    const char   *network_password;
    const char   *mesh_password;
    const char   *base_ssid;
    const char   *mqtt_server;
    const int    mqtt_port;
    const int    mesh_port;
    const char   *inTopic;
    const char   *outTopic;
#if HAS_OTA
    uint32_t freeSpaceStart;
    uint32_t freeSpaceEnd;
#endif
    cbuf       ringBuf;
    WiFiServer espServer;
    WiFiClient espClient[ESP8266_NUM_CLIENTS+1];
    uint8      espMAC[ESP8266_NUM_CLIENTS+1][6];
    AsyncMqttClient mqttClient;

    bool send_InProgress;
    int send_CurrentIdx;
    int send_Pos;
    ap_t ap[5];
    int ap_idx = 0;
    char mySSID[16];
    char readData[ESP8266_NUM_CLIENTS+1][MQTT_MAX_PACKET_SIZE];
    char buffer[MQTT_MAX_PACKET_SIZE];
    long lastMsg = 0;
    char msg[50];
    int value = 0;
    bool meshConnect = false;
    unsigned long lastReconnect = 0;
    unsigned long lastStatus = 0;
    bool connecting = 0;
    bool AP_ready = false;
    std::function<void(const char *topic, const char *msg)> callback;

    bool wifiConnected() { return (WiFi.status() == WL_CONNECTED); }
    void die() { while(1) {} }

    bool match_bssid(const char *bssid);
    void scan();
    void connect();
    void connect_mqtt();
    void shutdown_AP();
    void setup_AP();
    int read_subdomain(const char *fileName);
    void assign_subdomain();
    void send_bssids(int idx);
    void handle_client_data(int idx);
    void parse_message(const char *topic, const char *msg);
    void mqtt_callback(const char* topic, const byte* payload, unsigned int length);
    bool queue_message(int index, const char *topicOrMsg, const char *msg = NULL);
    void send_messages();
    void broadcast_message(const char *msg);
    void handle_ota(const char *cmd, const char *msg);
    ota_info_t parse_ota_info(const char *str);
    bool check_ota_md5();
    bool isAPConnected(uint8 *mac);
    void getMAC(IPAddress ip, uint8 *mac);

    WiFiEventHandler wifiConnectHandler;
    void onWifiConnect(const WiFiEventStationModeGotIP& event);
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    void onMqttUnsubscribe(uint16_t packetId);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void onMqttPublish(uint16_t packetId);

public:
    ESP8266MQTTMesh(unsigned int firmware_id, const char *firmware_ver,
                    const char **networks, const char *network_password, const char *mesh_password,
                    const char *base_ssid, const char *mqtt_server, int mqtt_port, int mesh_port,
                    const char *inTopic, const char *outTopic);
    void setCallback(std::function<void(const char *topic, const char *msg)> _callback);
    void begin();
    void loop();
    void publish(const char *subtopic, const char *msg);
    bool connected();
    static bool keyValue(const char *data, char separator, char *key, int keylen, const char **value);
};
#endif //_ESP8266MQTTMESH_H_
