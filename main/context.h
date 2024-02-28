
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "driver/gpio.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_event.h"
#include "driver/touch_pad.h"
#include <stdlib.h>
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "esp_check.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "ping/ping_sock.h"

#include "lwip/sockets.h"
#include "esp_netif.h"

#include "checksum_handler.h"
#include "bluetooth_ota/bluetooth_ota_handler.h"
#include "bluetooth_ota/ota_handler.h"
#include "bluetooth_ota/flash_boot_handler.h"


#undef BLUETOOTH_DB