#!/bin/bash
# $1 - program_name
# $2 - input_size - test , small
# $3 - exec no
BENCHMARK_DIR=/home/mejbah/benchmark/parsec-2.1/pkgs
BIN_DIR=/inst/amd64-linux.gcc-pthreads/bin/
PIN_HOME=/home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux
#OUTPUT_PATH=/home/mejbah/false_sharing_data
OUTPUT_PATH=./data

i=$3

export LD_LIBRARY_PATH=/home/mejbah/opencv-libs/lib:${LD_LIBRARY_PATH}

NTHREADS=4


echo "Running test for $1"

case $1 in
	"canneal" ) APP_NAME=canneal
	PKG_PART=/kernels/canneal
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} 5 100 ${INPUT}/10.nets 1"
		;;
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} 10000 2000 ${INPUT}/100000.nets 32"
		;;		
	esac
	;;
	"streamcluster" ) APP_NAME=streamcluster
	PKG_PART=/kernels/streamcluster
        OUTPUT=/run/output.txt
	case $2 in
		"test" ) RUN_ARGS="2 5 1 10 10 5 none ${BENCHMARK_DIR}${PKG_PART}${OUTPUT} ${NTHREADS}";;
		"small" ) RUN_ARGS="10 20 32 4096 4096 1000 none ${BENCHMARK_DIR}${PKG_PART}${OUTPUT} ${NTHREADS}";;
		
	esac;;

	"blackscholes" ) APP_NAME=blackscholes
	PKG_PART=/apps/blackscholes
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
        OUTPUT=${BENCHMARK_DIR}${PKG_PART}/run
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} ${INPUT}/in_4.txt ${OUTPUT}/prices.txt ";;
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} ${INPUT}/in_4K.txt ${OUTPUT}/prices.txt ";;
		
	esac;;


	"bodytrack" ) APP_NAME=bodytrack
	PKG_PART=/apps/bodytrack
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/sequenceB_1 4 1 5 1 0  ${NTHREADS}";;	
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/sequenceB_1 4 1 1000 5 0  ${NTHREADS}";;		
	esac;;


	"facesim" ) APP_NAME=facesim
	PKG_PART=/apps/facesim
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_native.tar; # for facesim the input folder has to be in the current folder
		RUN_ARGS="-h";;
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar;
		RUN_ARGS="-timing -threads ${NTHREADS}";;
	esac;;

	"ferret" ) APP_NAME=ferret
	PKG_PART=/apps/ferret
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	OUTPUT=${BENCHMARK_DIR}${PKG_PART}/run
	case $2 in
		"test" )  tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/corel lsh ${INPUT}/queries 1 1 ${NTHREADS} ${OUTPUT}/output.txt";;
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/corel lsh ${INPUT}/queries 10 20 ${NTHREADS} ${OUTPUT}/output.txt";;
	esac;;

	"swaptions" ) APP_NAME=swaptions
	PKG_PART=/apps/swaptions
	case $2 in
		"test" ) RUN_ARGS="-ns 1 -sm 5 -nt ${NTHREADS}";;
		"small" ) RUN_ARGS="-ns 16 -sm 5000 -nt ${NTHREADS}";;
	esac;;

	"raytrace" ) APP_NAME=rtview
	PKG_PART=/apps/raytrace
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/octahedron.obj -nodisplay -automove -nthreads  ${NTHREADS} -frames 1 -res 1 1";;
		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${INPUT}/happy_buddha.obj -nodisplay -automove -nthreads  ${NTHREADS} -frames 3 -res 480 270";;
	esac;;

	"fluidanimate" ) APP_NAME=fluidanimate
	PKG_PART=/apps/fluidanimate
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	OUTPUT=${BENCHMARK_DIR}${PKG_PART}/run
	
	case $2 in
		"test" ) 
		echo "Running for test input";
		tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} 1 ${INPUT}/in_5K.fluid ${OUTPUT}/out.fluid"
		;;
		"small" )
		echo "Running for small input"
		 tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="${NTHREADS} 5 ${INPUT}/in_35K.fluid ${OUTPUT}/out.fluid"
		;;
	esac;;

	"x264" ) APP_NAME=x264
	PKG_PART=/apps/x264
	INPUT=${BENCHMARK_DIR}${PKG_PART}/inputs
	OUTPUT=${BENCHMARK_DIR}${PKG_PART}/run
	case $2 in
		"test" ) tar -xvf ${INPUT}/input_test.tar -C ${INPUT}/;
		RUN_ARGS="--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fast-pskip --me umh --subme 7 --analyse b8x8,i4x4 --threads ${NTHREADS} -o eledream.264 ${INPUT}/eledream_32x18_1.y4m";;

		"small" ) tar -xvf ${INPUT}/input_simsmall.tar -C ${INPUT}/;
		RUN_ARGS="--quiet --qp 20 --partitions b8x8,i4x4 --ref 5 --direct auto --b-pyramid --weightb --mixed-refs --no-fastpskip --me umh --subme 7 --analyse b8x8,i4x4 --threads ${NTHREADS} -o eledream.264 ${INPUT}eledream_640x360_8.y4m";;
	esac;;

esac


DATA_PATH=/home/mejbah/pintool/mem_access_LR/data
MODE=0
CONF_DIR=./data/$APP_NAME/configs
PINTOOL_ARGS="-mode $MODE -datadir $DATA_PATH/$APP_NAME -mlp $OUTPUT_PATH/$APP_NAME/run$i"

mkdir -p ${OUTPUT_PATH}/${APP_NAME}/run$i

START_TIME=$(($(date +%s%N)/1000000))
${BENCHMARK_DIR}${PKG_PART}${BIN_DIR}${APP_NAME} ${RUN_ARGS}
END_TIME=$(($(date +%s%N)/1000000))
TOTAL_TIME=$(($END_TIME -$START_TIME))
echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/$APP_NAME/run$i/original.out


echo ${PIN_HOME}/pin -mt -t ./mentalist.so -c ./data/parsec.conf -o ${OUTPUT_PATH}/${APP_NAME}/run$i/report.out $PINTOOL_ARGS  -- ${BENCHMARK_DIR}${PKG_PART}${BIN_DIR}${APP_NAME} ${RUN_ARGS}
${PIN_HOME}/pin -mt -t ./mentalist.so -c ./data/parsec.conf -o ${OUTPUT_PATH}/${APP_NAME}/run$i/report.out $PINTOOL_ARGS -- ${BENCHMARK_DIR}${PKG_PART}${BIN_DIR}${APP_NAME} ${RUN_ARGS}

#done  
  
echo test for $1 done  
