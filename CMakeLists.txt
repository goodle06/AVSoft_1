# CMakeList.txt : CMake project for AVsoftTest_Semenov, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("AVsoftTest_Semenov")

# Add source to this project's executable.
add_executable (AVsoftTest_Semenov "AVsoftTest_Semenov.cpp" "AVsoftTest_Semenov.h" "WindowsThread.h" "WindowsThread.cpp" "Common.h" "CRWLock.h" "CRWLock.cpp")

# TODO: Add tests and install targets if needed.
