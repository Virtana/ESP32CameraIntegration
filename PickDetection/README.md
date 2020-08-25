# ESP EYE Camera Based Pick Detection 

The [ESP-EYE]([https://www.espressif.com/en/products/devkits/esp-eye/overview](https://www.espressif.com/en/products/devkits/esp-eye/overview))  is fitted with an on-board camera for image acquisition. This project submodule achieves motion detection solely through the use of camera vision and image processing as a cost effective and lightweight alternative to using motion specific sensors. 

### Image Processing and Motion Detection 
The motion detection algorithm implements the principles as detailed [here](https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/#tocwhat-is-naive-motion-detection). This Arduino implementation for the ESP32 cam was modelled for application using AFR on an ESP-EYE. 

The operating principle is the comparison of consecutive camera frames. The frames are downsampled initially, to decrease the processing time for an application that is meant to run real-time.  With predefined thresholds, both the `BLOCK_DIFF_THRESHOLD` and `IMAGE_DIFF_THRESHOLD`, and choice of block size in pixelating the image,  the sensitivity of detection can be varied to account for application context and noise.


### Workspace Setup
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

Deactivate by running `deactivate` in terminal. Within the **ESP32CameraIntegration** folder, 
* ` $ pip3 install --index-url=https://pypi.python.org/simple/ -r amazon-freertos/vendors/espressif/esp-idf/requirements.txt`

Before using the code base, please follow [Getting Started with Amazon FreeRTOS](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html#setup-espressif-prereqs) to ensure that all necessary prerequisites and configuration (IAM User, CMake, Toolchain, etc.) are done. Please do up to and inclusive of **Establish a serial connection**. 

Additionally, configuration for the FreeRTOS demo is required to use the code base as there are overlapping fundamental frameworks such as the AWS CLI that are required. Follow [Download and configure FreeRTOS](https://docs.aws.amazon.com/freertos/latest/userguide/getting_started_espressif.html#download-and-configure-espressif) ONLY. Do note that FreeRTOS is included in this repository as a submodule and <ins>should not be downloaded</ins> as directed in the beginning. The packages below may need to be installed prior to performing **Configure the FreeRTOS demo applications >Step 5**. This would all depend on whether a clean virtual environment has been setup as directed by this README.

*  ` $ pip install -U wheel`
*  ` $ pip install tornado`
*  ` $ pip install boto3`

##### 3. Building and Flashing
The code base assumes a Linux environment with a build executable specific to the ESP-EYE. Within `../ESP32CameraIntegration/PickDetection/`,  build the package using `$./build.sh`. Ensure that [build.sh script](https://github.com/Virtana/ESP32CameraIntegration/blob/master/PickDetection/build.sh) is executable. If not, create its executable using `chmod a+x build.sh`. Within the build folder, the package can be made and flashed using `make flash` or `make flash monitor`. The latter includes a serial monitor in the terminal to view the output from the board at runtime. Run `make monitor` to view the output of runtime for an already flashed board. 

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

Replace the {macro argument} in the files to change the following:

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

**NOTE:** The difference thresholds are only valid for 0 < `BLOCK_DIFF_THRESHOLD` < 1 and 0 < `IMAGE_DIFF_THRESHOLD` < 1. The width and height MUST reflect the resolution selected in **pickdet_camera.h**. An automated definition of `WIDTH` and `HEIGHT` dependent upon `CAMERA_FRAME_SIZE` is to be done.

##### *pickdet_http_display.h*
- Desired Access Point (AP) SSID `#define WIFI_SSID "{SSID}"`
- Desired Access Point (AP) password `#define WIFI_PASSWORD "{PASSWORD}"`
- Desired IP for accessing stream `#define EXAMPLE_IP_ADDR "{#.#.#.#}"`

##### *main.c*

Select between code operation by commenting/uncommenting `#define HTTP_STREAM`.When defined, motion detection is a coupled task with HTTP streaming to a web server. Connect to the ESP-EYE Access Point and access stream via your browser with default URL `192.168.4.1/stream`. When undefined, motion detection is run as an independent task without the option to view. <ins>Note</ins>:The current trade-off is motion detection with HTTP streaming does not provide a satisfactory stream frame rate, so the video is heavily lagged. Additionally, it is not as lightweight when compared to the independent motion detection task. Both modes of operation publishes MQTT messages to IOT Core each time motion is detected. 
