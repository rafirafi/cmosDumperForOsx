A cmos reader for osx, displaying the kext's method to read cmos from userland.  
  
usage :  
```
./cmosDumperForOsx offset length
```
ex:  
```
./cmosDumperForOsx 5 52
```
will output the 52 bytes from pos 5 of the cmos to stdio.  
  
Tested only with SL.  
  
  
Copyright 2012 rafirafi.  
Posted to projectosx before it was lost.  
  
In a more compact and clean form there is the fetchCMOS function from :  
https://sourceforge.net/p/dpcimanager/code/ci/master/tree/DPCIManager/AppDelegate.m
  
//  Created by PHPdev32 on 9/12/12  
//  Licensed under GPLv3, full text at http://www.gnu.org/licenses/gpl-3.0.txt  

```
-(IBAction)fetchCMOS:(id)sender{
    NSRange range = NSMakeRange(0, 128);
    unsigned char buff[range.length];
    NSMutableData *cmos = [NSMutableData data];
    io_service_t service;
    if ((service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceMatching("AppleRTC")))) {
        io_connect_t connect;
        if (IOServiceOpen(service, mach_task_self(), 0x0101FACE, &connect) == KERN_SUCCESS){
            while(IOConnectCallMethod(connect, 0, (uint64_t *)&range.location, 1, NULL, 0, NULL, NULL, buff, &range.length) == KERN_SUCCESS) {
                [cmos appendBytes:buff length:range.length];
                range.location += 128;
            }
            IOServiceClose(connect);
        }
        IOObjectRelease(service);
    }
}
```


