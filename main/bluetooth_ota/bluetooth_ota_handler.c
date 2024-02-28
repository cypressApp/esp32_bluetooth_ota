#include "context.h"

int  bluetooth_rec_data_counter = 0;
bool update_firmware_mode = false;
int  update_firmware_buffer_size = 0;

int  update_firmware_timeout_counter = 0;
bool new_update_firmware_data_received = false;
bool stop_firmware_timeout = false;

void process_bluetooth_data(esp_spp_cb_param_t *param , unsigned char *rx_buffer , int rx_buffer_len){

#ifdef BLUETOOTH_DB    
    printf("%s\n" , rx_buffer);
#endif

    // Select Write Partition
    if(rx_buffer[0] == SELECT_OTA_WRITE_PARTITION_HEADER && rx_buffer[5] == SELECT_OTA_WRITE_PARTITION_FOOTER){
        bt_ota_select_write_partition(param , rx_buffer);       
    }
    // Select Read Partition
    else if(rx_buffer[0] == SELECT_OTA_READ_PARTITION_HEADER && rx_buffer[5] == SELECT_OTA_READ_PARTITION_FOOTER){
        bt_ota_select_read_partition(param , rx_buffer);       
    }
    // Select Erase Partition
    else if(rx_buffer[0] == SELECT_OTA_ERASE_PARTITION_HEADER && rx_buffer[5] == SELECT_OTA_ERASE_PARTITION_FOOTER){
        bt_ota_select_erase_partition(param , rx_buffer);        
    }
    // Select Erase Write Partition
    else if(rx_buffer[0] == SELECT_OTA_ERASE_WRITE_PARTITION_HEADER && rx_buffer[5] == SELECT_OTA_ERASE_WRITE_PARTITION_FOOTER){       
        bt_ota_select_erase_write_partition(param , rx_buffer);        
    }
    // Write Partition Data
    else if(rx_buffer[0] == OTA_WRITE_TO_PARTITION_HEADER){//  && rx_buffer[rx_buffer[1] - 3] == OTA_WRITE_TO_PARTITION_FOOTER){
        bt_ota_write_to_partition(0 , rx_buffer);
    }  
    // Read Partition Data
    else if(rx_buffer[0] == OTA_READ_FROM_PARTITION_HEADER  && rx_buffer[5] == OTA_READ_FROM_PARTITION_FOOTER){
        bt_ota_read_from_partition(param , rx_buffer);        
    }   
    // Erase Partition Data
    else if(rx_buffer[0] == OTA_ERASE_PARTITION_HEADER  && rx_buffer[5] == OTA_ERASE_PARTITION_FOOTER){
        bt_ota_erase_partition(param , rx_buffer);        
    }  
    // Erase Write Partition Data
    else if(rx_buffer[0] == OTA_ERASE_WRITE_PARTITION_HEADER){//  && rx_buffer[rx_buffer[1] - 3] == OTA_ERASE_WRITE_PARTITION_FOOTER){
        
        int data_size = rx_buffer[2];
        if(rx_buffer[1] != 0xFF){
            data_size = ((rx_buffer[1] * 256) + rx_buffer[2]);
        }

        bt_ota_erase_write_partition(param , rx_buffer , data_size);     
    } 
    // Start Update Firmware
    else if(rx_buffer[0] == OTA_START_UPDATE_FIRMWARE_HEADER){// && rx_buffer[8] == OTA_START_UPDATE_FIRMWARE_FOOTER){
        bt_ota_start_update_firmware(param , rx_buffer);         
    }
    // End Update Firmware
    else if(rx_buffer[0] == OTA_END_UPDATE_FIRMWARE_HEADER){// && rx_buffer[5] == OTA_END_UPDATE_FIRMWARE_FOOTER){
        
        int data_size = rx_buffer[2];
        if(rx_buffer[1] != 0xFF){
            data_size = ((rx_buffer[1] * 256) + rx_buffer[2]);
        }
        bt_ota_end_update_firmware(param , rx_buffer , data_size);

    }
    // Get Firmware Version
    else if(rx_buffer[0] == GET_HARDWARE_FIRMWARE_VERSION_HEADER && rx_buffer[5] == GET_HARDWARE_FIRMWARE_VERSION_FOOTER){
        bt_ota_get_hardware_firmware_version(param , rx_buffer);        
    }
    // Cancel Update Firmware
    else if(rx_buffer[0] == OTA_CANCEL_UPDATE_FIRMWARE_HEADER && rx_buffer[5] == OTA_CANCEL_UPDATE_FIRMWARE_FOOTER){
        bt_ota_cancel_update_firmware(param , rx_buffer);        
    }
    // Set Partition
    else if(rx_buffer[0] == OTA_SET_BOOT_PARTITION_HEADER){// && rx_buffer[5] == OTA_END_UPDATE_FIRMWARE_FOOTER){
        bt_ota_set_boot_partition(param , rx_buffer);        
    }
}

