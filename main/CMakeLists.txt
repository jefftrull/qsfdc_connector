# set up Qt stuff
find_package(Qt4 REQUIRED)
set(QT_USE_QTSQL TRUE)
set(QT_USE_QTTEST TRUE)
include(${QT_USE_FILE})

include_directories("../lib" ${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_UI(pwui_SRC ../sfakonadi/sfdc.ui)
add_executable(main sfdc.cpp ${pwui_SRC})
target_link_libraries(main qsql_sfdc ${QT_LIBRARIES})
