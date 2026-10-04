idf_component_register(SRCS "${CMAKE_CURRENT_LIST_DIR}/../../../../src/brc.c"
                       INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../../../include")
target_compile_options(${COMPONENT_LIB} PRIVATE -std=c17 ${HW_OPT} -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -Wshadow -Wundef)