void update_firmware_timeout(){

    update_firmware_timeout_counter = 0;
    stop_firmware_timeout = false;
    while (update_firmware_timeout_counter < 50 && !stop_firmware_timeout){
        if(new_update_firmware_data_received){
            new_update_firmware_data_received = false;
            update_firmware_timeout_counter = 0;
        }else{
            update_firmware_timeout_counter++;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }

#ifdef BLUETOOTH_DB 
    printf("Timeout\r\n");
#endif

    bluetooth_rec_data_counter = 0;
    update_firmware_mode = false;
    vTaskDelete(NULL);

}

void bluetooth_ota_response(esp_spp_cb_param_t *param , char *response , int response_length , char header , char footer){
	
    char send_bt_data[128];
	
	send_bt_data[0] = OTA_RESPONSE_HEADER;
    long int temp_sum = send_bt_data[0];
    int i = 0;
    for(i = 0 ; i < response_length ; i++){
        send_bt_data[i + 1] = response[i];
        temp_sum += send_bt_data[i + 1];       
    }
	
	send_bt_data[i + 1] = OTA_RESPONSE_FOOTER;
    temp_sum += footer;
	send_bt_data[i + 2] = (temp_sum) & 0xFF;
	send_bt_data[i + 3] = 0x0D;
	send_bt_data[i + 4] = 0x0A;
    
    esp_spp_write(param->write.handle , strlen(send_bt_data) , (uint8_t *) send_bt_data);

}


void bt_ota_select_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }

    new_update_firmware_data_received = true;

    ota_set_new_write_partition(rx_buffer[2] , rx_buffer[4]);            

    bluetooth_ota_response(param , BLUETOOTH_OTA_SELECET_OTA_WRITE_RESPONSE , 6 ,
                           SELECT_OTA_WRITE_PARTITION_HEADER , SELECT_OTA_WRITE_PARTITION_FOOTER);
                

}

void bt_ota_select_erase_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){
    
    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }
    new_update_firmware_data_received = true;

    ota_set_new_erase_partition(rx_buffer[2] , rx_buffer[4]);            

    bluetooth_ota_response(param , BLUETOOTH_OTA_SELECET_OTA_ERASE_RESPONSE , 6 , 
                                SELECT_OTA_ERASE_PARTITION_HEADER , SELECT_OTA_ERASE_PARTITION_FOOTER);
            
}

void bt_ota_select_read_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }
    new_update_firmware_data_received = true;

    ota_set_new_read_partition(rx_buffer[2] , rx_buffer[4]);            

    bluetooth_ota_response(param , BLUETOOTH_OTA_SELECET_OTA_READ_RESPONSE , 6, 
                                SELECT_OTA_READ_PARTITION_HEADER , SELECT_OTA_READ_PARTITION_FOOTER);
            
}

void bt_ota_select_erase_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }
    new_update_firmware_data_received = true;

#ifdef BLUETOOTH_DB 
    printf("bt_ota_select_erase_write_partition\r\n");
#endif        
           
    char temp_type = rx_buffer[2];
    char temp_subtype = rx_buffer[4];

    if(temp_type == 0x30){
        temp_type -= 0x30;
    }
    if(temp_subtype == 0x30){
        temp_subtype -= 0x30;
    }

    ota_set_new_erase_write_partition(temp_type , temp_subtype);    
          
    bluetooth_ota_response(param , BLUETOOTH_OTA_SELECET_OTA_ERASE_WRITE_RESPONSE , 6, 
                                SELECT_OTA_ERASE_WRITE_PARTITION_HEADER , SELECT_OTA_ERASE_WRITE_PARTITION_FOOTER);
            
}


void bt_ota_write_to_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    new_update_firmware_data_received = true;
    ota_write(rx_buffer[2] , (char *) rx_buffer , 256);            

}

void bt_ota_read_from_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    new_update_firmware_data_received = true;
    ota_read(rx_buffer[2] , (char *) rx_buffer , 256);            

}

void bt_ota_erase_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    new_update_firmware_data_received = true;
    ota_erase(rx_buffer[2] , 256);            

}

