SHELL := /bin/bash

.PHONY: help all copy-native build-prebuilt build test demo-build clean

help:
	@echo "make all            # install iOS/macOS xcframeworks from prebuilt + swift build"
	@echo "make copy-native    # install iOS/macOS xcframeworks from prebuilt"
	@echo "make build-prebuilt # rebuild iOS and macOS prebuilt xcframework archives"
	@echo "make build      # swift build for Apple package"
	@echo "make test       # swift test for Apple package"
	@echo "make demo-build # build macOS demo package"
	@echo "make clean      # remove Apple package build outputs"

all: build-prebuilt copy-native build

copy-native:
	bash ./scripts/copy-native-xcframework.sh all

build-prebuilt:
	bash ../../scripts/build-shared.sh -p osx
	bash ../../scripts/build-shared.sh -p ios

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
