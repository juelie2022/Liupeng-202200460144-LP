add_library(sm4_basic
    sm4_basic.c
)

target_include_directories(sm4_basic PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 安装规则
install(TARGETS sm4_basic
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)