# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5")
  file(MAKE_DIRECTORY "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5")
endif()
file(MAKE_DIRECTORY
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/1"
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5"
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/tmp"
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/src/SMOOTH_BLINK_LED.Debug+PY32F002Bx5-stamp"
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/src"
  "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/src/SMOOTH_BLINK_LED.Debug+PY32F002Bx5-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/src/SMOOTH_BLINK_LED.Debug+PY32F002Bx5-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kfujii/ドキュメント/KeilStudio/PY32F002B/SMOOTH_BLINK_LED/tmp/SMOOTH_BLINK_LED.Debug+PY32F002Bx5/src/SMOOTH_BLINK_LED.Debug+PY32F002Bx5-stamp${cfgdir}") # cfgdir has leading slash
endif()
