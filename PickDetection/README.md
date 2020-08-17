# ESP EYE Camera Based Pick Detection 

The [ESP-EYE]([https://www.espressif.com/en/products/devkits/esp-eye/overview](https://www.espressif.com/en/products/devkits/esp-eye/overview))  is fitted with an on-board camera for image acquisition. This project submodule achieves motion detection solely through the use of camera vision and image processing as a cost effective and lightweight alternative to using motion specific sensors. 

### Image Processing and Motion Detection 
The motion detection algorithm implements the principles as detailed [here](https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/#tocwhat-is-naive-motion-detection). This Arduino implementation for the ESP32 cam was modelled for application using AFR on an ESP-EYE. 

The operating principle is the comparison of consecutive camera frames. The frames are downsampled initially, to decrease the processing time for an application that is meant to run real-time.  With predefined thresholds, both the `BLOCK_DIFF_THRESHOLD` and `IMAGE_DIFF_THRESHOLD`, and choice of block size in pixelating the image,  the sensitivity of detection can be varied to account for application context and noise.


### Workspace Setup
##### 1. Clone Repository
* Choose your directory for cloning this repository and run `$ git clone {HTTPS/SSH_LINK}`
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

Deactivate by running `deactivate` in terminal. Within the **ESP32CameraIntegration** folder, 
* ` $ pip3 install --index-url=https://pypi.python.org/simple/ -r amazon-freertos/vendors/espressif/esp-idf/requirements.txt`

Before using the code base, please follow [Getting Started with FreeRTOS]([https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html)) to ensure that all necessary prerequisites and configuration (CMake, Toolchain, AWS CLI Configuration etc.) is done. Do note that the packages below may need to be installed prior to performing **Configure the FreeRTOS demo applications >Step 5**. This would all depend on whether a clean virtual environment has been setup as directed.

*  ` $ pip install -U wheel`
*  ` $ pip install tornado`
*  ` $ pip install boto3`

##### 3. Building and Flashing
The code base includes a build executable specific to the ESP-EYE. Within `../ESP32CameraIntegration/PickDetection/`,  build the package using `$./build.sh`  and the package can be made and flashed using `make flash` or `make flash monitor`.

##### *NOTE:*
Unless integration is done with this code base to include additional threads and idle tasks, building the package as is should throw errors pertaining to *vApplicationTickHook* and *vApplicationIdleHook*. This is corrected by altering the **FreeRTOSConfig.h** file located in the *amazon-freetos* submodule present at `~../ESP32CameraIntegration/amazon-freertos/vendors/espressif/boards/esp32/aws_demos/config_files`.

The parameters as defined for Lines 64 and 65 should be changed to match below.

- `#define configUSE_IDLE_HOOK		0`	
- `#define configUSE_TICK_HOOK		0`

Additionally, any errors thrown during runtime pertaining to failure of the camera, its sensor or the inability to retreive frame should be resolved as follows: 

	1. Within the build folder, run $ make menuconfig 
	2. Navigate to Component config > Camera configuration 
	3. Deselect all options except for the OV2640 Support (ESP-EYE Specific) 
	4. Save and Exit
	5. Make and flash



### Configuration for Use

Replace the macro argument in the files to change the following:

##### *pickdet_camera.h*

- pixel format `#define CAMERA_PIXEL_FORMAT {FORMAT}`
- image resolution `#define CAMERA_FRAME_SIZE {RESOLUTION}`

**NOTE:** Comments do list and define all valid selections. `PIXFORMAT_JPEG`does build and run, however motion detection is currently not operational for this format.


##### *pickdet_motion.h*
- Width of selected resolution `#define WIDTH {WIDTH_VALUE}`
- Height of selected resolution `#define HEIGHT {HEIGHT_VALUE}`
- Desired pixelation downscale `#define BLOCK_SIZE {BLOCK_VALUE}`
- Block difference threshold `#define BLOCK_DIFF_THRESHOLD {BLOCK_DIFF_VALUE}`
- Image difference threshold `#define IMAGE_DIFF_THRESHOLD {IMAGE_DIFF_VALUE}`

**NOTE:** The difference thresholds are only valid for 0 < `BLOCK_DIFF_THRESHOLD` < 1 and 0 < `IMAGE_DIFF_THRESHOLD` < 1. The width and height MUST reflect the resolution selected in **pickdet_camera.h**. An automated definition of `WIDTH` and `HEIGHT` depedent upon `CAMERA_FRAME_SIZE` is to be done.

##### *main.c*

Select between code operation by commenting/uncommenting `#define HTTP_STREAM`.When defined, motion detection is coupled task with HTTP streaming to a web server, accessible via your browser with default IP `192.168.4.1/`. When undefined, motion detection is run in tandem with a task which publishes MQTT messages to IOT Core each time motion is detected. 