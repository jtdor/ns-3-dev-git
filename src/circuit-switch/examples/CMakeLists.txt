build_lib_example(
  NAME simple-circuit-switch SOURCE_FILES simple-circuit-switch.cc
  LIBRARIES_TO_LINK ${libcircuit-switch} ${libinternet} ${libinternet-apps}
                    ${libnetwork} ${libpoint-to-point}
)
