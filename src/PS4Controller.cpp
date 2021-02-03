#include "PS4Controller.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_bt_main.h>
#include <esp_bt_defs.h>
#include <cstring>
#include <esp_log.h>
#include <cstdio>

#define NO_GLOBAL_INSTANCES

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CONSTRAIN_8BIT(a) MIN(MAX(a, 0), 255)
#define ESP_BD_ADDR_HEX_PTR(addr) (unsigned int*)addr+0, (unsigned int*)addr+1, (unsigned int*)addr+2, (unsigned int*)addr+3, (unsigned int*)addr+4, (unsigned int*)addr+5

extern "C" {
  #include  "ps4.h"
}

PS4Controller::PS4Controller() = default;

bool PS4Controller::begin() {
    ps4SetEventObjectCallback(this, &PS4Controller::_event_callback);
    ps4SetConnectionObjectCallback(this, &PS4Controller::_connection_callback);

    /*
    if(!btStarted() && !btStart()){
        ESP_LOGE("DS4","btStart failed");
        return false;
    }
    */

    esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
    if(bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED){
        if (esp_bluedroid_init()) {
            ESP_LOGE("DS4","esp_bluedroid_init failed");
            return false;
        }
    }

    if(bt_state != ESP_BLUEDROID_STATUS_ENABLED){
        if (esp_bluedroid_enable()) {
            ESP_LOGE("DS4","esp_bluedroid_enable failed");
            return false;
        }
    }

    ps4Init();
    return true;
}


bool PS4Controller::begin(char *mac) {
    esp_bd_addr_t addr;

    if (std::sscanf(mac, ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX_PTR(addr)) != ESP_BD_ADDR_LEN){
        ESP_LOGE("DS4","Could not convert %s\n to a MAC address", mac);
        return false;
    }

    ps4SetBluetoothMacAddress( addr );

    return begin();
}


bool PS4Controller::end() {
    return true;
}


bool PS4Controller::isConnected() {
    return ps4IsConnected();
}


void PS4Controller::setLed(int r, int g, int b) {
    output.r = CONSTRAIN_8BIT(r);
    output.g = CONSTRAIN_8BIT(g);
    output.b = CONSTRAIN_8BIT(b);
}


void PS4Controller::setRumble(int small, int large) {
    output.smallRumble = CONSTRAIN_8BIT(small);
    output.largeRumble = CONSTRAIN_8BIT(large);
}


void PS4Controller::setFlashRate(int onTime, int offTime) {
    output.flashOn = CONSTRAIN_8BIT(onTime/10);
    output.flashOff = CONSTRAIN_8BIT(offTime/10);
}


void PS4Controller::sendToController() {
    ps4SetOutput(output);
}



void PS4Controller::attach(callback_t callback) {
    _callback_event = callback;
}


void PS4Controller::attachOnConnect(callback_t callback) {
    _callback_connect = callback;

}

void PS4Controller::attachOnDisconnect(callback_t callback) {
    _callback_disconnect = callback;
}

void PS4Controller::_event_callback(void *object, ps4_t data, ps4_event_t event) {
    auto* This = (PS4Controller*) object;

    memcpy(&This->data, &data, sizeof(ps4_t));
    memcpy(&This->event, &event, sizeof(ps4_event_t));

    if (This->_callback_event){
        This->_callback_event();
    }
}


void PS4Controller::_connection_callback(void *object, uint8_t is_connected) {
    auto* This = (PS4Controller*) object;

    if (is_connected) {
        vTaskDelay(250);    // ToDo: figure out how to know when the channel is free again so this delay can be removed

        if (This->_callback_connect){
            This->_callback_connect();
        }
    }
    else {
        if (This->_callback_disconnect){
            This->_callback_disconnect();
        }
    }
}

#if !defined(NO_GLOBAL_INSTANCES)
  PS4Controller PS4;
#endif
