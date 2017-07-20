#ifndef PublishDataSD_h
#define PublishDataSD_h

#include "SD/SdFat.h"

class PublishDataSD {
public:
    bool publish(double HXCI, double HXCO, double HTR, double HXHI, double HXHO,
                 int gasOn, int bucket_tip_count, File sdFile);
    bool pushToCell(File sdFile);
    void removeLinesFromFile(SdFat SD, char* fileName, int sLine, int eLine);
private:
    void CopyFiles(SdFat SD, char* ToFile, char* FromFile);
    char* fmt_string_SD;
    char* fmt_string_no_bucket_SD;
    struct LinesAndPositions
    {
        int NumberOfLines; // number of lines in file
        int SOL[50]; // start of line in file
        int EOL[50]; // end of line in file
    };
    LinesAndPositions FindLinesAndPositions(SdFat SD, char* filename);
};

#endif
