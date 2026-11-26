APPNAME = mapd-visualizer
OF_ROOT ?= $(CURDIR)/third_party/openFrameworks
OF_INCLUDE = $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

include $(OF_INCLUDE)

ifeq ($(PLATFORM_OS),Darwin)
MAPD_MAC_SDK = $(if $(MAC_OS_SDK_ROOT),$(MAC_OS_SDK_ROOT),$(shell xcrun --show-sdk-path))
ifeq ($(wildcard $(MAPD_MAC_SDK)/System/Library/Frameworks/AGL.framework),)
# openFrameworks 0.12.0 lists AGL, which recent macOS SDKs omit.
OF_PROJECT_LDFLAGS := $(subst -framework AGL,,$(OF_PROJECT_LDFLAGS))
endif
endif
