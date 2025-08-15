add_executable(sm4_gcm_example
    sm4_gcm_example.c
)

target_include_directories(sm4_gcm_example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sm4_gcm_example
    sm4_gcm
)

install(TARGETS sm4_gcm_example
    RUNTIME DESTINATION bin
)