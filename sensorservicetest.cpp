/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/sensor.h>
#include <gui/Sensor.h>
#include <gui/SensorManager.h>
#include <gui/SensorEventQueue.h>
#include <utils/Looper.h>

using namespace android;

static nsecs_t sStartTime = 0;


int receiver(int fd, int events, void* data)
{
    sp<SensorEventQueue> q((SensorEventQueue*)data);
    ssize_t n;
    ASensorEvent buffer[8];

    static nsecs_t oldTimeStamp = 0;

    while ((n = q->read(buffer, 8)) > 0) {
        for (int i=0 ; i<n ; i++) {
            float t;
            if (oldTimeStamp) {
                t = float(buffer[i].timestamp - oldTimeStamp) / s2ns(1);
            } else {
                t = float(buffer[i].timestamp - sStartTime) / s2ns(1);
            }
            oldTimeStamp = buffer[i].timestamp;

            if (buffer[i].type == Sensor::TYPE_ACCELEROMETER) {
                printf("%lld\t%8f\t%8f\t%8f\t%f\n",
                        buffer[i].timestamp,
                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
                        1.0/t);
            }
            if (buffer[i].type == Sensor::TYPE_GYROSCOPE) {
                printf("%lld\t%8f\t%8f\t%8f\t%f\n",
                        buffer[i].timestamp,
                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
                        1.0/t);
            }
            if (buffer[i].type == Sensor::TYPE_MAGNETIC_FIELD) {
                printf("%lld\t%8f\t%8f\t%8f\t%f\n",
                        buffer[i].timestamp,
                        buffer[i].data[0], buffer[i].data[1], buffer[i].data[2],
                        1.0/t);
            }

        }
    }
    if (n<0 && n != -EAGAIN) {
        printf("error reading events (%s)\n", strerror(-n));
    }
    return 1;
}


int main(int argc, char** argv)
{
    SensorManager& mgr(SensorManager::getInstance());

    Sensor const* const* list;
    ssize_t count = mgr.getSensorList(&list);
    printf("numSensors=%d\n", int(count));

    enum { SLACC, SLGYR, SLMAG };
    //select one sensor
    int Sel = 0;
    if(argc==1) {
	if(strcmp(argv[1],"acc"))
		Sel = 0;
	else if(strcmp(argv[1],"gyro"))
		Sel = 1;
	else if(strcmp(argv[1],"mag"))
		Sel = 2;
    }
    else {
	printf("please select a sensor: (0:acc//1:gyro//2:mag)");
	scanf("%d", &Sel);
    }

    sp<SensorEventQueue> q = mgr.createEventQueue();
    Sensor const* accelerometer = mgr.getDefaultSensor(Sensor::TYPE_ACCELEROMETER);
    Sensor const* gyroscope = mgr.getDefaultSensor(Sensor::TYPE_GYROSCOPE);
    Sensor const* magnetometer = mgr.getDefaultSensor(Sensor::TYPE_MAGNETIC_FIELD);
    printf("queue=%p\n", q.get());

    switch(Sel)
    {
	case SLACC:
		printf("The selected sensor is Acc\n");
		printf("accelerometer=%p (%s)\n",
		accelerometer, accelerometer->getName().string());
		sStartTime = systemTime();
		q->enableSensor(accelerometer);
		q->setEventRate(accelerometer, ms2ns(10));
		break;
	case SLGYR:
		printf("The selected sensor is Gyro\n");
		printf("gyroscope=%p (%s)\n",
		gyroscope, gyroscope->getName().string());
		sStartTime = systemTime();
		q->enableSensor(gyroscope);
		q->setEventRate(gyroscope, ms2ns(10));
		break;
	case SLMAG:
		printf("The selected sensor is Mag\n");
		printf("magnetometer=%p (%s)\n",
		magnetometer, magnetometer->getName().string());
		sStartTime = systemTime();
		q->enableSensor(magnetometer);
		q->setEventRate(magnetometer, ms2ns(10));
		break;
    }



    sp<Looper> loop = new Looper(false);
    loop->addFd(q->getFd(), 0, ALOOPER_EVENT_INPUT, receiver, q.get());

    do {
        //printf("about to poll...\n");
        int32_t ret = loop->pollOnce(-1);
        switch (ret) {
            case ALOOPER_POLL_WAKE:
                //("ALOOPER_POLL_WAKE\n");
                break;
            case ALOOPER_POLL_CALLBACK:
                //("ALOOPER_POLL_CALLBACK\n");
                break;
            case ALOOPER_POLL_TIMEOUT:
                printf("ALOOPER_POLL_TIMEOUT\n");
                break;
            case ALOOPER_POLL_ERROR:
                printf("ALOOPER_POLL_TIMEOUT\n");
                break;
            default:
                printf("ugh? poll returned %d\n", ret);
                break;
        }
    } while (1);


    return 0;
}
