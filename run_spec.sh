#!/bin/bash
# $1 - app name
# $2 - execno
#BENCHMARK_DIR=/home/mejbah/benchmark/SPEC/spec2000
#BIN_DIR=$BENCHMARK_DIR/bin
BIN_DIR=/home/mejbah/benchmark/SPEC/scripts/bin
PIN_HOME=/home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux

i=$2

#EXPERIMENT_HOME=/home/muzahid/projects/code/performance_bug/bugs/experiments
#TYPE=spec
DATA_DIR=./data
PROGRAM_DATA_DIR=${DATA_DIR}/${1}
#LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/muzahid/projects/code/all_pin/pin/intel64/lib-ext
#export LD_LIBRARY_PATH
#PIN=/home/muzahid/projects/code/all_pin/pin/intel64/bin/pinbin

export LD_LIBRARY_PATH=/home/mejbah/opencv-libs/lib:${LD_LIBRARY_PATH}

echo execution no $2

if [ -d "${PROGRAM_DATA_DIR}" ]; then
  echo ${PROGRAM_DATA_DIR} already exists
else
  echo creating new ${PROGRAM_DATA_DIR} directory ...
  mkdir ${PROGRAM_DATA_DIR}
fi
mkdir ${PROGRAM_DATA_DIR}/run$2  
SPECAPP="$1"

if [ "$SPECAPP" = "gzip.spec" ]
then
  ARGS="${BIN_DIR}/input.compressed 1"
fi

#if [ "$SPECAPP" = "vpr.spec" ]
#then
#  ARGS="${BIN_DIR}/net.in ${BIN_DIR}/arch.in ${PROGRAM_DATA_DIR}/place.${HOSTNAME}.$$.out ${PROGRAM_DATA_DIR}/dum.${HOSTNAME}.$$.out -nodisp -place_only -init_t 5 -exit_t 0.005 -alpha_t 0.9412 -inner_num 2"
#fi

if [ "$SPECAPP" = "gcc.spec" ]
then
  ARGS="${BIN_DIR}/cccp.i -o ${PROGRAM_DATA_DIR}/gcc1.${HOSTNAME}.$$.s"
fi

if [ "$SPECAPP" = "mcf.spec" ]
then
  ARGS="${BIN_DIR}/inp.in"
fi

if [ "$SPECAPP" = "perlbmk.spec" ]
then
#  cd ${BIN_DIR}
  ARGS="${BIN_DIR}/scrabbl.pl" 
  PIN_ARGS="-stdin ${BIN_DIR}/scrabbl.in"
fi

if [ "$SPECAPP" = "bzip2.spec" ]
then
  ARGS="${BIN_DIR}/input.random 58"
fi

if [ "$SPECAPP" = "twolf.spec" ]
then
  cd ${BIN_DIR}
  ARGS="test"
fi

APP_NAME=$1
OUTPUT_PATH=./data
DATA_PATH=/home/mejbah/pintool/mem_access_LR/data
MODE=0
CONF_DIR=./data/spec.conf
PINTOOL_ARGS="-mode $MODE -datadir $DATA_PATH/$APP_NAME -mlp $OUTPUT_PATH/$APP_NAME/run$i"

mkdir -p ${OUTPUT_PATH}/${APP_NAME}/run$i

START_TIME=$(($(date +%s%N)/1000000))
${BIN_DIR}/${SPECAPP} ${ARGS}
END_TIME=$(($(date +%s%N)/1000000))
TOTAL_TIME=$(($END_TIME -$START_TIME))
echo "Original elapsed time: $TOTAL_TIME milliseconds" > ${OUTPUT_PATH}/$APP_NAME/run$i/original.out

echo ${PIN_HOME}/pin -t ./mentalist.so -c ./data/spec.conf -o ${OUTPUT_PATH}/$APP_NAME/run$i/report.out $PINTOOL_ARGS -- ${BIN_DIR}/${SPECAPP} ${ARGS}

${PIN_HOME}/pin -t ./mentalist.so -c ./data/spec.conf -o  ${OUTPUT_PATH}/$APP_NAME/run$i/report.out $PINTOOL_ARGS -- ${BIN_DIR}/${SPECAPP} ${ARGS}

echo done

