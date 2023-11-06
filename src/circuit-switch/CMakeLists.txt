build_lib(
  LIBNAME circuit-switch
  SOURCE_FILES helper/circuit-switch-helper.cc
               model/circuit-switch-net-device.cc
  HEADER_FILES helper/circuit-switch-helper.h model/circuit-switch-net-device.h
  LIBRARIES_TO_LINK ${libbridge} ${libnetwork}
)
