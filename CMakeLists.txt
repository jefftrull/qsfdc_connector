cmake_minimum_required(VERSION 2.6)
project(SFDC)

# xsltproc is used for making an Akonadi resource and for determining the Force.com API version
find_program(XSLTPROC_EXECUTABLE xsltproc)

add_subdirectory(soap)
add_subdirectory(lib)
add_subdirectory(sfakonadi)
add_subdirectory(main)
