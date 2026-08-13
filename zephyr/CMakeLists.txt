if(CONFIG_CLIXON OR CONFIG_CLIXON_SERVICE)

zephyr_library()

# Add include directories
zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/zephyr)

if(CONFIG_CLIXON)
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/include)
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/lib/src)
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/lib/clixon)
  
  # Add Zephyr compatibility layer to include path FIRST
  # This allows our compatibility headers to override system headers
  zephyr_include_directories(BEFORE ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr)
  
  # Define that we have config.h
  zephyr_compile_definitions(HAVE_CONFIG_H=1)
  
  # Disable -Werror for clixon library sources to allow upstream code with warnings
  # Twister and some build configurations use -Werror by default
  zephyr_library_compile_options(-Wno-error)
endif()

# mbedTLS includes for NETCONF TLS support
if(CONFIG_CLIXON_NETCONF_TLS)
  # mbedTLS headers are automatically included by the mbedTLS module
  # But we ensure they're available for our service code
  zephyr_include_directories(${ZEPHYR_CURRENT_MODULE_DIR}/../../crypto/mbedtls/include)
endif()

# Clixon service (auto-initialization support)
if(CONFIG_CLIXON_SERVICE)
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_service.c
  )
  
  # Datastore backend support
  if(CONFIG_CLIXON_DATASTORE)
    zephyr_library_sources(
      ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_datastore.c
    )
  endif()
  
  # Service uses hash and handle when full library is enabled
  # These are already included in the full library build
endif()

# Zephyr compatibility layer
if(CONFIG_CLIXON)
  zephyr_library_sources(
    # Don't include dirent.c - Zephyr POSIX layer provides opendir/readdir/closedir
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/cligen/cligen.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_err_zephyr.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_log_zephyr.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_debug_zephyr.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_options_zephyr.c
  )
endif()

# Core clixon library sources (only when full library is enabled)
if(CONFIG_CLIXON)
  zephyr_library_sources(
    # Minimal core - only files that can build with CLIgen stubs
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_dispatcher.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_hash.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_map.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_regex.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_string.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml.c
    
    # Support infrastructure
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_handle.c
    # Using zephyr stub: ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_options.c
  )
endif()

# NETCONF support
if(CONFIG_CLIXON_NETCONF)
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_lib.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_input.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_monitoring.c
  )
endif()

if(CONFIG_CLIXON)
  # More to enable as dependencies are resolved:
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event_select.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_file.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_proc.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_sig.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_stream.c
  # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_uid.c
  # zephyr_library_sources(
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_api_path_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_instance_id_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_json_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_text_syntax_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_schemanode_parse.c
  #   ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_sub_parse.c
  # )

  # Add compiler definitions if debug is enabled
  if(CONFIG_CLIXON_DEBUG)
    zephyr_compile_definitions(CLIXON_DEBUG=1)
  endif()
endif() # CONFIG_CLIXON

endif() # CONFIG_CLIXON OR CONFIG_CLIXON_SERVICE
