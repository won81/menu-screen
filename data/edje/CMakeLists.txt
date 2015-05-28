ADD_CUSTOM_TARGET(index.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/index.edc index.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/index.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} index.edj)
INSTALL(FILES index.edj DESTINATION ${EDJEDIR})



ADD_CUSTOM_TARGET(item_4x4.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/item_4x4.edc item_4x4.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/item_4x4.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} item_4x4.edj)
INSTALL(FILES item_4x4.edj DESTINATION ${EDJEDIR})



ADD_CUSTOM_TARGET(group_4x4_portrait.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/group_4x4_portrait.edc group_4x4_portrait.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/group_4x4_portrait.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} group_4x4_portrait.edj)
INSTALL(FILES group_4x4_portrait.edj DESTINATION ${EDJEDIR})



ADD_CUSTOM_TARGET(all_apps_portrait.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/all_apps_portrait.edc all_apps_portrait.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/all_apps_portrait.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} all_apps_portrait.edj)
INSTALL(FILES all_apps_portrait.edj DESTINATION ${EDJEDIR})



ADD_CUSTOM_TARGET(layout_portrait.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/layout_portrait.edc layout_portrait.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/layout_portrait.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} layout_portrait.edj)
INSTALL(FILES layout_portrait.edj DESTINATION ${EDJEDIR})



ADD_CUSTOM_TARGET(button_edit.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/../images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/button_edit.edc button_edit.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/button_edit.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} button_edit.edj)
INSTALL(FILES button_edit.edj DESTINATION ${EDJEDIR})



# End of a file
