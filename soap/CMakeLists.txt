# building sfdc_soap library
# this bit translated from qmake
set(WSDL developeracct.partner.wsdl.xml)
add_definitions(-DWITH_OPENSSL)
set(CMAKE_CXX_FLAGS "-Wno-unused-parameter")
set(WSDL2H "wsdl2h")
set(SOAPCPP2 "soapcpp2")

if (GSOAP_BUILD_DIR)
  include_directories(BEFORE "${GSOAP_BUILD_DIR}")
  set(WSDL2H "${GSOAP_BUILD_DIR}/bin/linux386/wsdl2h")
  set(SOAPCPP2 "${GSOAP_BUILD_DIR}/bin/linux386/soapcpp2")
  set(GSOAP_IMPORT_DIR "${GSOAP_BUILD_DIR}/import")
  # find and refer to the gsoap library
  find_library(GSOAP_LIB
    NAMES gsoapssl++
    PATHS ${GSOAP_BUILD_DIR}
    NO_DEFAULT_PATH)
  add_library(gsoapssl++ UNKNOWN IMPORTED)
  set_property(TARGET gsoapssl++
    PROPERTY IMPORTED_LOCATION "${GSOAP_LIB}")

else (GSOAP_BUILD_DIR)
  find_path(GSOAP_INCLUDE_DIR
            NAMES stdsoap2.h)
  set(GSOAP_IMPORT_DIR "${GSOAP_INCLUDE_DIR}/import")
endif(GSOAP_BUILD_DIR)

# check gSoap version number
set(GSOAP_MIN_VERSION "2.7.10")
# soapcpp2 gives the version output on stderr, so redirect in two stages:
execute_process(COMMAND ${SOAPCPP2} -v
		RESULT_VARIABLE GSOAP_VER_S1_RESULT
		ERROR_VARIABLE GSOAP_VER_S1_OUTPUT)
execute_process(COMMAND echo "${GSOAP_VER_S1_OUTPUT}"
                COMMAND awk "/The gSOAP Stub and Skeleton Compiler/ {print $12}"
		OUTPUT_VARIABLE GSOAP_VER OUTPUT_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE GSOAP_VER_S2_RESULT)

if (NOT GSOAP_VER_S1_RESULT EQUAL 0 OR NOT GSOAP_VER_S2_RESULT EQUAL 0)
  message(FATAL_ERROR "gSoap version check failed.  Have you installed it?")
endif()

if (${GSOAP_VER} VERSION_LESS ${GSOAP_MIN_VERSION})
  message(FATAL_ERROR "installed gSoap version is ${GSOAP_VER} but at least ${GSOAP_MIN_VERSION} is required.  Please install, or use -DGSOAP_BUILD_DIR")
endif()

# define how to use gSoap
add_custom_command(OUTPUT myorg.h
                   COMMAND ${WSDL2H} -k -o myorg.h ${CMAKE_CURRENT_SOURCE_DIR}/${WSDL}
		   DEPENDS ${WSDL})
add_custom_command(OUTPUT soapC.cpp soapSoapBindingProxy.cpp soapSoapBindingProxy.h SoapBinding.nsmap soapH.h soapStub.h
                   COMMAND ${SOAPCPP2} -I${GSOAP_IMPORT_DIR} -i -C -L -x -w myorg.h
		   DEPENDS myorg.h)
# apply the soapC.cpp hack for gsoap bug 2896129
add_custom_command(OUTPUT soapC_patched.cpp soapC_patched.cpp.orig
                   COMMAND cp soapC.cpp soapC_patched.cpp
		   COMMAND patch soapC_patched.cpp ${CMAKE_CURRENT_SOURCE_DIR}/s2qname_bugfix.patch
		   DEPENDS soapC.cpp s2qname_bugfix.patch)


# to get soapSoapBindingProxy.h (generated)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library(soap_sfdc forcewrapper.cpp soapC_patched.cpp soapSoapBindingProxy.cpp)
target_link_libraries(soap_sfdc gsoapssl++ crypto ssl)
