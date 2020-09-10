TARGET := iphone:clang:latest:7.0
INSTALL_TARGET_PROCESSES = RevelariOS

include $(THEOS)/makefiles/common.mk

APPLICATION_NAME = RevelariOS

RevelariOS_CODESIGN_FLAGS = -Sent.xml
RevelariOS_FILES = main.m RevAppDelegate.m RevRootViewController.m
RevelariOS_FRAMEWORKS = UIKit CoreGraphics
RevelariOS_CFLAGS = -fobjc-arc

include $(THEOS_MAKE_PATH)/application.mk
include $(THEOS_MAKE_PATH)/aggregate.mk
