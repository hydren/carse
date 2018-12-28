#!/bin/bash
echo "### Preparing linux makefiles for prime time ###"

# These need to be in par with the current build configuration names. Be sure that these are updated.
BUILD_CONFIG_ALLEGRO_4_4=linux-gcc-allegro4.4-release
BUILD_CONFIG_ALLEGRO_5_0=linux-gcc-allegro5.0-release
BUILD_CONFIG_SDL_1_2=linux-gcc-sdl1.2-release
BUILD_CONFIG_SDL_2_0=linux-gcc-sdl2.0-release
BUILD_CONFIG_SFML_1_6=linux-gcc-sfml1.6-release
BUILD_CONFIG_SFML_2_0=linux-gcc-sfml2.0-release

# One of the configs. will be used to construct the backend-agnostic makefiles
BUILD_CONFIG_AGNOSTIC=$BUILD_CONFIG_SDL_1_2

function patch_makefiles {
	local DPATH=$1
	local OLD=$2
	local NEW=$3
	
	echo "Will patch .mk files within the $DPATH folder."
	echo "The patch will replace $OLD with $NEW" 
	
	find $DPATH -name *.mk -type f -exec sed -i "s,$OLD,$NEW,g" {} \;
	find $DPATH -name *.mk -type f -exec echo "Patched " {} \;
}

# --- IT BEGINS HERE -----------------------------------------------------------------------
echo "- Erasing content of build folder (except backend-makefiles/)..."
mv build/backend-makefiles .
rm build/* -rf
mv backend-makefiles build/ 

echo "- Performing make clean in one of the build folders ($BUILD_CONFIG_AGNOSTIC)..."
cd  $BUILD_CONFIG_AGNOSTIC
make clean
cd ..

echo "- Copying makefiles from $BUILD_CONFIG_AGNOSTIC folder to build folder..."
cp $BUILD_CONFIG_AGNOSTIC/* -r build/

echo "- Replacing current folder with generic BUILD_PATH token..."
patch_makefiles ./build/ ${PWD} BUILD_PATH

echo "- Moving makefiles specific to $BUILD_CONFIG_AGNOSTIC configuration to separate folder..."
mv build/makefile   build/backend-makefiles/$BUILD_CONFIG_AGNOSTIC/
mv build/objects.mk build/backend-makefiles/$BUILD_CONFIG_AGNOSTIC/
mv build/sources.mk build/backend-makefiles/$BUILD_CONFIG_AGNOSTIC/

echo "- Dealing with remaining makefiles that are specific to other build configurations..."
for BUILD_CONFIG in $BUILD_CONFIG_ALLEGRO_4_4 $BUILD_CONFIG_ALLEGRO_5_0 $BUILD_CONFIG_SDL_2_0 $BUILD_CONFIG_SFML_1_6 $BUILD_CONFIG_SFML_2_0
do
	echo "- Erasing contents of build/backend-makefiles/$BUILD_CONFIG folder (except configure.sh)..."
	mv build/backend-makefiles/$BUILD_CONFIG/configure.sh .
	rm build/backend-makefiles/$BUILD_CONFIG/* -rf
	mv configure.sh build/backend-makefiles/$BUILD_CONFIG/
	
	echo "- Performing make clean in $BUILD_CONFIG folder..."
	cd $BUILD_CONFIG
	make clean
	cd ..
	
	echo "- Copying makefiles from $BUILD_CONFIG folder to build/ folder (without overwriting)..."
	cp $BUILD_CONFIG/* -r -n build/
	
	echo "- Replacing current folder with generic BUILD_PATH token..."
	patch_makefiles ./build/ ${PWD} BUILD_PATH
	
	echo "- Moving makefiles specific to $BUILD_CONFIG configuration to build/backend-makefiles/$BUILD_CONFIG folder..."
	mv build/makefile   build/backend-makefiles/$BUILD_CONFIG/
	mv build/objects.mk build/backend-makefiles/$BUILD_CONFIG/
	mv build/sources.mk build/backend-makefiles/$BUILD_CONFIG/
	
	echo "Done dealing with $BUILD_CONFIG"
done

echo "Done."
