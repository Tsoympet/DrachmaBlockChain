# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-src")
  file(MAKE_DIRECTORY "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-src")
endif()
file(MAKE_DIRECTORY
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-build"
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix"
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/tmp"
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/src/leveldb-populate-stamp"
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/src"
  "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/src/leveldb-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/src/leveldb-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/runner/work/BlockChainDrachma/BlockChainDrachma/_codeql_build_dir/_deps/leveldb-subbuild/leveldb-populate-prefix/src/leveldb-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
