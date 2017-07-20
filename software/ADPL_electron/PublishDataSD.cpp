#include "application.h"
#include "PublishDataSD.h"
#include "SD/SdFat.h"

bool PublishDataSD::publish(double HXCI, double HXCO, double HTR, double HXHI, double HXHO,
                            int gasOn, int bucket_tip_count, File sdFile){
    char data_str [69];
    int bitsWritten = -1;
    // bucket.tip_count will be ignored if not needed by sprintf
    fmt_string_SD = "HXCI:%.1f,HXCO:%.1f,HTR:%.1f,HXHI:%.1f,HXHO:%.1f,V:%d,B:%d";
    fmt_string_no_bucket_SD = "HXCI:%.1f,HXCO:%.1f,HTR:%.1f,HXHI:%.1f,HXHO:%.1f,V:%d";

    sprintf(data_str, (bucket_tip_count > 0) ? fmt_string_SD : fmt_string_no_bucket_SD,
            HXCI, HXCO, HTR, HXHI, HXHO, gasOn, bucket_tip_count);

    // open the file for write at end like the "Native SD library"
    if (!sdFile.open("adpl_data.txt", O_RDWR | O_CREAT | O_AT_END)) {
        Log.error("opening file for write failed.");
    }
    // if the file opened okay, write to it:
    // print time
    sdFile.print("[");
    sdFile.print(Time.hour());
    sdFile.print(":");
    sdFile.print(Time.minute());
    sdFile.print(":");
    sdFile.print(Time.second());
    sdFile.print("]");
    // print data
    bitsWritten = sdFile.println(data_str);

    sdFile.close();
    //Indicates publishing success
    if(bitsWritten > -1){
        return true;
    } else {
        return false;
    }

}

bool PublishDataSD::pushToCell(File sdFile) {
    // open the file for write at end like the "Native SD library"
    if (!sdFile.open("adpl_data.txt", O_RDWR)) {
        Log.error("opening file for write failed.");
        return false;
    }
    // initialize a bool to determine success
    bool success = true;
    // read each line in the file and subsequently publish it
    char data_str[69];
    size_t n;
    while ((n = sdFile.fgets(data_str, sizeof(data_str))) > 0) {
        //determine if reading the end of a line
        if (data_str[n - 1] != '\n') {
            Log.error("Line in data file did not end with newline character.");
            return false;
        }
        if(!Particle.publish("SD DATA", data_str)){
            // if publish failed
            success = false;
        }
    }
    // close the file:
    sdFile.close();
    // return the success bool
    return success;
}

// All of the following code is refactored from RemoveLinesFromFile.ino by Andrew Mascolo
void PublishDataSD::removeLinesFromFile(SdFat SD, char* fileName, int sLine, int eLine) {
    File myFileOrig;
    File myFileTemp;

    // If by some chance it exists, remove the temp file from the card
    if (SD.exists("tempFile.txt"))
        SD.remove("tempFile.txt");

    // Get the start and end of line positions from said file
    LinesAndPositions FileLines = FindLinesAndPositions(SD, fileName);

    if ((sLine > FileLines.NumberOfLines) || (eLine > FileLines.NumberOfLines))
        return;

    myFileTemp.open("tempFile.txt", O_WRITE);
    myFileOrig.open(fileName, O_RDWR);
    int position = 0;

    if (myFileOrig)
    {
        while (myFileOrig.available())
        {
            char C = myFileOrig.read();

            // Copy the file but exclude the entered lines by user
            if ((position < FileLines.SOL[sLine - 1]) || (position > FileLines.EOL[eLine - 1]))
                myFileTemp.println(C); //may need to be print, requires testing

            position++;
        }
        myFileOrig.close();
        myFileTemp.close();
    }

    //copy the contents of tempFile back to the original file
    CopyFiles(SD, fileName, "tempFile.txt");

    // Remove the tempfile from the SD card
    if (SD.exists("tempFile.txt"))
        SD.remove("tempFile.txt");
}

void PublishDataSD::CopyFiles(SdFat SD, char* ToFile, char* FromFile) {
    File myFileOrig;
    File myFileTemp;

    if (SD.exists(ToFile))
        SD.remove(ToFile);

    //Serial << "\nCopying Files. From:" << FromFile << " -> To:" << ToFile << "\n";

    myFileTemp.open(ToFile, O_WRITE);
    myFileOrig.open(FromFile, O_RDWR);
    if (myFileOrig)
    {
        while (myFileOrig.available())
        {
            myFileTemp.write( myFileOrig.read() ); // make a complete copy of the original file
        }
        myFileOrig.close();
        myFileTemp.close();
        //Serial.println("done.");
    }
}

PublishDataSD::LinesAndPositions PublishDataSD::FindLinesAndPositions(SdFat SD, char *filename) {
    File myFile;
    LinesAndPositions LNP;

    myFile.open(filename, O_RDWR);
    if (myFile) {
        LNP.NumberOfLines = 0;
        LNP.SOL[0] = 0; //the very first start-of-line index is always zero
        int i = 0;

        while (myFile.available()) {
            if (myFile.read() == '\n') // read until the newline character has been found
            {
                LNP.EOL[LNP.NumberOfLines] = i; // record the location of where it is in the file
                LNP.NumberOfLines++; // update the number of lines found
                LNP.SOL[LNP.NumberOfLines] =
                        i + 1; // the start-of-line is always 1 character more than the end-of-line location
            }
            i++;
        }
        LNP.EOL[LNP.NumberOfLines] = i; // record the last locations
        LNP.NumberOfLines += 1; // record the last line

        myFile.close();
    }
    return LNP;
}
