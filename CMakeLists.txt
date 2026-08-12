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
endif()

# Clixon service (auto-initialization support)
if(CONFIG_CLIXON_SERVICE)
  zephyr_library_sources(
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/clixon_service.c
  )
endif()

# Zephyr compatibility layer
if(CONFIG_CLIXON)
  zephyr_library_sources(
    # Don't include dirent.c - Zephyr POSIX layer provides opendir/readdir/closedir
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/cligen/cligen.c
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
    
    # More to enable as dependencies are resolved:
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_err.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event_select.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_file.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_handle.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_log.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_options.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_proc.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_sig.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_stream.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_uid.c
  )
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
