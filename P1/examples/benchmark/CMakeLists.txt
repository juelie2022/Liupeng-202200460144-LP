add_executable(sm4_benchmark
    sm4_benchmark.c
)

target_include_directories(sm4_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sm4_benchmark
    sm4_basic
    sm4_t_table
    sm4_aesni
    sm4_modern_inst
    sm4_gcm
)

install(TARGETS sm4_benchmark
    RUNTIME DESTINATION bin
)