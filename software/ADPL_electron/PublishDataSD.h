#ifndef PublishDataSD_h
#define PublishDataSD_h

#include "application.h"
#include "SD/SD.h"

class PublishDataSD {
public:
    bool publish(double HXCI, double HXCO, double HTR, double HXHI, double HXHO,
                 int gasOn, int bucket_tip_count, File sdFile);
    //TODO: implement formatting within publishing class
private:
    char* fmt_string_SD;
    char* fmt_string_no_bucket_SD;
};

#endif
