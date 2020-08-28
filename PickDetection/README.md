# ESP EYE Camera Based Pick Detection 

The [ESP-EYE]([https://www.espressif.com/en/products/devkits/esp-eye/overview](https://www.espressif.com/en/products/devkits/esp-eye/overview))  is fitted with an on-board camera for image acquisition. This project submodule achieves motion detection solely through the use of camera vision and image processing as a cost effective and lightweight alternative to using motion specific sensors. 

### Image Processing and Motion Detection 
The motion detection algorithm implements the principles as detailed [here](https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/#tocwhat-is-naive-motion-detection). This Arduino implementation for the ESP32 cam was modelled for application using AFR on an ESP-EYE. 

The operating principle is the comparison of consecutive camera frames. The frames are downsampled initially, to decrease the processing time for an application that is meant to run real-time.  With predefined thresholds, both the `BLOCK_DIFF_THRESHOLD` and `IMAGE_DIFF_THRESHOLD`, and choice of block size in pixelating the image,  the sensitivity of detection can be varied to account for application context and noise.


### Workspace Setup (Linux Only)
##### 1. Clone Repository
* Choose your directory for cloning this repository and run `$ git clone --recursive {HTTPS/SSH_LINK}`
##### 2. Prerequisite Installation
Run these terminal commands:

	1.	$ sudo apt update
	2.	$ sudo apt install build-essential
	3.	$ sudo apt install python3-pip
	4.	$ python3 -m pip install --user virtualenv
	5.	$ sudo apt-get install python3-venv

Create a folder for virtual environments. To create an environment, within this directory:
* ` $ python3 -m venv {VIRTUAL_ENV_NAME}`

To enable the environment:
* ` $ {PATH_TO_VIRTUAL_ENV}/bin/activate`

To deactivate, run `deactivate` in terminal. In the root folder of the repository, run 
* ` $ pip3 install --index-url=https://pypi.python.org/simple/ -r amazon-freertos/vendors/espressif/esp-idf/requirements.txt`

Start by following the [Getting Started with Amazon FreeRTOS](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html#setup-espressif-prereqs) guide. After completing the **Establish a serial connection** section, return to this README. 

##### 3. Configure FreeRTOS Demo 

Follow [Download and configure FreeRTOS](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html#download-and-configure-espressif) and return to this README. FreeRTOS is already included in this repository, so <ins>skip the instructions to download it </ins>. 

If you encounter problems with Step 5 of the demo configuration guide, install the following packages by running:

*  ` $ pip install -U wheel`
*  ` $ pip install tornado`
*  ` $ pip install boto3`

##### 4. Building and Flashing
Make the build.sh script in `ESP32CameraIntegration/PickDetection/` executable: `chmod a+x build.sh`

Build the package using `$./build.sh`. This generates the build files in the `build/` folder.

Change directory to this folder using `cd build/` and build the binary and flash to the ESP-EYE using `make flash`. If you wish to view terminal output from the board during runtime, run `make flash monitor` instead. 

##### *NOTE:*
If there are build errors which reference *vApplicationTickHook* and *vApplicationIdleHook*, in the **FreeRTOSConfig.h** file located in `ESP32CameraIntegration/amazon-freertos/vendors/espressif/boards/esp32/aws_demos/config_files/`, change the macro arguments to reflect those below.

- `#define configUSE_IDLE_HOOK		0`	
- `#define configUSE_TICK_HOOK		0`

Additionally, any errors thrown during runtime pertaining to failure of the camera, its sensor or the inability to retrieve frame should be resolved as follows: 

	1. Within the build folder, run $ make menuconfig 
	2. Navigate to Component config > Camera configuration 
	3. Deselect all options except for the OV2640 Support (ESP-EYE Specific) 
	4. Save and Exit
	5. Make and flash



### Configuration for Use

Replace the {macro argument} in the files to change the following:

##### *pickdet_camera.h*

- pixel format `#define CAMERA_PIXEL_FORMAT {FORMAT}`
- image resolution `#define CAMERA_FRAME_SIZE {RESOLUTION}`

**NOTE:** Comments list and define all valid selections. `PIXFORMAT_JPEG`does build and run, however motion detection is currently not operational for this format.


##### *pickdet_motion.h*
- Width of selected resolution `#define WIDTH {WIDTH_VALUE}`
- Height of selected resolution `#define HEIGHT {HEIGHT_VALUE}`
- Desired pixelation downscale `#define BLOCK_SIZE {BLOCK_VALUE}`
- Block difference threshold `#define BLOCK_DIFF_THRESHOLD {BLOCK_DIFF_VALUE}`
- Image difference threshold `#define IMAGE_DIFF_THRESHOLD {IMAGE_DIFF_VALUE}`

**NOTE:** The difference thresholds are only valid for 0 < `BLOCK_DIFF_THRESHOLD` < 1 and 0 < `IMAGE_DIFF_THRESHOLD` < 1. The width and height MUST reflect the resolution selected in **pickdet_camera.h**. An automated definition of `WIDTH` and `HEIGHT` dependent upon `CAMERA_FRAME_SIZE` is to be done.

##### *pickdet_http_display.h*
- Desired Access Point (AP) SSID `#define WIFI_SSID "{SSID}"`
- Desired Access Point (AP) password `#define WIFI_PASSWORD "{PASSWORD}"`
- Desired IP for accessing stream `#define EXAMPLE_IP_ADDR "{#.#.#.#}"`

##### *main.c*

Use the `#define HTTP_STREAM` macro to enable or disable HTTP streaming with motion detection. If streaming is enabled, connect to the ESP-EYE AP via WiFi, open your browser and enter `192.168.4.1/stream` by default to view camera stream.

<ins>Note</ins>: With HTTP streaming enabled, motion detection is only done when connected to the AP. Currently, the operating frame rate is roughly 1-2 fps.
