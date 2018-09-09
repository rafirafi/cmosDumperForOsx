// cmosDumperForOsx
// have fun dumping your cmos
// Copyright 2012 rafirafi
// License WTFPL

#include <iostream>
#include <IOKit/IOKitLib.h>

bool connectToKext(void);
kern_return_t readRtcData(unsigned char * buffer,int offset, int length);

static io_connect_t userClient;

#define kAppleRTCUserClientReadBytes 0

int main (int argc, char * const argv[])
{
    int i, offset, length;
    unsigned char * buffer;
    kern_return_t kernRet;

    // Basic check for the arguments
    if (argc != 3 || strlen (argv[1]) > 3 || strlen (argv[2]) > 3)
        goto fail;

    offset = atoi(argv[1]);
    length = atoi(argv[2]);

    if (offset < 0 || offset >= 256 || length <= 0 || length > 256 || offset + length > 256)
        goto fail;

    // Connect to kernel extension
    if (!connectToKext())
    {
        printf("sorry, could not connect to AppleRTC\n");
        return 1;
    }
    sleep(1); // just a little time to let the kernel notification handlers finish

    // Read rtc
    // allocate buffer
    buffer = (unsigned char *)calloc(length,1);
    if (buffer == NULL)
    {
        printf("alloc problem\n");
        return 1;
    }

    kernRet = readRtcData( buffer, offset, length);
    if (kernRet == KERN_SUCCESS)
    {
        // output rtc content to stdio
        for (i = 0; i < length; i++)
        {
            if (buffer[i] == 0)
                printf("offset:\t%d \tvalue: 0x00\n", offset +i);
            else if(buffer[i] <= 0x0f)
                printf("offset:\t%d \tvalue: 0x0%x\n", offset +i, buffer[i]);
            else
                printf("offset:\t%d \tvalue: 0x%x\n", offset +i, buffer[i]);
        }
    }
    else
    {
        printf("Can't read cmos, please check parameters\n");
        printf("Don't use offset + length > 128 if you have only one bank available\n");
    }

    free(buffer);

    // Disconnect from kernel extension
    IOServiceClose(userClient);

    return 0;

fail:
    printf("This program needs 2 argument : Offset and Length \nUsage: \n .%s 2 36\n", argv[0]);
    printf("\t Offset + Length should not exceed the size \n\t of the cmos ram : 256 or 128 bytes (if only 1 bank)\n");
    return 1;
}

// connectToKext
bool connectToKext()
{
    mach_port_t		masterPort;
    io_service_t	serviceObject = 0;
    io_iterator_t 	iterator;
    CFDictionaryRef	classToMatch;
    Boolean			result = true; // assume success

    // return the mach port used to initiate communication with IOKit
    if (IOMasterPort(MACH_PORT_NULL, &masterPort) != KERN_SUCCESS)
    {
        return false;
    }

    // need to match rtc to get userClient instance
    classToMatch = IOServiceMatching("AppleRTC");
    if (!classToMatch)
    {
        return false;
    }

    // create an io_iterator_t of all instances of our driver's class that exist in the IORegistry
    if (IOServiceGetMatchingServices(masterPort, classToMatch, &iterator) != KERN_SUCCESS)
    {
        return false;
    }

    // get the first item in the iterator.
    serviceObject = IOIteratorNext(iterator);

    // release the io_iterator_t now that we're done with it.
    IOObjectRelease(iterator);

    if (!serviceObject){
        result = false;
        goto bail;
    }

    // instantiate the user client
    // 0x0101FACE : local user
    if(IOServiceOpen(serviceObject, mach_task_self(), 0x0101FACE, &userClient) != KERN_SUCCESS) {
        result = false;
        goto bail;
    }

bail:
    if (serviceObject) {
        IOObjectRelease(serviceObject);
    }

    return result;
}

// readRTCData
kern_return_t readRtcData( unsigned char *buffer, int offset, int length)
{
    kern_return_t kernResult;
    size_t lengthRTC   = length;
    uint64_t offsetRTC = offset;

    // IOConnectCallMethod since 10.5 only
    kernResult = IOConnectCallMethod(userClient,
                                     kAppleRTCUserClientReadBytes,
                                     &offsetRTC,    //input
                                     1,             //inputcount
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     buffer,        //output
                                     &lengthRTC);   //outputcount

    return kernResult;
}
