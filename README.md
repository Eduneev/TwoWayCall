# 2WayCall

Calling client for Eduneev 2WayLive!

## Running the App
First, define the APP_ID with your App ID.

```
#define APP_ID _T("Your App ID")
```
In order to run the client, you also need access to the Eduneev Server, which the app reaches out to.

## Documenation

### Center/Classroom

1. Retrieve the Channel for the Current Session from the View Sessions tab in the 2WayLive Portal 
2. Input the channel in 2WayCall application and press "Set Channel". 
3. The application should now state 'Connected'. This means that the server has successfully accepted the channel.
4. This is all that is needed, the teacher will connect in case he wants to.
5. When teacher makes a call, 2WayCall opens up in full screen mode by default. To reduce size, press the Esc key. This should open a diminished view where other changes regarding the call can be made, including muting audio, change input video source and others.

### Studio

1. Keep an eye on the 2WayCall page in 2WayLive portal to view all the connected classrooms.
2. When the teacher desires, he can call any of the connected classrooms.
3. To start 2WayCall on the Studio end, enter the Channel for the current session and press 'Join'.

### Error Messages

1. In case 2WayCall is unable to connect to the server for the Center/Classroom, it will open up a prompt stating 'Unable to connect to Server'. 
 In this case, please reach out to support. 
2. In case PC is not authorized to use 2WayCall, an 'Unauthorized access' error will be shown.


## License

The MIT License (MIT).
