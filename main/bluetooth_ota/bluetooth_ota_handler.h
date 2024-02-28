#include "stdbool.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#define CHECK_OTA_CREDENTIAL 
#define OTA_USERNAME  "Cypress"
#define OTA_PASSWORD  "123"

#define SELECT_OTA_WRITE_PARTITION_HEADER         0x84                       
#define SELECT_OTA_WRITE_PARTITION_FOOTER         0x0F                       
#define SELECT_OTA_ERASE_PARTITION_HEADER         0x87                       
#define SELECT_OTA_ERASE_PARTITION_FOOTER         0x0F                       
#define SELECT_OTA_READ_PARTITION_HEADER          0x89                       
#define SELECT_OTA_READ_PARTITION_FOOTER          0x0F                       
#define SELECT_OTA_ERASE_WRITE_PARTITION_HEADER   0x91                       
#define SELECT_OTA_ERASE_WRITE_PARTITION_FOOTER   0x0F                       
#define OTA_WRITE_TO_PARTITION_HEADER             0x8A                       
#define OTA_WRITE_TO_PARTITION_FOOTER             0x0F                       
#define OTA_READ_FROM_PARTITION_HEADER            0x8C                       
#define OTA_READ_FROM_PARTITION_FOOTER            0x0F                       
#define OTA_ERASE_PARTITION_HEADER                0x8D                       
#define OTA_ERASE_PARTITION_FOOTER                0x0F                       
#define OTA_ERASE_WRITE_PARTITION_HEADER          0x93                       
#define OTA_ERASE_WRITE_PARTITION_FOOTER          0x0F                       
#define OTA_END_WRITE_PARTITION_HEADER            0x8F                       
#define OTA_END_WRITE_PARTITION_FOOTER            0x0F                       
#define OTA_RESPONSE_HEADER                       0x94                       
#define OTA_RESPONSE_FOOTER                       0x0F                       
#define OTA_START_UPDATE_FIRMWARE_HEADER          0x95                       
#define OTA_START_UPDATE_FIRMWARE_FOOTER          0x0F 
#define OTA_END_UPDATE_FIRMWARE_HEADER            0x96                       
#define OTA_END_UPDATE_FIRMWARE_FOOTER            0x0F                       
#define GET_HARDWARE_FIRMWARE_VERSION_HEADER      0x8E                       
#define GET_HARDWARE_FIRMWARE_VERSION_FOOTER      0x0F 
#define OTA_ERROR_UPDATE_FIRMWARE_HEADER          0x97                       
#define OTA_ERROR_UPDATE_FIRMWARE_FOOTER          0x0F 
#define OTA_CANCEL_UPDATE_FIRMWARE_HEADER         0x98                       
#define OTA_CANCEL_UPDATE_FIRMWARE_FOOTER         0x0F 
#define OTA_SET_BOOT_PARTITION_HEADER             0x99               
#define OTA_SET_BOOT_PARTITION_FOOTER             0x0F 

#define BLUETOOTH_OTA_SELECET_OTA_WRITE_RESPONSE       "ACKS_W"
#define BLUETOOTH_OTA_SELECET_OTA_ERASE_RESPONSE       "ACKS_E"
#define BLUETOOTH_OTA_SELECET_OTA_READ_RESPONSE        "ACKS_R"
#define BLUETOOTH_OTA_SELECET_OTA_ERASE_WRITE_RESPONSE "ACKSEW"
#define BLUETOOTH_OTA_WRITE_RESPONSE                   "ACKO_W"
#define BLUETOOTH_OTA_ERASE_RESPONSE                   "ACKO_E"
#define BLUETOOTH_OTA_READ_RESPONSE                    "ACKO_R"
#define BLUETOOTH_OTA_ERASE_WRITE_RESPONSE             "ACKOEW"
#define BLUETOOTH_OTA_START_UPDATE_FIRMWARE_RESPONSE   "ACKSUF"
#define BLUETOOTH_OTA_ERROR_UPDATE_FIRMWARE_RESPONSE   "ACKERR"
#define BLUETOOTH_OTA_END_UPDATE_FIRMWARE_RESPONSE     "ACKEUF"
#define BLUETOOTH_OTA_CANCEL_UPDATE_FIRMWARE_RESPONSE  "ACKCAN"
#define BLUETOOTH_FW_HW_VERSION_RESPONSE               "ACKVER"
#define BLUETOOTH_OTA_SET_BOOT_PARTITION_RESPONSE      "ACKSBP"

#define HARDWARE_VERSION        "01.20"
#define FIRMWARE_VERSION        "01.20"

#define BOOT_PARTION_TYPE         0x59
#define BOOT_PARTITION_SUBTYPE    0x0B

extern int  update_firmware_timeout_counter;
extern bool new_update_firmware_data_received;
extern bool stop_firmware_timeout;

void process_bluetooth_data(esp_spp_cb_param_t *param , unsigned char *rx_buffer , int rx_buffer_len);
void update_firmware_timeout();
void bluetooth_ota_response(esp_spp_cb_param_t *param , char *response , int response_length , char header , char footer);
void bt_ota_select_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_select_erase_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_select_read_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_select_erase_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_write_to_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_read_from_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_erase_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_erase_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer , long int data_size);
void bt_ota_start_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_end_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer , long int data_size);
void bt_ota_set_boot_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_get_hardware_firmware_version(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void bt_ota_cancel_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer);
void restart_esp_task(void *pvParameter);