#!/bin/sh
# $1 TEST_NO

APP_NAME=bc
APP_HOME=/home/mejbah/seq_bugs/bc-1.06_BUG/src
INPUT_DIR=/home/mejbah/seq_bugs/bc-1.06_BUG/inputs

OUTPUT_PATH=./data
PIN_HOME=/home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux
export LD_LIBRARY_PATH=/home/mejbah/opencv-libs/lib:${LD_LIBRARY_PATH}
DATA_PATH=/home/mejbah/pintool/mem_access_LR/data
MODE=0
CONF_DIR=./data/$APP_NAME/configs
PINTOOL_ARGS="-mode $MODE -datadir $DATA_PATH/$APP_NAME -mlp $OUTPUT_PATH/$APP_NAME/run$1"
OUTPUT_PATH=./data
mkdir -p ${OUTPUT_PATH}/${APP_NAME}/run$1

START_TIME=$(($(date +%s%N)/1000000))
${APP_HOME}/$APP_NAME $INPUT_DIR/good_delta.b
END_TIME=$(($(date +%s%N)/1000000))
TOTAL_TIME=$(($END_TIME -$START_TIME))
echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/$APP_NAME/run$1/original.out


echo ${PIN_HOME}/pin -t ./mentalist.so -c ./data/spec.conf  -o ./data/$APP_NAME/run$1/report.out $PINTOOL_ARGS --  ${APP_HOME}/$APP_NAME $INPUT_DIR/good_delta.b

${PIN_HOME}/pin -t ./mentalist.so -c ./data/spec.conf -o ./data/$APP_NAME/run$1/report.out $PINTOOL_ARGS --  ${APP_HOME}/$APP_NAME $INPUT_DIR/good_delta.b

