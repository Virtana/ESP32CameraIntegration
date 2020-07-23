
#include "FreeRTOS.h"

#include "abr_apriltags.h"

#include "apriltag.h"
#include "tag36h11.h"
#include "zarray.h"


#include "iot_system_init.h"
#include "iot_logging_task.h"
#include "esp_system.h"

void detect_apriltags(camera_fb_t* fb)
{   
    image_u8_t* image = NULL;

    //See apriltag/common/image_u8.c
    //this function calls image_u8_create_stride() with a calculated stride value.
    image = image_u8_create_alignment(fb->width,fb->height,32); // set alignment to 32 to remove all line padding (i.e. stride = width) (32-bit cpu and 8 bit data in image... no padding requied)

    if(image == NULL)
    {
        configPRINTF(("Failed to create u8 image\n"));
        return;
    }

    //copy fb->buf to image->buf, accounting for any line padding in image->buf
    for(int row=0;row<(image->height);row++)
    {
        for(int col=0;col<(image->width);col++)
        {
            image->buf[(row*(image->stride))+col] = fb->buf[(row*(image->width))+col];
        }
    }

    configPRINTF(("WIDTH,HEIGHT,STRIDE = %i,%i,%i\n",image->width,image->height,image->stride));


    apriltag_detector_t* detector = apriltag_detector_create();
    apriltag_family_t* family = tag36h11_create();

    apriltag_detector_add_family_bits(detector,family,1);

    zarray_t* detections = apriltag_detector_detect(detector,image);

    configPRINTF(("Detections: %i\n",zarray_size(detections)));

    if(zarray_size(detections) > 0)
    {
        //print family and unique ID of each apriltag detected in the image
        for(int i=0;i<zarray_size(detections);i++)
        {
            apriltag_detection_t* detection;

            //store i'th term of detections in detection
            zarray_get(detections,i,&detection);

            configPRINTF(("Det: %i => Family: %s | ID: %i\n",i+1,detection->family->name,detection->id));

            apriltag_detection_destroy(detection);
        }
    }

    configPRINTF(("\n"));

    //Free resources
    zarray_destroy(detections);
    image_u8_destroy(image);
    tag36h11_destroy(family);
    apriltag_detector_destroy(detector);

}