add_executable(sm4_basic_example
    sm4_basic_example.c
)

target_include_directories(sm4_basic_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sm4_basic_example
    sm4_basic
    sm4_t_table
    sm4_aesni
    sm4_modern_inst
)

install(TARGETS sm4_basic_example
    RUNTIME DESTINATION bin
)