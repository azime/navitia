SET(GEOREF_SRC_TEST
    builder.h
    builder.cpp
)
add_library(georef_test_utils ${GEOREF_SRC_TEST})

add_executable(georef_test georef_test.cpp)
target_link_libraries(georef_test georef_test_utils fare georef utils
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
    ${Boost_REGEX_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} log4cplus)
ADD_BOOST_TEST(georef_test)

add_executable(street_network_test street_network_test.cpp)
target_link_libraries(street_network_test georef_test_utils georef data routing fare autocomplete utils
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY}
        ${Boost_REGEX_LIBRARY} ${Boost_SERIALIZATION_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} log4cplus)
ADD_BOOST_TEST(street_network_test)
