build_lib_example(
  NAME simple-circuit-switch SOURCE_FILES simple-circuit-switch.cc
  LIBRARIES_TO_LINK ${libcircuit-switch} ${libinternet} ${libinternet-apps}
                    ${libnetwork} ${libpoint-to-point}
)

build_lib_example(
  NAME rotating-circuit-switch SOURCE_FILES rotating-circuit-switch.cc
  LIBRARIES_TO_LINK ${libcircuit-switch} ${libinternet} ${libinternet-apps}
                    ${libnetwork} ${libpoint-to-point}
)

build_lib_example(
  NAME rotating-circuit-switch-topo-read
  SOURCE_FILES rotating-circuit-switch-topo-read.cc
  LIBRARIES_TO_LINK ${libcircuit-switch} ${libinternet} ${libinternet-apps}
                    ${libnetwork} ${libpoint-to-point} ${libtopology-read}
)
