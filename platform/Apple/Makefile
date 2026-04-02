SHELL := /bin/bash

.PHONY: help all native build test demo-build clean

help:
	@echo "make all        # build iOS/macOS xcframeworks + swift build"
	@echo "make native     # rebuild iOS and macOS xcframeworks"
	@echo "make build      # swift build for Apple package"
	@echo "make test       # swift test for Apple package"
	@echo "make demo-build # build macOS demo package"
	@echo "make clean      # remove Apple package build outputs"

all: native build

native:
	bash ./scripts/build_native_xcframework.sh

build:
	swift build

test:
	swift test

demo-build:
	cd Examples-MacOS && swift build

clean:
	rm -rf .build
	rm -rf binaries/SweetLineCoreIOS.xcframework
	rm -rf binaries/SweetLineCoreOSX.xcframework
