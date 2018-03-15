all: invoke_tests

invoke_tests: invoke_make
	cd build && ./unittesting

invoke_make: invoke_cmake
	$(MAKE) -C build/

invoke_cmake: CMakeLists.txt
	cd build && cmake ${CMAKE_ARGS} ../

collect_coverage: invoke_make
	$(MAKE) -C build/ collect_coverage

clean:
	rm -rf build/*
