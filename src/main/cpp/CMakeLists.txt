cmake_minimum_required(VERSION 3.4.1)
project(oboe-amp LANGUAGES C CXX)

find_package(oboe REQUIRED CONFIG)

add_library(oboe-amp SHARED
    LiveEffectEngine.cpp
    FullDuplexStream.cpp
    jni_bridge.cpp
)

target_link_libraries(oboe-amp PRIVATE oboe::oboe android atomic log)
