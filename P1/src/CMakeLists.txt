add_subdirectory(basic)
add_subdirectory(t_table)
add_subdirectory(aesni)
add_subdirectory(modern_inst)
add_subdirectory(gcm)

# 创建主库，包含所有实现
add_library(sm4_all INTERFACE)
target_link_libraries(sm4_all INTERFACE
    sm4_basic
    sm4_t_table
    sm4_aesni
    sm4_modern_inst
    sm4_gcm
)

# 安装规则
install(TARGETS sm4_all EXPORT sm4_all_targets)
install(EXPORT sm4_all_targets
    FILE sm4_all_targets.cmake
    NAMESPACE SM4::
    DESTINATION lib/cmake/sm4_opt
)