void bt_ota_erase_write_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer , long int data_size){

#ifdef BLUETOOTH_DB         
        printf("erase_write %ld %d %d %d\r\n" , data_size , rx_buffer[data_size - 3] , rx_buffer[data_size - 2] , rx_buffer[data_size - 1]);
#endif

    if(!is_checksum_correct(rx_buffer , 0 , data_size - 3 , data_size - 2)){
        return;
    }

    new_update_firmware_data_received = true;

#ifdef BLUETOOTH_DB 
        printf("Data size: %ld\r\n" , data_size);
#endif        

    long int temp_offset = 0;
    for(int i = 3 ; i < 11 ; i++){
        if(rx_buffer[i] >= 0x30 && rx_buffer[i] <= 0x39){
            temp_offset += (rx_buffer[i] - 0x30);
            temp_offset *= 10;
        }else{
#ifdef BLUETOOTH_DB                 
            printf("Error: Address offset\r\n");
#endif                
            return;
        }
    }
    temp_offset /= 10;
#ifdef BLUETOOTH_DB         
    printf("Address offset: %ld\r\n" , temp_offset);
#endif
    ota_erase_write(temp_offset , (char *) (rx_buffer + 11) , data_size - 14);

    bluetooth_ota_response(param , BLUETOOTH_OTA_ERASE_WRITE_RESPONSE , 6, 
                                OTA_ERASE_WRITE_PARTITION_HEADER , OTA_ERASE_WRITE_PARTITION_FOOTER);
            
}

void bt_ota_start_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer){
    
    new_update_firmware_data_received = true;

    last_erase_offset = 0;

    long int temp_update_firmware_buffer_size = 0;
    for(int i = 1 ; i < 8 ; i++){
        if(rx_buffer[i] >= 0x30 && rx_buffer[i] <= 0x39){
            temp_update_firmware_buffer_size += (rx_buffer[i] - 0x30);
            temp_update_firmware_buffer_size *= 10;
        }else{
#ifdef BLUETOOTH_DB                     
            printf("Error: Firmware Buffer Size\r\n");
#endif                    
            return;
        }
    }

    int  temp_username_len = 0;
    char temp_username[256] = {0};
    for(int i = 8 ; i < 256 ; i++){
        if(rx_buffer[i] == 0x20){
            break;
        }else{
            temp_username[temp_username_len] = rx_buffer[i];
            temp_username_len++;
        }
    }
#ifdef BLUETOOTH_DB        
    printf("Username: %s %d %d\n" , temp_username , temp_username_len , strlen(OTA_USERNAME));
#endif
    int  temp_password_len = 0;
    char temp_password[256] = {0};
    for(int i = (temp_username_len + 1 + 8)  ; i < temp_username_len + 1 + 8 + 256 ; i++){
        if(rx_buffer[i] == 0x20){
            break;
        }else{
            temp_password[temp_password_len] = rx_buffer[i];
            temp_password_len++;
        }
    }
#ifdef BLUETOOTH_DB        
    printf("Password: %s %d %d\n" , temp_password , temp_password_len , strlen(OTA_PASSWORD));
#endif

#ifdef CHECK_OTA_CREDENTIAL

    if(strlen(OTA_USERNAME) != temp_username_len || strlen(OTA_PASSWORD) != temp_password_len ){
        
        bluetooth_ota_response(param , BLUETOOTH_OTA_ERROR_UPDATE_FIRMWARE_RESPONSE , 6 , 
                OTA_ERROR_UPDATE_FIRMWARE_HEADER , OTA_ERROR_UPDATE_FIRMWARE_FOOTER);

        return;
    }

    if(memcmp(temp_username , OTA_USERNAME , temp_username_len) != 0 ||
            memcmp(temp_password , OTA_PASSWORD , temp_password_len) != 0){
            
        bluetooth_ota_response(param , BLUETOOTH_OTA_ERROR_UPDATE_FIRMWARE_RESPONSE , 6 , 
                OTA_ERROR_UPDATE_FIRMWARE_HEADER , OTA_ERROR_UPDATE_FIRMWARE_FOOTER);

        return;
    }

#endif

    temp_update_firmware_buffer_size /= 10;
    update_firmware_buffer_size = temp_update_firmware_buffer_size;
    update_firmware_mode = true;
    xTaskCreate(update_firmware_timeout, "update_firmware_timeout", 1024, (void*)AF_INET, 5, NULL);

#ifdef BLUETOOTH_DB             
    printf("Firmware Buffer Size: %d\r\n" , update_firmware_buffer_size);
#endif
    bluetooth_ota_response(param , BLUETOOTH_OTA_START_UPDATE_FIRMWARE_RESPONSE , 6 , 
                                OTA_START_UPDATE_FIRMWARE_HEADER , OTA_START_UPDATE_FIRMWARE_FOOTER);
            
}

