#!/bin/bash
PKG_CONFIG_PATH := /home/mejbah/opencv-libs/lib/pkgconfig:${PKG_CONFIG_PATH}
export PKG_CONFIG_PATH 
LD_LIBRARY_PATH := /home/mejbah/opencv-libs/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH 

