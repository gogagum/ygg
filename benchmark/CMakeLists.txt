project(benchmark C CXX)

include(FetchContent)

################################################################################
# Google-benchmark                                                             #
################################################################################

FetchContent_Declare(
    gbenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.7.1.zip
)

FetchContent_MakeAvailable(gbenchmark)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/draup) 
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/draup/draup.hpp")
	message( FATAL_ERROR "draup.hpp not found. Did you forget to run 'git submodule update --init'?" )
endif()

message(STATUS "PAPI_LIBRARIES ${PAPI_LIBRARIES}")

set(CMAKE_CXX_STANDARD 17)
set(LIBS "${LIBS} Threads::Threads benchmark::benchmark")

# Normal
add_executable(run_all run_all.cpp random.cpp)
target_link_libraries(run_all Threads::Threads benchmark::benchmark)

if (PAPI_FOUND)
	add_executable(run_all_papi run_all.cpp random.cpp)
	target_link_libraries(run_all_papi Threads::Threads benchmark::benchmark ${PAPI_LIBRARIES})
	set_target_properties (run_all_papi PROPERTIES COMPILE_DEFINITIONS "USEPAPI")
endif()

# OP-COUNTING
add_executable(run_all_count run_all.cpp random.cpp)
target_link_libraries(run_all_count Threads::Threads benchmark::benchmark)
set_target_properties (run_all_count PROPERTIES COMPILE_DEFINITIONS "COUNTOPS")

# Zipf
add_executable(run_all_zipf run_all.cpp random.cpp)
target_link_libraries(run_all_zipf Threads::Threads benchmark::benchmark)
set_target_properties (run_all_zipf PROPERTIES COMPILE_DEFINITIONS "USEZIPF")

if (PAPI_FOUND)
	add_executable(run_all_zipf_papi run_all.cpp random.cpp)
	target_link_libraries(run_all_zipf_papi Threads::Threads benchmark::benchmark ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_zipf_papi PUBLIC USEPAPI USEZIPF)
endif()

# OP-COUNTING + Zipf
add_executable(run_all_zipf_count run_all.cpp random.cpp)
target_link_libraries(run_all_zipf_count Threads::Threads benchmark::benchmark)
set_target_properties (run_all_zipf_count PROPERTIES COMPILE_DEFINITIONS "COUNTOPS;USEZIPF")


# Presorted
add_executable(run_all_presorted run_all.cpp random.cpp)
target_link_libraries(run_all_presorted Threads::Threads benchmark::benchmark)
set_target_properties (run_all_presorted PROPERTIES COMPILE_DEFINITIONS "PRESORT")

if (PAPI_FOUND)
	add_executable(run_all_presorted_papi run_all.cpp random.cpp)
	target_link_libraries(run_all_presorted_papi Threads::Threads benchmark::benchmark ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_presorted_papi PUBLIC USEPAPI PRESORT)
endif()

# OP-COUNTING + Presorted
add_executable(run_all_presorted_count run_all.cpp random.cpp)
target_link_libraries(run_all_presorted_count Threads::Threads benchmark::benchmark)
set_target_properties (run_all_presorted_count PROPERTIES COMPILE_DEFINITIONS "COUNTOPS;PRESORT")

# Skewed
add_executable(run_all_skewed run_all.cpp random.cpp)
target_link_libraries(run_all_skewed Threads::Threads benchmark::benchmark)
set_target_properties (run_all_skewed PROPERTIES COMPILE_DEFINITIONS "USESKEWED")

if (PAPI_FOUND)
	add_executable(run_all_skewed_papi run_all.cpp random.cpp)
	target_link_libraries(run_all_skewed_papi Threads::Threads benchmark::benchmark ${PAPI_LIBRARIES})
	target_compile_definitions(run_all_skewed_papi PUBLIC USEPAPI USESKEWED)
endif()

# OP-COUNTING + Skewed
add_executable(run_all_skewed_count run_all.cpp random.cpp)
target_link_libraries(run_all_skewed_count Threads::Threads benchmark::benchmark)
set_target_properties (run_all_skewed_count PROPERTIES COMPILE_DEFINITIONS "COUNTOPS;USESKEWED")

# Create BST scripts
set(BENCH_DATASTRUCTURE "BST")
set(BST_OPERATIONS "Insert;Delete;Move;Erase")
FOREACH(OPERATION ${BST_OPERATIONS})
	FOREACH(POSTFIX "" _zipf _skewed _presorted)
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX})
		if (PAPI_FOUND)
			configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark_papi.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX}_papi)
		endif()
	ENDFOREACH()
ENDFOREACH()

# Create DST scripts
set(BENCH_DATASTRUCTURE "DST")
set(DST_OPERATIONS "Insert;Delete;Move")
FOREACH(OPERATION ${DST_OPERATIONS})
	FOREACH(POSTFIX "" _zipf _skewed _presorted)
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX})
		configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark_count.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX}_count)
		if (PAPI_FOUND)
			configure_file(${PROJECT_SOURCE_DIR}/../scripts/benchmark/benchmark_papi.sh.in ${CMAKE_CURRENT_BINARY_DIR}/bench_${BENCH_DATASTRUCTURE}_${OPERATION}${POSTFIX}_papi)			
		endif()
	ENDFOREACH()
ENDFOREACH()


add_executable(pathlengths pathlengths.cpp random.cpp)
target_link_libraries(pathlengths benchmark::benchmark)

add_executable(wbtree_balance wbtree_balance.cpp)
target_link_libraries(wbtree_balance benchmark::benchmark)

add_executable(dst_balance dst_balance.cpp)
target_link_libraries(dst_balance benchmark::benchmark)

add_executable(count_rotations count_rotations.cpp)
target_link_libraries(count_rotations benchmark::benchmark)

add_executable(translate_sequence translate_sequence.cpp)
target_link_libraries(translate_sequence benchmark::benchmark)

add_executable(sequence sequence.cpp)
target_link_libraries(sequence benchmark::benchmark)

add_executable(paired paired.cpp random.cpp)
target_link_libraries(paired Threads::Threads benchmark::benchmark)
