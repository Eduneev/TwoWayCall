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

### Studio

1. Keep an eye on the 2WayCall page in 2WayLive portal to view all the connected classrooms.
2. When the teacher desires, he can call any of the connected classrooms.
3. To start 2WayCall on the Studio end, enter the Channel for the current session and press 'Join'.

## License

The MIT License (MIT).
