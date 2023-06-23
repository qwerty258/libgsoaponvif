cmake_minimum_required(VERSION 3.0.0)
project(gsoaponvifclient-c)
add_library(gsoaponvifclient-c SHARED
	generated_code/soapClientLib.c)
target_include_directories(gsoaponvifclient-c PRIVATE generated_code)
target_compile_definitions(gsoaponvifclient-c PUBLIC WITH_DOM)

