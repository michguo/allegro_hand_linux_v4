add_executable (grasp main.cpp canAPI.cpp RockScissorsPaper.cpp)

#find cppzmq wrapper, installed by make of cppzmq
find_package(cppzmq)
target_link_libraries(grasp cppzmq)
target_link_libraries (grasp BHand pcanbasic pthread cppzmq)

#install (TARGETS grasp DESTINATION ${PROJECT_SOURCE_DIR}/bin)
install (TARGETS grasp DESTINATION ${PROJECT_BINARY_DIR}/bin)
