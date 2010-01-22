# set up Qt stuff
find_package(Qt4 REQUIRED)
set(QT_USE_QTSQL TRUE)
include(${QT_USE_FILE})

include_directories("../soap" "${CMAKE_CURRENT_BINARY_DIR}/../soap")
if (GSOAP_BUILD_DIR)
  include_directories(BEFORE ${GSOAP_BUILD_DIR})
endif()
add_library(qsql_sfdc qsql_sfdc.cpp)
target_link_libraries(qsql_sfdc soap_sfdc ${QT_LIBRARIES})
