#!/bin/bash
SCRIPT_DIR=$(dirname "$0")
cmake $SCRIPT_DIR -DENABLE_TESTS=ON -DENABLE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo \
                  -DENABLE_ZMQ=ON -DENABLE_BEDROCK=ON
