target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ./
                           )

add_subdirectory(throttle)
add_subdirectory(sequence)
add_subdirectory(setpoint)
add_subdirectory(bit)
add_subdirectory(thruster-start)
