add_executable(sm4_test
    sm4_test.c
)

target_include_directories(sm4_test PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sm4_test
    sm4_basic
    sm4_t_table
    sm4_aesni
    sm4_modern_inst
    sm4_gcm
)

add_test(NAME sm4_test COMMAND sm4_test)

install(TARGETS sm4_test
    RUNTIME DESTINATION bin
)