void bt_ota_end_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer , long int data_size){

#ifdef BLUETOOTH_DB        
    printf("bt_ota_end_update_firmware\r\n");
#endif

    if(!is_checksum_correct(rx_buffer , 0 , data_size - 3 , data_size - 2)){
        return;
    }

    new_update_firmware_data_received = true;
    stop_firmware_timeout = true;

    flash_erase_write_boot_partition(rx_buffer[11]);
    
    update_firmware_buffer_size = 0;
    update_firmware_mode = false;

    bluetooth_ota_response(param , BLUETOOTH_OTA_END_UPDATE_FIRMWARE_RESPONSE , 6 , 
                            OTA_END_UPDATE_FIRMWARE_HEADER , OTA_END_UPDATE_FIRMWARE_FOOTER);


    xTaskCreate(restart_esp_task, "restart_esp_task", 1024, (void*)AF_INET, 5, NULL);


}

void bt_ota_set_boot_partition(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

#ifdef BLUETOOTH_DB        
    printf("OTA set boot partition\r\n");
#endif


    int  temp_username_len = 0;
    char temp_username[256] = {0};
    for(int i = 1 ; i < 256 + 1 ; i++){
        if(rx_buffer[i] == 0x20){
            break;
        }else{
            temp_username[temp_username_len] = rx_buffer[i];
            temp_username_len++;
        }
    }
#ifdef BLUETOOTH_DB        
    printf("Username: %s %d %d\n" , temp_username , temp_username_len , strlen(OTA_USERNAME));
#endif
    int  temp_password_len = 0;
    char temp_password[256] = {0};
    for(int i = (temp_username_len + 2)  ; i < temp_username_len + 2 + 256 ; i++){
        if(rx_buffer[i] == 0x20){
            break;
        }else{
            temp_password[temp_password_len] = rx_buffer[i];
            temp_password_len++;
        }
    }
#ifdef BLUETOOTH_DB        
    printf("Password: %s %d %d\n" , temp_password , temp_password_len , strlen(OTA_PASSWORD));
#endif

#ifdef CHECK_OTA_CREDENTIAL

    if(strlen(OTA_USERNAME) != temp_username_len || strlen(OTA_PASSWORD) != temp_password_len ){
        
        bluetooth_ota_response(param , BLUETOOTH_OTA_ERROR_UPDATE_FIRMWARE_RESPONSE , 6 , 
                OTA_ERROR_UPDATE_FIRMWARE_HEADER , OTA_ERROR_UPDATE_FIRMWARE_FOOTER);

        return;
    }

    if(memcmp(temp_username , OTA_USERNAME , temp_username_len) != 0 ||
            memcmp(temp_password , OTA_PASSWORD , temp_password_len) != 0){
            
        bluetooth_ota_response(param , BLUETOOTH_OTA_ERROR_UPDATE_FIRMWARE_RESPONSE , 6 , 
                OTA_ERROR_UPDATE_FIRMWARE_HEADER , OTA_ERROR_UPDATE_FIRMWARE_FOOTER);

        return;
    }

#endif

    flash_erase_write_boot_partition(rx_buffer[1 + temp_username_len + 1 + temp_password_len + 1]);
    
    update_firmware_buffer_size = 0;
    update_firmware_mode = false;

    bluetooth_ota_response(param , BLUETOOTH_OTA_SET_BOOT_PARTITION_RESPONSE , 6 , 
                            OTA_SET_BOOT_PARTITION_HEADER , OTA_SET_BOOT_PARTITION_FOOTER);

    xTaskCreate(restart_esp_task, "restart_esp_task", 1024, (void*)AF_INET, 5, NULL);

}

void bt_ota_get_hardware_firmware_version(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }

    new_update_firmware_data_received = true;


    char  temp_response[64] = {0};

    int len = sprintf(temp_response , "%s%s,%s" , BLUETOOTH_FW_HW_VERSION_RESPONSE , FIRMWARE_VERSION , HARDWARE_VERSION);

    bluetooth_ota_response(param , temp_response , len , 
                                GET_HARDWARE_FIRMWARE_VERSION_HEADER , GET_HARDWARE_FIRMWARE_VERSION_FOOTER);
            

}

void bt_ota_cancel_update_firmware(esp_spp_cb_param_t *param , unsigned char *rx_buffer){

    if(!is_checksum_correct(rx_buffer , 0 , 5 , 6)){
        return;
    }
    stop_firmware_timeout = true;
    bluetooth_rec_data_counter = 0;
    update_firmware_mode = false;

    bluetooth_ota_response(param , BLUETOOTH_OTA_CANCEL_UPDATE_FIRMWARE_RESPONSE , 6 , 
                            OTA_CANCEL_UPDATE_FIRMWARE_HEADER , OTA_CANCEL_UPDATE_FIRMWARE_FOOTER);
            

}

void restart_esp_task(void *pvParameter){

    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();

}