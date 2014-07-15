/*

1.0 - 11/11/2010 - Initial release

1.1 - 11/11/2010 -

    * Made some documentation corrections.

1.2 - 11/11/2010

    * Added user API callback options for teh following signals:-
        1PPS
        NMEA RMC message processed
        NMEA GGA message processed
                
    * Added example1.cpp and example2.cpp to demonstrate how to 
    setup and use various features/functions.

1.3 - 11/11/2010

    * Increased the frequency at which process_request flag to checked.
    
    * Included the baud() and format() methods which are just passed onto
    the parent object, Serial.
    
    * Updated the example1.cpp and example2.cpp to include more details.

1.4 - 13/10/2010

    * Changed the name to MODGPS as "GPS" was too generic.
    
    * Improved the example1.cpp program with a lot more details 
    on the usage of the module.

1.5 - 14/11/2010

    * Added more cookbook documentation.

1.6 - 14/11/2010

    * Added more doxyden API comments. 
    
1.7 - 14/10/2010

    * Added a lot more doxygen comments for teh API group.

1.8 - 14/10/2010

    * Added a lot more doxygen comments for the API group.

1.9 - 15/11/2010

    * Added @ingroup API to getGPSquality()
    
1.10 - 17/11/2010

    * Improved doxygen documentation

1.11 - 17/11/2010

    * Mbed's systems don't like @mainpage, rolling back

1.12 - 15/04/2011

    * Added VTG sentence (vectors speed and direction)
      See example3.cpp

1.13 - 16/04/2011

    * Found that concat commas in NMEA sentences (indicating an empty field)
      caused strtok() to return pointers in incorrect positions. 
    * Found copying geodetic and vtg data structs to temp buffers actually
      didn't work properly.  
      
1.14 - 16/04/2011

    * Doh, left in some debugging code which causes compile fail outside 
      of the test harness.
      
1.15 - 20/04/2011

    * Added the new feature that allows the library to set the Mbed RTC
      to the GPS/UTC time. This ensures the RTC is kept up to date with
      the latest GPS aquired time (syncs once each minute).

1.16 - 21/04/2011

    * Added the ability for an application to request a copy of a NMEA sentence
      before it gets processed (mangled).
      See setRmc(), setGga(), setVtg() and setUkn().
            
*/
