set(src_files 
    "src/spo2_algorithm.cpp"
    "src/MAX30105.cpp"
    "src/heartRate.cpp" 
)

idf_component_register(SRCS ${src_files}
                    INCLUDE_DIRS "src" 
                    )