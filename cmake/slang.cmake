##########################################################
#                      INSTALLATION                      #
##########################################################

message(STATUS "Check for working Slang compiler")

find_package(slang CONFIG QUIET)

if (slang_FOUND)
    message(STATUS "Check for working Slang compiler - done")
else ()
    message(STATUS "Check for working Slang compiler - failed")

    set(SLANG_RELEASE_TAG "2025.23.2")
    set(SLANG_RELEASE_URL "https://github.com/shader-slang/slang/releases/download/v${SLANG_RELEASE_TAG}")

    # Fetch binaries depending on the operating system
    if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SLANG_BINARIES "slang-${SLANG_RELEASE_TAG}-windows-x86_64.tar.gz")
    elseif (UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(SLANG_BINARIES "slang-${SLANG_RELEASE_TAG}-linux-x86_64.tar.gz")
    else ()
        message(
            FATAL_ERROR "No prebuilt Slang asset configured for platform ${CMAKE_SYSTEM_NAME} / ${CMAKE_SYSTEM_PROCESSOR}"
        )
    endif ()

    # Set up download/install paths
    set(SLANG_BINARIES_URL "${SLANG_RELEASE_URL}/${SLANG_BINARIES}")
    set(SLANG_INSTALL_DIR "${CMAKE_BINARY_DIR}/external/slang")

    # If compiler binary doesn't exist yet
    if (NOT EXISTS "${SLANG_INSTALL_DIR}/bin/slangc" AND NOT EXISTS "${SLANG_INSTALL_DIR}/bin/slangc.exe")
        # Download the binaries
        message(STATUS "Downloading Slang binaries from ${SLANG_BINARIES_URL}")

        set(SLANG_BINARIES_ARCHIVE "${CMAKE_BINARY_DIR}/${SLANG_BINARIES}")

        file(DOWNLOAD "${SLANG_BINARIES_URL}" "${SLANG_BINARIES_ARCHIVE}" SHOW_PROGRESS)

        # Extract the downloaded binaries archive into the installation directory
        file(MAKE_DIRECTORY "${SLANG_INSTALL_DIR}")

        file(ARCHIVE_EXTRACT INPUT "${SLANG_BINARIES_ARCHIVE}" DESTINATION "${SLANG_INSTALL_DIR}")

        # Remove the binaries archive
        file(REMOVE_RECURSE "${SLANG_BINARIES_ARCHIVE}")
    endif ()

    list(APPEND CMAKE_PREFIX_PATH "${SLANG_INSTALL_DIR}")

    find_package(slang CONFIG REQUIRED)
endif ()

target_link_libraries(NobleEngine PRIVATE slang::slang)

##########################################################
#                  SHADERS COMPILATION                   #
##########################################################

set(SHADERS_DIR_RELATIVE resources/shaders)
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/${SHADERS_DIR_RELATIVE})

function (add_slang_shader_target TARGET)
    cmake_parse_arguments(SHADERS "" "" "SOURCES;ENTRIES" ${ARGN})

    set(SHADERS_SPV_DIR_RELATIVE ${SHADERS_DIR_RELATIVE}/spv)
    set(SHADERS_SPV_DIR ${SHADERS_DIR}/spv)
    file(MAKE_DIRECTORY ${SHADERS_SPV_DIR})

    set(SPV_OUTPUTS "")

    foreach (SOURCE ${SHADERS_SOURCES})
        cmake_path(GET SOURCE STEM SOURCE_NAME)

        foreach (ENTRY_PAIR ${SHADERS_ENTRIES})
            string(REPLACE "=" ";" ENTRY_SPLIT "${ENTRY_PAIR}")
            list(GET ENTRY_SPLIT 0 ENTRY_POINT)
            list(GET ENTRY_SPLIT 1 STAGE_NAME )

            string(SUBSTRING "${STAGE_NAME}" 0 4 STAGE_NAME_SHORT)

            set(SPV_FILE_NAME ${SOURCE_NAME}.${STAGE_NAME_SHORT}.spv)
            set(SPV_FILE ${SHADERS_SPV_DIR}/${SPV_FILE_NAME})
            list(APPEND SPV_OUTPUTS ${SPV_FILE})

            # Run Slangc command
            add_custom_command(
                OUTPUT ${SPV_FILE}
                COMMAND "${SLANGC_EXECUTABLE}" ${SOURCE}
                -target spirv
                -entry ${ENTRY_POINT}
                -stage ${STAGE_NAME}
                -profile spirv_1_4
                -emit-spirv-directly
                -fvk-use-entrypoint-name
                -o ${SPV_FILE}
                DEPENDS ${SOURCE}
                COMMENT "Building SPIR-V object ${SHADERS_SPV_DIR_RELATIVE}/${SPV_FILE_NAME}"
                VERBATIM
            )
        endforeach ()
    endforeach ()

    add_custom_target(${TARGET} DEPENDS ${SPV_OUTPUTS})
endfunction ()

file(GLOB_RECURSE SHADERS_SOURCES ${SHADERS_DIR}/src/*.slang)

add_slang_shader_target(
    SlangShaders
    SOURCES ${SHADERS_SOURCES}
    ENTRIES "fragMain=fragment" "vertMain=vertex"
)
