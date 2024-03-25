set(NLDTOOL_FILES
  examples/crypto_factory.cpp
  examples/crypto_factory.h
  examples/crypto_options.cpp
  examples/crypto_options.h
  examples/main.cpp
)

# search for available crypto algorithms
file(GLOB CRYPTO_GLOB RELATIVE "${CMAKE_SOURCE_DIR}/examples" "${CMAKE_SOURCE_DIR}/examples/*/sources.cmake")
foreach(i ${CRYPTO_GLOB})
  string(REPLACE "/sources.cmake" "" i ${i})
  set(CRYPTO ${CRYPTO} ${i})
endforeach()

# include all available crypto algorithms if none was specified
set(NLDTOOL_CRYPTO ${CRYPTO} CACHE STRING
  "Choose the list of crypto algorithms to include in the tool, options are: ${CRYPTO}")

# add crypto source files
foreach(i ${NLDTOOL_CRYPTO})
  include(examples/${i}/sources.cmake)
  set(NLDTOOL_FILES ${NLDTOOL_FILES} ${CRYPTO_FILES})
endforeach()

# generate crypto factory file
file(WRITE  examples/crypto_factory.cpp  "// \\brief Automatically generated file. Do not edit!\n")
file(APPEND examples/crypto_factory.cpp  "\n")
file(APPEND examples/crypto_factory.cpp  "#include \"crypto_factory.h\"\n")
file(APPEND examples/crypto_factory.cpp  "\n")
foreach(i ${NLDTOOL_CRYPTO})
  include(examples/${i}/sources.cmake)
  foreach(j ${CRYPTO_FILES})
    if (j MATCHES ".*[.]h$")
      string(REPLACE "examples/" "" HEADER ${j})
      file(APPEND examples/crypto_factory.cpp "#include \"${HEADER}\"\n")
    endif()
  endforeach()
endforeach()
file(APPEND examples/crypto_factory.cpp  "\n")
file(APPEND examples/crypto_factory.cpp  "Crypto* CryptoFactory(cxxopts::Options& options) {\n")
file(APPEND examples/crypto_factory.cpp  "  std::string function(options[\"function\"].as<std::string>());\n")
foreach(i ${NLDTOOL_CRYPTO})
  include(examples/${i}/sources.cmake)
  foreach(j ${MAP_OPTION_TO_CLASSNAME})
    string(REGEX REPLACE ":.*" "" OPTION ${j})
    string(REGEX REPLACE ".*:" "" CLASSNAME ${j})
    file(APPEND examples/crypto_factory.cpp  "  if (function.compare(\"${OPTION}\") == 0) return new ${CLASSNAME}(options);\n")
  endforeach()
endforeach()
file(APPEND examples/crypto_factory.cpp  "  std::cerr << \"error: invalid crypto function\" << std::endl;\n")
file(APPEND examples/crypto_factory.cpp  "  exit(-1);\n")
file(APPEND examples/crypto_factory.cpp  "}\n")

# generate crypto options file
file(WRITE  examples/crypto_options.cpp "// \\brief Automatically generated file. Do not edit!\n")
file(APPEND examples/crypto_options.cpp  "\n")
file(APPEND examples/crypto_options.cpp  "#include \"crypto_options.h\"\n")
file(APPEND examples/crypto_options.cpp  "\n")
foreach(i ${NLDTOOL_CRYPTO})
  include(examples/${i}/sources.cmake)
  foreach(j ${CRYPTO_FILES})
    if (j MATCHES ".*[.]h$")
      string(REPLACE "examples/" "" HEADER ${j})
      file(APPEND examples/crypto_options.cpp "#include \"${HEADER}\"\n")
    endif()
  endforeach()
endforeach()
file(APPEND examples/crypto_options.cpp  "\n")
file(APPEND examples/crypto_options.cpp "void AddCryptoSpecificOptions(cxxopts::Options& options) {\n")
foreach(i ${NLDTOOL_CRYPTO})
  include(examples/${i}/sources.cmake)
  foreach(j ${MAP_OPTION_TO_CLASSNAME})
    string(REGEX REPLACE ".*:" "" CLASSNAME ${j})
    file(APPEND examples/crypto_options.cpp "  ${CLASSNAME}::AddToOptions(options);\n")
  endforeach()
endforeach()
file(APPEND examples/crypto_options.cpp "}\n")

# add test case for "./nldtool --help"
add_test(_help nldtool --help)

# add check char test cases for each crypto algorithm
foreach(CRYPTO ${NLDTOOL_CRYPTO})
  # add test cases for all testvectors
  file(GLOB TESTVECTORS RELATIVE "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/examples/${CRYPTO}/testvectors/*.xml")
  foreach(TESTFILE ${TESTVECTORS})
    add_test(${TESTFILE} nldtool -i ${CMAKE_SOURCE_DIR}/${TESTFILE} --check-char 2)
  endforeach()
  # add test cases for all other xml files (excluding testvectors)
  file(GLOB XMLFILES RELATIVE "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/examples/${CRYPTO}/*/*.xml" EXCLUDE testvectors)
  foreach(TESTFILE ${XMLFILES})
    add_test(${TESTFILE} nldtool -i ${CMAKE_SOURCE_DIR}/${TESTFILE} --check-char 2)
  endforeach()
endforeach()

# add search test cases for each crypto algorithm (only MD4 for now)
add_test(md4_search nldtool -i ${CMAKE_SOURCE_DIR}/examples/md4/eurocryptWangLFCY05/start.xml -R 963821092 -E)
