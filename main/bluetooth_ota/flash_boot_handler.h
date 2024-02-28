#include "esp_err.h"

esp_err_t flash_read_boot_partition();
esp_err_t flash_erase_boot_partition();
esp_err_t flash_erase_write_boot_partition(char boot_partition);