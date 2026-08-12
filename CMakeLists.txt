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
    ${ZEPHYR_CURRENT_MODULE_DIR}/zephyr/dirent.c
  )
endif()

# Core clixon library sources (only when full library is enabled)
if(CONFIG_CLIXON)
  zephyr_library_sources(
    # Core functionality (no external dependencies)
    # Note: Excluding files that require CLIgen library or dlopen/plugin support
    # Also excluding files requiring OpenSSL
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_autocli.c  # Requires cligen
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_autocli_generate.c # Requires cligen
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_client.c # Requires cligen
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_data.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_datastore.c # Requires dlfcn (plugins)
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_datastore_read.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_datastore_write.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_debug.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_digest.c # Requires openssl
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_dispatcher.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_err.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_event_select.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_file.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_handle.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_hash.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_json.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_log.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_map.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_nacm.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_input.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_lib.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netconf_monitoring.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_netns.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_options.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_path.c
    # ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_plugin.c # Requires dlfcn (plugins)
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_proc.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_proto.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_proto_client.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_regex.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_sig.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_stream.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_string.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_text_syntax.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_uid.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_validate.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_validate_minmax.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_bind.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_changelog.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_default.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_diff.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_io.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_map.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_nsctx.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_sort.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xml_vec.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_ctx.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_eval.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_function.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_optimize.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_xpath_yang.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_cardinality.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_module.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_parse_lib.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_schema_mount.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_sub_parse.c
    ${ZEPHYR_CURRENT_MODULE_DIR}/lib/src/clixon_yang_type.c
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
