#ifndef Bucket_h
#define Bucket_h

#include "application.h"

class Bucket {
    public:
        Bucket(int pin);
        void tipped();
        unsigned int tip_count;
        void publish();
        double updateFlow();
        double flow_rate;
    private:
        static constexpr _NUMSAMPLES int 6; //number of samples of bucket data
        double _bucket_array[_NUMSAMPLES]; //array to read and average over

        int iter;
};

#endif
