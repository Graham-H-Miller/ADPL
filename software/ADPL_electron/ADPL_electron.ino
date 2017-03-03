/* ADPL_complete.ino
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
#define FLOW_DELAY 1000     // 2.5 min b/w flow calculations

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
#define INCINERATE_LOW_TEMP 68  // will be 68 in field
#define INCINERATE_HIGH_TEMP 72 // will be 72 in field
#define IGNITE_DELAY 900000     // 15 min between ignitor fires with open valve

#include "Pump.h"
Pump pump(PUMP);
#define KEEP_PUMP_ON_TIME 10000     // 10s on every 30min
#define KEEP_PUMP_OFF_TIME 1790000   // 30min-10s off time (29 min 50s)

#include "Bucket.h"
Bucket bucket(BUCKET);

#include "PinchValve.h"
PinchValve pinchvalve(DIR, STEP, SLEEP);

// initialize some time counters
unsigned long currentTime = 0;
unsigned long last_publish_time = 0;
int temp_count = 1;


void setup() {
    Serial.begin(9600);
    Particle.variable("currentTime", currentTime);

    // collect the system firmware version to fetch OTA
    SYS_VERSION = System.versionNumber();
    Particle.variable("SYS_VERSION", SYS_VERSION);

    // count bucket tips on one-shot rise
    attachInterrupt(BUCKET, bucket_tipped, RISING);

    // respond to push button interface
    attachInterrupt(UP, up_pushed, FALLING);
    attachInterrupt(DOWN, down_pushed, FALLING);
}

void loop() {
    currentTime = millis();

    // rotate through temp probes, only reading 1 / loop since it takes 1 s / read
    temp_count = read_temp(temp_count);

    // publish last publish data if greater than last publish time
    if ((currentTime - last_publish_time) > PUBLISH_DELAY) {
        last_publish_time = publish_data(last_publish_time);
    }

    if ((currentTime - bucket.time_last_measured) > FLOW_DELAY) {
        bucket.update();
        if (bucket.flow_rate < 6 && pinchvalve.quarter_turns < 10){
          pinchvalve.shiftUp();
        }
        else if (bucket.flow_rate > 12 && pinchvalve.quarter_turns > -10){
          pinchvalve.shiftDown();
        }
    }

    // measure temp, determine if light gas
    if (tempHTR.temp <= INCINERATE_LOW_TEMP && !valve.gasOn) {
        valve.open();
        delay(100);
        ignitor.fire();
    }

    // turn off gas valve if greater than high temp
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

    // start pumping if both off and greater than off time
    currentTime = millis();
    if (!pump.pumping && (currentTime - pump.offTime) > KEEP_PUMP_OFF_TIME) {
        pump.turnOn();
    }
    // if on keep on until past on time
    else if (pump.pumping) {
        if ((currentTime - pump.onTime) > KEEP_PUMP_ON_TIME) {
            pump.turnOff();
        }
    }

    if(pinchvalve.down) {
        pinchvalve.shiftDown();
    }
    if(pinchvalve.up) {
        pinchvalve.shiftUp();
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


void up_pushed() {
    pinchvalve.up = true;
}

void down_pushed(){
    pinchvalve.down = true;
}
