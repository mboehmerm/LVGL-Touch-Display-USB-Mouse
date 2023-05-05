# Custom Boards/Partitions for Arduino IDE.

 Edit :

-   C:\Users\<user>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.8\boards.txt or
-   C:\Users\<user>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.9\boards.txt

After every update boards.txt has to be edited again !

Partitions can be found here: 

- C:\Users\<user>\AppData\Local\Arduino15\packages\esp32\hardware\esp32\2.0.9\tools\partitions

Arduino IDE cached the board.txt. 

So remove the *.ldb files and the *.log file inside this folder :

- C:\Users\<user>\AppData\Roaming\arduino-ide\Local Storage\leveldb

--------------------------------------------------------------------------

solution found here : https://forum.arduino.cc/t/partition-scheme-ide-2-0-x/1065762


Edit:
Sorry, I read it now, that it is a caching problem (2.0.3 still has this problem):

    Thank you, that workaround solved my problem. 

The menu is cached. So you have to open:

    %APPDATA%\arduino-ide\Local Storage\leveldb

and remove the *.ldb files inside this folder. 
You will loose all settings for the boards but the boards.txt will be reloaded (and cached again).






