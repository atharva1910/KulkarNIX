ADD_LIBRARY(KLibHAL
            src/PIC.cpp
            src/Interrupts.cpp
            src/x86.cpp
            src/Paging.cpp)

TARGET_INCLUDE_DIRECTORIES(KLibHAL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/inc ${KNIX_ARCH_INCLUDE})
