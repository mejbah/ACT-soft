#!/bin/sh
TEST_NO=$2
APP_NAME=$1
MODE=$3 # 1 - offline train only 2 - Online only 0 - both
BENCHMARK_PATH=/home/mejbah/splash2.annotated
KERNELS_PATH=$BENCHMARK_PATH/codes/kernels
APPS_PATH=$BENCHMARK_PATH/codes/apps
DATA_PATH=/home/mejbah/pintool/mem_access_LR/data

#INPUT_PATH=/home/mejbah/bugbenchmarks/mysql644/
PIN_HOME=/home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux
OUTPUT_PATH=./data/$APP_NAME
CONF_DIR=./data/$APP_NAME/configs

export LD_LIBRARY_PATH=/home/mejbah/opencv-libs/lib:${LD_LIBRARY_PATH}

mkdir -p $OUTPUT_PATH/run$TEST_NO

PINTOOL_ARGS="-mode $MODE -datadir $DATA_PATH/$APP_NAME -mlp $OUTPUT_PATH/run$TEST_NO"

case $APP_NAME in
	"fft" )
	ARGS=" -p4"
	BIN_PATH=$KERNELS_PATH/$APP_NAME
        START_TIME=$(($(date +%s%N)/1000000))
        ${BIN_PATH}/FFT $ARGS 
	END_TIME=$(($(date +%s%N)/1000000))
        TOTAL_TIME=$(($END_TIME -$START_TIME))
        echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/run$TEST_NO/original.out 

	echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/fft.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out  $PINTOOL_ARGS -- ${BIN_PATH}/FFT $ARGS
	${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/fft.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out $PINTOOL_ARGS -- ${BIN_PATH}/FFT $ARGS
        ;;

	"radix" )
	ARGS=" -p4"
	BIN_PATH=$KERNELS_PATH/$APP_NAME

	START_TIME=$(($(date +%s%N)/1000000))
        ${BIN_PATH}/RADIX $ARGS
	END_TIME=$(($(date +%s%N)/1000000))
        TOTAL_TIME=$(($END_TIME -$START_TIME))
        echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/run$TEST_NO/original.out

	echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c ./data/radix.conf -o ${OUTPUT_PATH}/run$1/report.out -- ${BIN_PATH}/RADIX $ARGS
	${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/radix.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out $PINTOOL_ARGS -- ${BIN_PATH}/RADIX $ARGS
        ;;

	"lu" )
	ARGS=" -p4"
	BIN_PATH=$KERNELS_PATH/lu-cont

	START_TIME=$(($(date +%s%N)/1000000))
        ${BIN_PATH}/LU $ARGS
	END_TIME=$(($(date +%s%N)/1000000))
        TOTAL_TIME=$(($END_TIME -$START_TIME))
        echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/run$TEST_NO/original.out
	
	echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/lu.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out $PINTOOL_ARGS -- ${BIN_PATH}/LU $ARGS
	${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/lu.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out $PINTOOL_ARGS -- ${BIN_PATH}/LU $ARGS
        ;;

	"fmm" )	
	BIN_PATH=$APPS_PATH/fmm
        INPUT_PATH=$APPS_PATH/fmm/inputs
	
	START_TIME=$(($(date +%s%N)/1000000))
        ${BIN_PATH}/FMM < $INPUT_PATH/input.256
	END_TIME=$(($(date +%s%N)/1000000))
        TOTAL_TIME=$(($END_TIME -$START_TIME))
        echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/run$TEST_NO/original.out 

	echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/fmm.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out  -stdin $INPUT_PATH/input.256 $PINTOOL_ARGS -- ${BIN_PATH}/FMM $ARGS
	${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/fmm.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out  -stdin $INPUT_PATH/input.256 $PINTOOL_ARGS -- ${BIN_PATH}/FMM $ARGS
        ;;
	"volrend" )
	INPUT_PATH=$APPS_PATH/$APP_NAME/inputs
	ARGS=" 4 $INPUT_PATH/head-scaleddown4"
	BIN_PATH=$APPS_PATH/volrend

	START_TIME=$(($(date +%s%N)/1000000))
        ${BIN_PATH}/VOLREND $ARGS
	END_TIME=$(($(date +%s%N)/1000000))
        TOTAL_TIME=$(($END_TIME -$START_TIME))
	echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/run$TEST_NO/original.out

	echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c ./data/FFT.config -o ${OUTPUT_PATH}/run$1/FFT.out -- ${BIN_PATH}/VOLREND $ARGS
	${PIN_HOME}/pin -mt -t ./mentalist.so -c $CONF_DIR/volrend.conf.$TEST_NO -o ${OUTPUT_PATH}/run$TEST_NO/report.out $PINTOOL_ARGS -- ${BIN_PATH}/VOLREND $ARGS
        ;;

esac

echo done

