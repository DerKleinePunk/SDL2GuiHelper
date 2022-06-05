# Problem show SDL2 with 1024x600 with Waveshare 7 Zoll

## Links

https://github.com/raspberrypi/linux/issues/4719

## Solution ?

./testdisplayinfo

with KMS DRM
INFO: Using video target 'KMSDRM'.
INFO: See 1 displays.
INFO: 0: "0" (720x576, (0, 0)), 23 modes.

with KMS DRM and video=HDMI-A-1:1024x600M@60eD in cmdline.txt

DEBUG: Opened DRM FD (3)
DEBUG: Failed to find index of mode attached to the CRTC.
DEBUG: No connected displays found.
DEBUG: error getting KMSDRM displays information

with Fake KMS DRM and hdmi config in config.txt

DEBUG: KMSDRM_VideoInit()
DEBUG: Opening device /dev/dri/card1
DEBUG: Opened DRM FD (3)
DEBUG: Failed loading udev_device_get_action: /usr/local/lib/libSDL2-2.0.so.0: undefined symbol: _udev_device_get_action
DEBUG: Failed to get evdev touchscreen name
INFO: Using video target 'KMSDRM'.
INFO: See 1 displays.
INFO: 0: "0" (1024x600, (0, 0)), 1 modes.
DEBUG: That operation is not supported


## SDL2

wget -N https://www.libsdl.org/release/$DIRECTORY.tar.gz


wget -N https://www.libsdl.org/projects/SDL_image/release/SDL2_image-2.0.5.tar.gz

wget -N https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.0.18.tar.gz
tar -xzf SDL2_ttf-2.0.18.tar.gz
cd SDL2_ttf-2.0.18
./autogen.sh
./configure
make -j$(nproc)
sudo make install