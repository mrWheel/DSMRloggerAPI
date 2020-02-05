/*
 * timers.h is developed by Erik
 * 
 * I made some small changes due to the "how can I handle the millis() rollover"
 * and added the CHANGE_TIMER macro's.
 * by Willem aan der Wiel
 * 
 * And some more improvements by Robert van den Breemen
 * 
 * by Edgar Bonet
 * 
 * DECLARE_TIMER(timername, interval)
 *  Declares two unsigned longs: 
 *    <timername>_last for last execution
 *    <timername>_interval for interval in seconds
 *    
 *    
 * DECLARE_TIMERms is same as DECLARE_TIMER **but** uses milliseconds!
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
 *    // to change the screenUpdate interval:
 *    CHANGE_INTERVAL(screenUpdate, 500);  // change interval to 500 ms
 *  }
 *  
 *  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
 */
#define DECLARE_TIMER_MIN(timerName, timerTime) static uint32_t timerName##_interval = (timerTime * 60 * 1000), \
                                                timerName##_last = millis()+random(timerName##_interval); 
#define DECLARE_TIMER(timerName, timerTime)     static uint32_t timerName##_interval = (timerTime * 1000),      \
                                                timerName##_last = millis()+random(timerName##_interval);
#define DECLARE_TIMER_MS(timerName, timerTime)  static uint32_t timerName##_interval = timerTime,               \
                                                timerName##_last = millis()+random(timerName##_interval);

#define DECLARE_TIMER_SEC DECLARE_TIMER

#define CHANGE_INTERVAL_MIN(timerName, timerTime) if (timerName##_interval != timerTime * 60*1000)  \
                                                    {timerName##_interval = timerTime * 60 * 1000;  \
                                                    timerName##_last = millis();}     
#define CHANGE_INTERVAL(timerName, timerTime)     if (timerName##_interval != timerTime * 1000)     \
                                                    {timerName##_interval = timerTime * 1000;       \
                                                    timerName##_last = millis();}
#define CHANGE_INTERVAL_MS(timerName, timerTime)  if (timerName##_interval != timerTime)            \
                                                    {timerName##_interval != timerTime;             \
                                                    timerName##_last = millis();)
#define CHANGE_INTERVAL_SEC CHANGE_INTERVAL


#define SINCE(timerName)  ((int32_t)(millis() - timerName##_last))
#define DUE(timerName) (( SINCE(timerName) < timerName##_interval) ? 0 : (timerName##_last=millis()))

/*
 * 
*/
