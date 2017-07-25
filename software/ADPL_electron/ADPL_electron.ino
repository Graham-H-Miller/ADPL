/* ADPL_complete.ino // BATCH TEST
 *
 * Master MCU code for all components:
 * + temp probes
 * + relays
 * + bucket tip
 *
 * LICENSE: Apache v2.0 (see LICENSE file)
 *
 * Copyright (c) 2015-2016 Mark Palmeri (Duke University)
 */

SYSTEM_THREAD(ENABLED);  // parallel user and system threads
// to allow function w/o cell connectivity

unsigned long SYS_VERSION;

#define PUBLISH_DELAY 150000  // 2.5 min b/w variable publish

#include "pin_mapping.h"

#include "TempProbe.h"
TempProbe tempHXCI("HXCI", HXCI);
TempProbe tempHXCO("HXCO", HXCO);
TempProbe tempHTR("HTR", HTR);
TempProbe tempHXHI("HXHI", HXHI);
TempProbe tempHXHO("HXHO", HXHO);

#include "Valve.h"
Valve valve(VALVE);
#include "Ignitor.h"
Ignitor ignitor(IGNITOR);
#define INCINERATE_LOW_TEMP 62  // Lowered due to expected longer, consistent residence time
#define INCINERATE_HIGH_TEMP 66 //
#define IGNITE_DELAY 900000     // 15 min between ignitor fires with open valve

#include "Pump.h"
Pump pump(PUMP);
#define KEEP_PUMP_ON_TIME 10000     // 10s on every 30min
#define KEEP_PUMP_OFF_TIME 1790000   // 30min-10s off time (29 min 50s)

#include "Bucket.h"
#define VOLUME 325 //325 Central, 320 North
#define OPTIMAL_FLOW 2.25 //5.0 L/hr optimal, varies by location // 25 for bench testing


Bucket bucket(BUCKET, VOLUME, OPTIMAL_FLOW);


#include "PinchValve.h"
PinchValve pinchValve(DIR, STEP, SLEEP, UP, DOWN, RESET);
#define FEEDBACK_RESOLUTION 0.125 // mm of movement 16/turn
#define PUSH_BUTTON_RESOLUTION 1.0 // mm of movement
#define HALF_RESOLUTION 0.5 // mm of movement
#define UNCLOG_RESOLUTION 4.0 // mm of movment
#define MAX_POSITION 10.0 // in mm
#define MIN_POSITION 0.0 // in mm
#define RESET_RISE 1.5 // mm of movement
#define BATCH_MOVEMENT 4 // mm of movement, added for batch tests

// initialize some time counters
unsigned long currentTime = 0;
unsigned long last_publish_time = 0;
int temp_count = 1;
int write_address = 0;

void setup() {
    Serial.begin(9600);
    pinchValve.position = EEPROM.get(write_address, pinchValve.position);
    Particle.variable("currentTime", currentTime);
    // count bucket tips on one-shot rise
    attachInterrupt(BUCKET, bucket_tipped, RISING);
    // collect the system firmware version to fetch OTA
    SYS_VERSION = System.versionNumber();
    Particle.variable("SYS_VERSION", SYS_VERSION);
    // sense buttons
    attachInterrupt(UP, up_pushed, FALLING);
    attachInterrupt(DOWN, down_pushed, FALLING);
    attachInterrupt(RESET, res_pushed, FALLING);

    // initialize the pinch valve variables for raise and time tracking
    pinchValve.isRaised = false;
    pinchValve.lastTime = millis();
}

