add_library(sm4_modern_inst
    sm4_modern_inst.c
)

target_include_directories(sm4_modern_inst PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 检查GFNI和其他现代指令集支持
if(HAVE_GFNI AND ENABLE_GFNI)
    target_compile_definitions(sm4_modern_inst PRIVATE -DHAVE_GFNI=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(sm4_modern_inst PRIVATE -mgfni -mavx512vl -mavx512bw -mavx512dq)
    endif()
else()
    target_compile_definitions(sm4_modern_inst PRIVATE -DHAVE_GFNI=0)
endif()

# 安装规则
install(TARGETS sm4_modern_inst
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)