# See https://github.com/bxparks/EpoxyDuino for documentation about this
# Makefile to compile and run Arduino programs natively on Linux or MacOS.

APP_NAME := tests

ARDUINO_LIBS := libraries/eva-core-sk libraries/AUnit src

include build-tests/EpoxyDuino.mk

