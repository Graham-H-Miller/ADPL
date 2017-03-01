/*
 * Bucket.cpp - bucket tipping counter
 */

#include "application.h"
#include "Bucket.h"

Bucket::Bucket(int pin) {
    pinMode(pin, INPUT_PULLDOWN);
    unsigned int tip_count = 0;
    Particle.variable("bucket", (int) tip_count);
    iter = 0;
}

void Bucket::tipped() {
    tip_count++;
}

void Bucket::update(){
    iter++;
    time_last_measured = millis();
    _bucket_array[iter%6] = tip_count;
    if (iter!=5){
        flow_rate = 4*(_bucket_array[iter]-_bucket_array[iter+1]);
    }
    else{
        flow_rate = 4*(_bucket_array[5]-_bucket_array[0]);
    }
}

void Bucket::publish() {
    Particle.publish(String("BUCKET"), String(tip_count));
}
