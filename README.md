# SDL2 GUI Helper

Some classes to build an GUI using SDL2

For now these are just repositories which can be used in different projects without the need to copy/paste the code into the projects source code.

Some examples show how to use them.

![Mainscreen](dokumentation/images/MiniSampleScreen1.png)

Litte Video Displays the Media Data

https://user-images.githubusercontent.com/20199046/172757128-42f9fedc-0681-4619-ba37-226707f851ce.mp4

With Enabled cairo painter
![Progressbar 50 %](dokumentation/images/ProgressBar50.png)

The Map View Build with [LibOsmScout](http://libosmscout.sourceforge.net/)
![Map View](dokumentation/images/MapView.png)

## Creating Map Data

[Importing Mapdata](http://libosmscout.sourceforge.net/tutorials/importing/)

## Defines

LIBOSMSCOUT -> Enable Map Manger & Co

ENABLEAUDIOMANAGER -> Enable Audiomanger
ENABLEMUSIKMANAGER -> Enable Audiomanger need FFMPEG an can Play Mp3 and Flac files

ENABLECAIRO-> Some GUI Elements need Cairo to paint (try at now we muss test)

DEFAULT_SMALL_FONT -> Fontname for small fons

DEFAULT_FONT -> Fontname Default

## Need to Compile

Debian Pakage Names
 build-essential
 libdrm-dev 
 libgbm-dev
 libsamplerate0-dev
 libpng-dev
 libtiff-dev
 libwebp-dev
 libcairo2-dev
 libmarisa-dev
 libxml2-dev
 libpango1.0-dev
 libprotobuf-dev
 protobuf-compiler
 libsqlite3-dev
 

### SDL2
SDL2 -> I always use own Build Versions

wget -N http://www.libsdl.org/release/SDL2-2.0.22.tar.gz
tar -xzf SDL2-2.0.22.tar.gz
cd SDL2-2.0.22
./autogen.sh
On Desktop ./configure --disable-esd --disable-video-x11 

wget -N https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz
tar zxvf SDL2_image-2.0.5.tar.gz
cd SDL2_image-2.0.5

wget -N https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-2.0.4.tar.gz
tar zxvf SDL2_mixer-2.0.4.tar.gz
cd SDL2_mixer-2.0.4
./autogen.sh
./configure
make
sudo make install
cd ..

wget -N https://www.libsdl.org/projects/SDL_net/release/SDL2_net-2.0.1.tar.gz
tar zxvf SDL2_net-2.0.1.tar.gz
cd SDL2_net-2.0.1
./autogen.sh
./configure
make
sudo make install
cd ..

wget -N http://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.18.tar.gz
tar zxvf SDL2_ttf-2.0.18.tar.gz
cd SDL2_ttf-2.0.18
./autogen.sh
./configure
make
sudo make install
cd ..

### libosmscout

git clone https://github.com/Framstag/libosmscout.git
git checkout 8cd316736da94541f272bd3ab8e9f0eb4750e73c

echo buildding libosmscout debug
DIRECTORY="build"
if [ ! -d "$DIRECTORY" ]; then
	mkdir $DIRECTORY
fi
cd $DIRECTORY
cmake .. -DCMAKE_BUILD_TYPE=Debug -DOSMSCOUT_ENABLE_SSE=ON -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_TOOL_OSMSCOUTOPENGL=OFF -DCMAKE_CXX_FLAGS="-Wno-psabi"
cmake --build . -j $(nproc)
sudo cmake --build . --target install
cd ..

cmake .. -DCMAKE_BUILD_TYPE=Release -DOSMSCOUT_ENABLE_SSE=ON -DOSMSCOUT_BUILD_DOC_API=OFF -DOSMSCOUT_BUILD_DEMOS=OFF -DOSMSCOUT_BUILD_TOOL_OSMSCOUTOPENGL=OFF -DCMAKE_CXX_FLAGS="-Wno-psabi"