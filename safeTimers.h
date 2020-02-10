/*
 * safeTimers.h (original name timers.h) is developed by Erik
 * 
 * Willem Aandewiel made some small changes due to the "how can I handle the millis() rollover"
 * by Edgar Bonet and added CHANGE_INTERVAL() and RESTART_TIMER() macro's
 * Robert van den Breemen made some more improvements on how to handle timers safely
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
 *  https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
 */
#define DECLARE_TIMER_MIN(timerName, timerTime) static uint32_t timerName##_interval = (timerTime * 60 * 1000), \
                                                timerName##_last = millis();
#define DECLARE_TIMER_SEC(timerName, timerTime) static uint32_t timerName##_interval = (timerTime * 1000),      \
                                                timerName##_last = millis();
#define DECLARE_TIMER_MS(timerName, timerTime)  static uint32_t timerName##_interval = timerTime,               \
                                                timerName##_last = millis();

#define DECLARE_TIMER   DECLARE_TIMER_SEC

#define CHANGE_INTERVAL_MIN(timerName, timerTime) timerName##_interval = (timerTime * 60 * 1000);    
#define CHANGE_INTERVAL_SEC(timerName, timerTime) timerName##_interval = (timerTime * 1000);
#define CHANGE_INTERVAL_MS(timerName, timerTime)  timerName##_interval = timerTime;             

#define CHANGE_INTERVAL CHANGE_INTERVAL_SEC

#define RESTART_TIMER(timerName)                  timerName##_last = millis(); 

#define SINCE(timerName)  ((int32_t)(millis() - timerName##_last))
#define DUE(timerName) (( SINCE(timerName) < timerName##_interval) ? 0 : (timerName##_last=millis()))
/*
 * 
*/
