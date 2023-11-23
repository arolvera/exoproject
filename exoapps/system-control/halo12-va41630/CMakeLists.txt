if("${EXOPROJECT_APP_SUBTYPE}" STREQUAL "app")
        add_subdirectory(app)
else()
        add_subdirectory(tests)
endif()

add_subdirectory(project-shared)
