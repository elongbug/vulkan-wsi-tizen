CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(vulkan-wsi-tizen C)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BUILD_ARCH "$ENV{RPM_ARCH}")

IF(${BUILD_ARCH} MATCHES "aarch64")
SET(LIB "lib64")
SET(LIBDIR "/usr/lib64")
ELSEIF(${BUILD_ARCH} MATCHES "x86_64")
SET(LIB "lib64")
SET(LIBDIR "/usr/lib64")
ELSE(${BUILD_ARCH} MATCHES "aarch64")
SET(LIB "lib")
SET(LIBDIR "/usr/lib")
ENDIF(${BUILD_ARCH} MATCHES "aarch64")

#################################################################
# Build Package Config File
# ------------------------------
CONFIGURE_FILE(src/wsi/vulkan-wsi-tizen.json.in src/wsi/vulkan-wsi-tizen.json @ONLY)
