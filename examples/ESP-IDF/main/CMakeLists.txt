set(SOURCE_FILES "")
file(GLOB_RECURSE SOURCE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

idf_component_register(
  SRCS
    ${SOURCE_FILES}

  PRIV_REQUIRES
    esp_event
    lwip
    esp_wifi
    nvs_flash
    spi_flash
    esp_netif
    DCCEXProtocol
  INCLUDE_DIRS
    "."
)
