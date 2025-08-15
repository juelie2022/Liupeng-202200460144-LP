add_library(sm4_gcm
    sm4_gcm.c
)

target_include_directories(sm4_gcm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(sm4_gcm
    sm4_basic
)

# 安装规则
install(TARGETS sm4_gcm
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)