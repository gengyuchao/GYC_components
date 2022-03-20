set(src_files 
    "src/Wire.cpp"
    "src/i2c_port.c"
    # "src/esp32-hal-misc.c"
    # "src/Stream.cpp"
    # "src/Print.cpp"
)

idf_component_register(SRCS ${src_files}
                    INCLUDE_DIRS "src" 
                    PRIV_REQUIRES esp_rom
                    )