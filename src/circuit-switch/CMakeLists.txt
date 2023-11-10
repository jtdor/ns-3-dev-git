build_lib(
  LIBNAME circuit-switch
  SOURCE_FILES
    helper/circuit-configuration-rotator-helper.cc
    helper/circuit-switch-helper.cc model/circuit-configuration-rotator.cc
    model/circuit-switch-net-device.cc
  HEADER_FILES
    helper/circuit-configuration-rotator-helper.h helper/circuit-switch-helper.h
    model/circuit-configuration-rotator.h model/circuit-switch-net-device.h
  LIBRARIES_TO_LINK ${libbridge} ${libnetwork}
)
