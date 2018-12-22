#include <hardware/gps.h>
#include <utils/Log.h>

#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#define GPS_LIB_PATH "/system/lib/libagps_hal.so"

static void* libagps_hal = NULL;
static GpsInterface* gpsInterface = NULL;
static GpsInterface* (*gps_get_hardware_interface)();

const GpsInterface* gps_get_gps_interface(struct gps_device_t* dev)
{
    if( gpsInterface == NULL )
    {
        ALOGI("Using 32bits GPS lib");
        ALOGI("full_path of gps lib is : %s", GPS_LIB_PATH);
        libagps_hal = dlopen(GPS_LIB_PATH, RTLD_LOCAL);
        if( libagps_hal == NULL )
        {
            ALOGE("dlopen failed: %s", dlerror());
            return NULL;
        }
        gps_get_hardware_interface = (GpsInterface* (*)())dlsym(libagps_hal, "gps_get_hardware_interface");
        if( gps_get_hardware_interface == NULL )
        {
            ALOGE("Symbol 'gps_get_hardware_interface' was not found in %s", GPS_LIB_PATH);
        }
        else
        {
            gpsInterface = gps_get_hardware_interface();
            ALOGI("Looking for GPS hardware ...");
            if( gpsInterface != NULL )
            {
                ALOGI("GPS hardware found on this device");
            }
            else
            {
                ALOGE("Error: no GPS hardware on this device");
            }
        }
    }
    return gpsInterface;
}

static int mrvl_open_gps(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    struct gps_device_t *dev = (struct gps_device_t *) malloc(sizeof(struct gps_device_t));

    if(dev == NULL)
        return -1;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->get_gps_interface = gps_get_gps_interface;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t gps_module_methods = {
    .open = mrvl_open_gps
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = 1,
    .hal_api_version = 0,
    .id = GPS_HARDWARE_MODULE_ID,
    .name = "MRVL GPS Module",
    .author = "Marvell",
    .methods = &gps_module_methods,
};
