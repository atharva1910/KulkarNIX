ADD_LIBRARY(KLibHAL
	    Src/PIC.cpp
	    Src/Interrupts.cpp
	    Src/x86.cpp
            Src/Paging.cpp)

TARGET_INCLUDE_DIRECTORIES(KLibHAL PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Inc ${CMAKE_SOURCE_DIR}/Inc ${KNIX_ARCH_INCLUDE})
