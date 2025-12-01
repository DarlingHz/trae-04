# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-src")
  file(MAKE_DIRECTORY "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-src")
endif()
file(MAKE_DIRECTORY
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-build"
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix"
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/tmp"
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp"
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src"
  "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/soma/code/trae/12-01-01/04/build/_deps/bcrypt-subbuild/bcrypt-populate-prefix/src/bcrypt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