void loop() {
    // read the push buttons
    currentTime = millis();

    // rotate through temp probes, only reading 1 / loop since it takes 1 s / read
    temp_count = read_temp(temp_count);
    if ((currentTime - last_publish_time) > PUBLISH_DELAY) {
        last_publish_time = publish_data(last_publish_time);
    }

    // measure temp, determine if light gas
    if (tempHTR.temp <= INCINERATE_LOW_TEMP && !valve.gasOn) {
        valve.open();
        delay(100);
        ignitor.fire();
    }

    if(valve.gasOn) {
        currentTime = millis();
        if (tempHTR.temp >= INCINERATE_HIGH_TEMP) {
            valve.close();
        }
            // if 15 min have elapsed since last ignitor fire, then fire again
        else if((currentTime - ignitor.timeLastFired) > IGNITE_DELAY) {
            ignitor.fire();
        }
    }

    currentTime = millis();
    if (!pump.pumping && (currentTime - pump.offTime) > KEEP_PUMP_OFF_TIME) {
        pump.turnOn();
    }
    else if (pump.pumping) {
        if ((currentTime - pump.onTime) > KEEP_PUMP_ON_TIME) {
            pump.turnOff();
        }
    }

    // flag variables changed in attachInterrupt function
    if(pinchValve.down) {
        pinchValve.shiftDown(pinchValve.resolution);
        EEPROM.put(write_address, pinchValve.position);
    }
    if(pinchValve.up) {
        pinchValve.shiftUp(pinchValve.resolution);
        EEPROM.put(write_address, pinchValve.position);
    }

    // unclog if no tip in a long while
    // open all the way up and come back to optimum

    /////// COMMENTED OUT FOR BATCH TESTS ////////
    //  currentTime = millis();
    //  if ((currentTime - bucket.lastTime) > (2 * bucket.lowFlow)) {
    //      pinchValve.unclog(UNCLOG_RESOLUTION);
    //      bucket.lastTime = currentTime; // commented out for tests

    //      if(pinchValve.clogCounting >= 2 && pinchValve.position < MAX_POSITION){
    //          pinchValve.up = true;
    //          pinchValve.resolution = PUSH_BUTTON_RESOLUTION;
    //      }

    //  }
    /////// COMMENTED OUT FOR BATCH TESTS ////////

    //  if(bucket.tip) {
    //      pinchValve.clogCounting = 0;
    //      bucket.updateFlow();
    //      if (bucket.tipTime < bucket.highFlow && bucket.tipTime > bucket.highestFlow && pinchValve.position > MIN_POSITION) {
    //        pinchValve.down = true;
    //        pinchValve.resolution = FEEDBACK_RESOLUTION;
    //      }
    //      else if (bucket.tipTime > bucket.lowFlow && pinchValve.position < MAX_POSITION){
    //        pinchValve.up = true;
    //        pinchValve.resolution = FEEDBACK_RESOLUTION;
    //      }
    //      else if (bucket.tipTime < bucket.highestFlow){
    //        pinchValve.down = true; // handles sudden large flow
    //        pinchValve.resolution = HALF_RESOLUTION;
    //      }
    //  }

    /////// COMMENTED OUT FOR BATCH TESTS ////////

    ////THIS IS THE NEW STUFF////////

    currentTime = millis();
    if(((currentTime - pinchValve.lastTime) > ((3600*VOLUME)/OPTIMAL_FLOW)) && (!pinchValve.isRaised)) {  // Gives all times in ms
        //  (3600 * VOLUME) *  (1 / OPTIMAL_FLOW)
        pinchValve.up = true;
        pinchValve.resolution = BATCH_MOVEMENT; // 3mm , make variable
        pinchValve.lastTime = millis();
        pinchValve.isRaised = true;
    }
    if(bucket.tip) {
        if (pinchValve.isRaised) {
            pinchValve.down = true;
            pinchValve.resolution = BATCH_MOVEMENT;
        }
        bucket.tip = false;
        pinchValve.isRaised = false;
        pinchValve.lastTime = millis();
    }
}

int read_temp(int temp_count) {
    switch (temp_count) {
        case 1:
            tempHXCI.read();
            temp_count++;
            break;
        case 2:
            tempHXCO.read();
            temp_count++;
            break;
        case 3:
            tempHTR.read();
            temp_count++;
            break;
        case 4:
            tempHXHI.read();
            temp_count++;
            break;
        case 5:
            tempHXHO.read();
            temp_count = 1;
            break;
    }

    return temp_count;
}

void bucket_tipped() {
    bucket.tipped();
    bucket.tip = true;
}

int publish_data(int last_publish_time) {
    bool publish_success;
    char data_str [69];

    // allow for data str to be created that doesn't update bucket if count = 0
    const char* fmt_string = "HXCI:%.1f,HXCO:%.1f,HTR:%.1f,HXHI:%.1f,HXHO:%.1f,V:%d,B:%d";
    const char* fmt_string_no_bucket = "HXCI:%.1f,HXCO:%.1f,HTR:%.1f,HXHI:%.1f,HXHO:%.1f,V:%d";

    // bucket.tip_count will be ignored if not needed by sprintf
    sprintf(data_str, (bucket.tip_count > 0) ? fmt_string : fmt_string_no_bucket,
            tempHXCI.temp, tempHXCO.temp, tempHTR.temp, tempHXHI.temp, tempHXHO.temp,
            int(valve.gasOn), int(bucket.tip_count));

    publish_success = Particle.publish("DATA",data_str);

    if (publish_success) {
        last_publish_time = currentTime;
        // reset the bucket tip count after every successful publish
        // (webserver will accumulate count)
        bucket.tip_count = 0;
    }
    return last_publish_time;
}

void res_pushed(){
    pinchValve.position = 0.0;
//  pinchValve.up = true; // Commented out for Batch Tests
//  pinchValve.resolution = RESET_RISE; // Commented out for Batch Tests
//  bucket.lastTime = millis();
    pinchValve.lastTime = millis();
    pinchValve.isRaised = false;
}

void up_pushed() {
    pinchValve.up = true;
    pinchValve.resolution = PUSH_BUTTON_RESOLUTION;
}

void down_pushed(){
    pinchValve.down = true;
    pinchValve.resolution = PUSH_BUTTON_RESOLUTION;
}
