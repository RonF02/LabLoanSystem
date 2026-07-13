# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "CMakeFiles\\LabLoanSystemGUI_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\LabLoanSystemGUI_autogen.dir\\ParseCache.txt"
  "LabLoanSystemGUI_autogen"
  )
endif()
