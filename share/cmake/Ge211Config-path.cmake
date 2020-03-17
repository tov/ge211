# Adds the current directory to the path for future module loading.
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
list(APPEND CMAKE_MODULE_PATH ${_DIR})
