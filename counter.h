#ifndef COUNTER_H
#define COUNTER_H
#pragma once
/*
 * Dependencies
 */
#include <windows.h>

/**
 * which handles timing.
 * @author Brett Jones
 */
class Counter {
private:
    /**
     * Variables
     */
	DWORD startTime;
    DWORD endTime;
    DWORD targetTime;
public:
    /**
     * Constructor resets the member variables.
     */
	Counter() {	
		reset();
	}
	~Counter() {
		reset();
		targetTime = 0;
	}
    /**
     * set() sets the target time.
     * @param m is the millisecond target time.
     */
    void set(int m) {
        //reset();
        start();
        targetTime = m;
    }
    /**
     * start() starts the timer.
     */
    void start() {
        startTime = GetTickCount();
    }
    /**
     * end() ends the timer.
     */
    void end() {
		endTime = GetTickCount();
    }
    /**
     * isDone() checks the time to see if it meets or exceeds the target time.
     * @return returns true or false.
     */
    bool isDone() {
        end();
        if(targetTime <= duration())
            return true;
        return false;
    }
    /**
     * duration() returns the duration between the start time and end time.
     * @return 
     */
    DWORD duration() {
		return (endTime - startTime);
    }
    /**
     * reset() resets the timer.
     */
    void reset() {
        startTime = 0;
        endTime = 0;
    }
	//let's add a repeat
	void repeat() {
		set(targetTime);
	}
};
#endif
