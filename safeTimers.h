/*
 * safeTimers.h (original name timers.h) is developed by Erik
 * 
 * Willem Aandewiel made some small changes due to the "how can I handle the millis() rollover"
 * by Edgar Bonet and added CHANGE_INTERVAL() and RESTART_TIMER() macro's
 *
 * see: https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
 * 
 * DECLARE_TIMER(timername, interval)
 *  Declares two unsigned longs: 
 *    <timername>_last for last execution
 *    <timername>_interval for interval in seconds
 *    
 *    
 * DECLARE_TIMER_MS is same as DECLARE_TIMER **but** uses milliseconds!
 *    
 * DUE(timername) 
 *  returns false (0) if interval hasn't elapsed since last DUE-time
 *          true (current millis) if it has
 *  updates <timername>_last
 *  
 *  Usage example:
 *  
 *  DECLARE_TIMER(screenUpdate, 200) // update screen every 200 ms
 *  ...
 *  loop()
 *  {
 *  ..
 *    if ( DUE(screenUpdate) ) {
 *      // update screen
 *    }
 * 
 * CHANGE_INTERVAL(timername, interval)
 *  Changes the unsigned longs declared by DECLARE_TIMER(): 
 *    <timername>_last for last execution
 *    <timername>_interval for interval in seconds
 *    
 *    // to change the screenUpdate interval:
 *    CHANGE_INTERVAL(screenUpdate, 500);  // change interval to 500 ms
 *    
 * RESTART_TIMER(timername)
 *  Changes the unsigned long declared by DECLARE_TIMER(): 
 *    <timername>_last = millis()
 *    
 *    // to restart the screenUpdate interval:
 *    RESTART_TIMER(screenUpdate);        // restart timer so next DUE is in 500ms
 *  }
 *  
 */
#define DECLARE_TIMER_MIN(timerName, timerTime)   static uint32_t timerName##_interval = (timerTime * 60 * 1000), \
                                                  timerName##_last = millis();
#define DECLARE_TIMER_SEC(timerName, timerTime)   static uint32_t timerName##_interval = (timerTime * 1000),      \
                                                  timerName##_last = millis();
#define DECLARE_TIMER_MS(timerName, timerTime)    static uint32_t timerName##_interval = timerTime,               \
                                                  timerName##_last = millis();
#define DECLARE_TIMER   DECLARE_TIMER_SEC

#define CHANGE_INTERVAL_MIN(timerName, timerTime) timerName##_interval = (timerTime * 60 * 1000);    
#define CHANGE_INTERVAL_SEC(timerName, timerTime) timerName##_interval = (timerTime * 1000);
#define CHANGE_INTERVAL_MS(timerName, timerTime)  timerName##_interval = timerTime;             
#define CHANGE_INTERVAL CHANGE_INTERVAL_SEC

#define RESTART_TIMER(timerName)                  timerName##_last = millis(); 

#define TIME_LEFT_MIN(timerName)                  (uint32_t)((timerName##_interval + timerName##_last - millis()) / 60 / 1000)
#define TIME_LEFT_SEC(timerName)                  (uint32_t)((timerName##_interval + timerName##_last - millis())/ 1000)
#define TIME_LEFT_MS(timerName)                   (uint32_t)( timerName##_interval + timerName##_last - millis())
#define TIME_LEFT TIMER_LEFT_SEC

#define SINCE(timerName)                          ((uint32_t)(millis() - timerName##_last))

#define DUE_OLD(timerName)                        (( SINCE(timerName) < timerName##_interval) ? 0 : (timerName##_last = millis()))         
#define DUE(timerName)                            (( SINCE(timerName) < timerName##_interval) ? 0 : (timerName##_last = timerName##_last + timerName##_interval))   

/*
 * DUE Effect: This makes sure timer gets restarted after it is DUE moment, so if <PROCESS> inbetween takes longer the timer just accepts that.
 * 
 * So this means, let's say you setup a timer interval off 10 seconds.
 * Timer starts, then loops thru the DUE statement every 3 seconds, because of procesing that happend.
 * Then the following happend:
 * 0:00 - timer resets to 0
 * 0:00 - processing happens for 3 seconds
 * 0:03 - DUE (3<10) = false
 * 0:03 - processing happens for 2 seconds
 * 0:05 - DUE (5<10) = false
 * 0:05 - processing happens for 4 seconds
 * 0:09 - DUE (9<10)= false
 * 0:09 - processing happens for 5 seconds
 * 0:14 - DUE(14<10) = true
 * 0:15 - timer resets to 15
 * 0:15 - processing happens for 3 seconds
 * 0:18 - DUE ((18-15)<10) = false
 * 0:18 - processing happens for 2 seconds
 * 0:20 - DUE ((20-15)<10) = false
 * 0:20 - processing happens for 4 seconds
 * 0:24 - DUE ((29<10)= false
 * 0:24 - processing happens for 5 seconds
 * 0:29 - DUE((29-15)<10) = true
 * 0:29 - timer resets to 29
 * etc... you get the point, this is a time drifted interval loop
 * 
 * DUE_INTERVAL Effect: This makes sure the interval time is constant between actual DUE moments.
 * 0:00 - timer resets to 0
 * 0:00 - processing happens for 3 seconds
 * 0:03 - DUE (3<10) = false
 * 0:03 - processing happens for 2 seconds
 * 0:05 - DUE (5<10) = false
 * 0:05 - processing happens for 4 seconds
 * 0:09 - DUE (9<10)= false
 * 0:09 - processing happens for 5 seconds
 * 0:14 - DUE(14<10) = true
 * 0:15 - timer resets to 10
 * 0:15 - processing happens for 3 seconds
 * 0:18 - DUE ((18-10)<8) = false
 * 0:18 - processing happens for 2 seconds
 * 0:20 - DUE ((20-10)<10) = true
 * 0:20 - timer resets to 20
 * This second INTERVAL timer makes sure the timer interval stays constant, independent of the processing inbetween DUE_INTERVAL.
 * Consequence: No time drifting possible
 */
