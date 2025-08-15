add_library(sm4_t_table
    sm4_t_table.c
)

target_include_directories(sm4_t_table PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 安装规则
install(TARGETS sm4_t_table
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)