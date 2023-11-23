#!/usr/bin/bash

(cd exodrivers && git checkout develop)
(cd hardwarecontrol && git checkout develop)
(cd exomodules && git checkout develop)

(cd third-party/canopen-stack && git checkout exomaster)
(cd third-party/freertos && git checkout exomaster)
(cd third-party/libcrc-utility && git checkout exomaster)
