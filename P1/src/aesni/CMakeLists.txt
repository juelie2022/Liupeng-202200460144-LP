add_library(sm4_aesni
    sm4_aesni.c
)

target_include_directories(sm4_aesni PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
)

# 检查AES-NI支持
if(HAVE_AESNI AND ENABLE_AESNI)
    target_compile_definitions(sm4_aesni PRIVATE -DHAVE_AESNI=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(sm4_aesni PRIVATE -maes)
    endif()
else()
    target_compile_definitions(sm4_aesni PRIVATE -DHAVE_AESNI=0)
endif()

# 安装规则
install(TARGETS sm4_aesni
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)