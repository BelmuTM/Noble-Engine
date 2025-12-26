#[[
    Slang
        * Upstream: https://github.com/shader-slang/slang
        * License: Apache 2.0
]]

##########################################################
#                      INSTALLATION                      #
##########################################################

function (fetch_slang TARGET)

    message(CHECK_START "Check for Slang compiler")

    find_package(slang CONFIG QUIET)

    if (slang_FOUND)
        message(CHECK_PASS "done")
    else()
        message(CHECK_FAIL "failed")

        set(SLANG_RELEASE_TAG "2025.23.2")
        set(SLANG_RELEASE_URL "https://github.com/shader-slang/slang/releases/download/v${SLANG_RELEASE_TAG}")

        # Fetch binaries depending on the operating system
        if (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SLANG_BINARIES "slang-${SLANG_RELEASE_TAG}-windows-x86_64.tar.gz")
        elseif (UNIX AND CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(SLANG_BINARIES "slang-${SLANG_RELEASE_TAG}-linux-x86_64.tar.gz")
        else()
            message(
                FATAL_ERROR
                "No prebuilt Slang binaries configured for platform ${CMAKE_SYSTEM_NAME} / ${CMAKE_SYSTEM_PROCESSOR}"
            )
        endif()

        # Set up download/install paths
        set(SLANG_BINARIES_URL "${SLANG_RELEASE_URL}/${SLANG_BINARIES}")
        set(SLANG_INSTALL_DIR "${CMAKE_BINARY_DIR}/external/slang")

        # If compiler binary doesn't exist yet
        if (NOT EXISTS "${SLANG_INSTALL_DIR}/bin/slangc" AND NOT EXISTS "${SLANG_INSTALL_DIR}/bin/slangc.exe")
            # Download the binaries
            message(STATUS "Downloading Slang binaries from ${SLANG_BINARIES_URL}")

            set(SLANG_BINARIES_ARCHIVE "${CMAKE_BINARY_DIR}/${SLANG_BINARIES}")

            file(DOWNLOAD
                "${SLANG_BINARIES_URL}"
                "${SLANG_BINARIES_ARCHIVE}"
                SHOW_PROGRESS
                STATUS DOWNLOAD_STATUS
                LOG DOWNLOAD_LOG
            )

            # Check if the download succeeded
            list(GET DOWNLOAD_STATUS 0 DOWNLOAD_STATUS_CODE)

            if (NOT DOWNLOAD_STATUS_CODE EQUAL 0)
                message(FATAL_ERROR "Failed downloading Slang binaries:\n${DOWNLOAD_LOG}")
            else()
                message(STATUS "Finished downloading Slang binaries")
            endif()

            # Extract the downloaded binaries archive into the installation directory
            file(MAKE_DIRECTORY "${SLANG_INSTALL_DIR}")

            file(ARCHIVE_EXTRACT INPUT "${SLANG_BINARIES_ARCHIVE}" DESTINATION "${SLANG_INSTALL_DIR}")

            # Remove the binaries archive
            file(REMOVE_RECURSE "${SLANG_BINARIES_ARCHIVE}")
        endif()

        list(APPEND CMAKE_PREFIX_PATH "${SLANG_INSTALL_DIR}")

        find_package(slang CONFIG REQUIRED)
    endif()

    target_link_libraries(${TARGET} PRIVATE slang::slang)

    message(STATUS "Linked Slang library")

endfunction()

##########################################################
#                  SHADERS COMPILATION                   #
##########################################################

function (compile_slang TARGET)
    # Set shaders directory
    set(SHADERS_DIR_RELATIVE resources/shaders)
    set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/${SHADERS_DIR_RELATIVE})

    function (add_slang_shader_target SHADER_TARGET)

        cmake_parse_arguments(SHADERS "" "" "SOURCES;ENTRIES" ${ARGN})

        # Set compiled SPIR-V outptut directory
        set(SHADERS_SPV_DIR_RELATIVE ${SHADERS_DIR_RELATIVE}/spv)
        set(SHADERS_SPV_DIR ${SHADERS_DIR}/spv)
        file(MAKE_DIRECTORY ${SHADERS_SPV_DIR})

        set(SPV_OUTPUTS "")

        # Fetch shaders source files
        foreach (SOURCE ${SHADERS_SOURCES})
            cmake_path(GET SOURCE STEM SOURCE_NAME)

            # Compile each stage (entrypoint)
            foreach (ENTRY_PAIR ${SHADERS_ENTRIES})
                # Separate the compiled outputs per entrypoint
                string(REPLACE "=" ";" ENTRY_SPLIT "${ENTRY_PAIR}")
                list(GET ENTRY_SPLIT 0 ENTRY_POINT)
                list(GET ENTRY_SPLIT 1 STAGE_NAME )

                string(SUBSTRING "${STAGE_NAME}" 0 4 STAGE_NAME_SHORT)

                set(SPV_FILE_NAME ${SOURCE_NAME}.${STAGE_NAME_SHORT}.spv)
                set(SPV_FILE ${SHADERS_SPV_DIR}/${SPV_FILE_NAME})
                list(APPEND SPV_OUTPUTS ${SPV_FILE})

                # Run Slangc command to compile shaders to SPIR-V
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
            endforeach()
        endforeach()

        add_custom_target(${SHADER_TARGET} DEPENDS ${SPV_OUTPUTS})

    endfunction()

    # Collect Slang shaders sources
    file(GLOB_RECURSE SHADERS_SOURCES ${SHADERS_DIR}/*.slang)

    # Filter out Slang files inside the include folder
    list(FILTER SHADERS_SOURCES EXCLUDE REGEX "${SHADERS_DIR}/include/.*")

    # Compile shaders
    add_slang_shader_target(
        SlangShaders
        SOURCES ${SHADERS_SOURCES}
        ENTRIES "fragMain=fragment" "vertMain=vertex"
    )

    add_dependencies(${TARGET} SlangShaders)

endfunction()
