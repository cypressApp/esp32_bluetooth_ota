idf_component_register(SRCS 
"bluetooth_ota/bluetooth_ota_handler.c"
"bluetooth_ota/ota_handler.c"
"bluetooth_ota/flash_boot_handler.c"
"checksum_handler.c" 
"main.c"
                    INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
