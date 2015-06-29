#!/bin/bash$

cp  ~/data/bin/protobuf/config.h src/android/jni/protobuf/
rsync -av ~/data/bin/protobuf/src/google/ src/android/jni/protobuf/google/
rsync -av ../cheminot.c/src/protobuf/ src/android/jni/cheminotc/protobuf/
cp ../cheminot.c/src/cheminotc.cpp src/android/jni/cheminotc/
cp ../cheminot.c/src/cheminotc.h src/android/jni/cheminotc/
cp ../cheminot.c/src/settings.h src/android/jni/cheminotc/
