'use strict';
// 2WayCall what is required.
/*

    Studio:
        Initiate ws-session with the following connection
        {
            profile: Profile.2WAYCALL,
            type: Event.CONNECTION,
            StudioName: <studioName>,
            StudioId: <StudioId>,
            channel: <channel name>
        }

        Receive Message from Classrooms wanting to join
        {
            profile: Profile.2WAYCALL,
            type: EVENT.CONNECTION,
            ClassroomName: <ClassroomName>,
            wsID: <websocket id>
        }

        {
            profile: twowaycall,
            type: connection,
            ClassroomName: Piteampura 1,
            wsID: 123
        }


        Upon receiving message:
            1) Add div to add classroom connection. Create attributes wsid, 
            2) Make an icon for call and make an onclick button that sends a join/leave message depending on the state

        {
            profile: Profile.2WAYCALL,
            type: Event.MESSAGE,
            action: Action.JOIN/LEAVE,
            wsID: <websocket id>,
            channel: <channel name>
        }

        Close Connection:

            {
              profile: Profile.TWOWAYCALL,
              type: Event.CLOSE,
              channel: <channel name>
            }

            Upon closing, we wish to delete channel and close all corresponding classroom connections
        
        {
            profile: Profile.2WAYCALL,
            type: Event.DISCONNECTION,
            wsID: [list of all wsIDs],
            channel: <channel name>
        }

            Do the following at server:
            1) Close classroom websocket connections and delete from ws dictionary
            2) Delete connection object from Connections list
            3) Close Studio Connection and delete from ws dictionary

*/

class Studio {
    StudioID: number;
    StudioName: string;
    wsID: number; // websocket id
    conn: Connection;

    constructor (StudioID:number, StudioName:string, wsID:number, conn:Connection) {
        this.StudioID = StudioID;
        this.StudioName = StudioName;
        this.wsID = wsID;
        this.conn = conn;
    }

}

class Classroom {
    ClassroomID: number;    
    ClassroomName: string;
    CenterName: string;
    wsID: number; // websocket id
    conn: Connection;

    constructor (ClassroomID:number, ClassroomName:string, CenterName:string, wsID:number, conn:Connection) {
        this.ClassroomID = ClassroomID;
        this.ClassroomName = ClassroomName;
        this.wsID = wsID;
        this.conn = conn;
        this.CenterName = CenterName;
    }
}

class Connection {
    channel: string;
    wsID: number; // Websocket id of Studio attached to this channel
    classroomWsID: number[];

    constructor (channel:string, wsID:number) {
        this.channel = channel;
        this.wsID = wsID;
        this.classroomWsID = [];
    }
}

enum Events {
    CONNECTION = "Connection",
    MESSAGE = "Message",
    DISCONNECTION = "Disconnect",
    CLOSE = "Close"
}

enum Profile {
    TWOWAYCALL = "2WayCall",
}

enum Action {
    JOIN = "join",
    LEAVE = "leave"
}

enum Message {
    PROFILE = 'profile',
    TYPE = 'type',
    WSID = 'wsID',
    CHANNEL = 'channel',
    STUDIONAME = 'StudioName',
    STUDIOID = 'StudioID',
    CLASSROOMNAME = 'ClassroomName',
    CLASSROOMID = 'ClassroomID',
    ACTION = 'action',
    CENTERNAME = 'CenterName'
}

import * as fs from 'fs';
import * as util from 'util';
var log_file = fs.createWriteStream(__dirname + '/debug.log', {flags : 'w'});
var log_stdout = process.stdout;

console.log = function(d) { //
    d = new Date().toString() + " | " +  d;
  log_file.write(util.format(d) + '\n');
  log_stdout.write(util.format(d) + '\n');
};

import * as WebSocket from 'ws';
let connections: {[channel: string]:Connection} = {};
let websockets: {[wsID:number]: WebSocket;} = {}; // websocket mapping
let studios: {[wsID:number]: Studio} = {};
let classrooms: {[wsID:number]: Classroom} = {};
var port: number = 2000;
var idCounter:number = 0;

const wss = new WebSocket.Server({ port: port});

wss.on('connection', function connection(ws: WebSocket) {
    ws.on('close', function(){
        try {
            HandleDisconnection(ws);
        }
        catch (e) {
            console.error(e);
        }
    });

    ws.on('message', function(message) {
        try {
            var json:JSON = JSON.parse(message);
            HandleMessage(ws, json);
        } catch (e) {
            console.error("Exception: " + e);
        }
    });

    ws.on('pong', function() {
        this.isAlive=true
    });
});

function assert(condition, message) {
    if (!condition) {
        message = message || "Assertion failed";
        if (typeof Error !== "undefined") {
            throw new Error(message);
        }
        throw message; // Fallback
    }
}

function HandleDisconnection(ws:WebSocket) {
    // If studio disconnection, remove connection, as well as corresponding classrooms
    if (ws.id in studios) {
        console.log("Removing Studio Connection");
        var studioWsID:number = ws.id
        var studio:Studio = studios[studioWsID];

        delete websockets[studioWsID];
        delete studios[studioWsID];
    } 
    else if (ws.id in classrooms) {
        console.log("Removing Classroom connection. Number of Connections is " + (Object.keys(classrooms).length - 1));
        var classroomWsID:number = ws.id;
        var classroom:Classroom = classrooms[classroomWsID];
        var conn:Connection = classroom.conn;

        var index:number = -1;
        for (let i:number=0; i < conn.classroomWsID.length; i++) {
            if (classroomWsID === conn.classroomWsID[i]) {
                index = i;
                break;
            }   
        }

        if (index === -1) {
            console.error("Classroom not in Connection Classroom list");
        } else {
            conn.classroomWsID.splice(index, 1);

            // Send studio notification of Classroom disconnection
            var studioWs:WebSocket = websockets[conn.wsID];
            studioWs.send (
                JSON.stringify({
                    profile: Profile.TWOWAYCALL,
                    type: Events.DISCONNECTION,
                    wsID: classroomWsID,
                    channel: conn.channel
                }));
        }

        
        delete classrooms[classroomWsID];
        delete websockets[classroomWsID];
    }
}

function CloseConnection(wsID:number, json:JSON) {
    var channel:string = json[Message.CHANNEL];

    console.log("Closing Channel: " + channel);
    assert (channel in connections, "Channel not present in Connections");

    var conn:Connection = connections[channel];
    
    for (var wsID of conn.classroomWsID) {
        websockets[wsID].close();
        delete websockets[wsID];
        delete classrooms[wsID];
    }

    delete studios[conn.wsID];
    delete websockets[conn.wsID];

    delete connections[channel];
}

function HandleMessage(ws:WebSocket, json:JSON) {
    assert(Message.PROFILE in json, "Profile non existent in message");
    if (json[Message.PROFILE] === Profile.TWOWAYCALL) {
        if (json[Message.TYPE] == Events.CONNECTION) {
            ws.id = idCounter++;
            if (Message.STUDIONAME in json) {
                AddStudio(ws, json);
            }
            else if (Message.CLASSROOMNAME in json) {
                AddClassroom(ws,json);                
            }
        }
        else if (json[Message.TYPE] == Events.MESSAGE) {
            assert(ws.id in studios, "Illegal Entity sending message. Do not send!");
            console.log("Message: " + JSON.stringify(json));
            SendMessage(ws.id, json);
        }
        else if (json[Message.TYPE] == Events.CLOSE)
            CloseConnection(ws.id, json)
    }
}

function AddStudio (ws: WebSocket, json:JSON) {
    var studioWsID = ws.id;
    var channel:string = json[Message.CHANNEL];
    console.log ("Adding new studio and connection " + studioWsID);

    if (channel in connections) {
        console.log ("Channel connection already exists");
        var conn = connections[channel];
        if (conn.wsID in studios) {
            throw "Studio connection already exists. Rejecting Studio Connection!";
        }
        conn.wsID = studioWsID;
    }
    else {
        var conn:Connection = new Connection (json[Message.CHANNEL], studioWsID);
        connections[channel] = conn;
        console.log ("Number of Connections: " + Object.keys(connections).length);
    }

    var studio:Studio = new Studio (json[Message.STUDIOID], json[Message.STUDIONAME], studioWsID, conn);
    studios[studioWsID] = studio; // Add to studios dict

    websockets[studioWsID] = ws; // Add to websockets dict

    // Send notice about all connected classrooms to studio
    for (let wsID of conn.classroomWsID){
        ws.send(JSON.stringify({
            profile: Profile.TWOWAYCALL,
            type: Events.CONNECTION,
            ClassroomName: classrooms[wsID].ClassroomName,
            CenterName: classrooms[wsID].CenterName,
            wsID: wsID
        }));
    }
}

function AddClassroom (ws: WebSocket, json:JSON) {
    var classroomWsID = ws.id;
    console.log ("Adding new classroom " + classroomWsID + ". Number of classrooms " + (Object.keys(classrooms).length + 1));
    var channel = json[Message.CHANNEL];

    assert(channel in connections, "Could not add Classroom. Connection channel non existent!");

    var conn:Connection = connections[channel];
    conn.classroomWsID.push(classroomWsID);
    console.log("Added classroom to connection list.");

    var classroom:Classroom = new Classroom(json[Message.CLASSROOMID], json[Message.CLASSROOMNAME], json[Message.CENTERNAME], classroomWsID, conn );
    classrooms[classroomWsID] = classroom // Add to classrooms dict

    websockets[classroomWsID] = ws // Add to websockets dict

    // Send Message to Studio to Add Classroom
    var studiows:WebSocket = websockets[conn.wsID];
    console.log("Sending classroom connection to Studio " + studios[conn.wsID].StudioName);
    studiows.send(
        JSON.stringify ({
            profile: Profile.TWOWAYCALL,
            type: Events.CONNECTION,
            ClassroomName: json[Message.CLASSROOMNAME],
            CenterName:json[Message.CENTERNAME],
            wsID: classroomWsID
        }));
}

function SendMessage (studioWsID: number, json:JSON) {
    var studio:Studio = studios[studioWsID];
    var conn:Connection = studio.conn;
    
    assert(containsObject(Number(json[Message.WSID]), conn.classroomWsID), "Classroom not in connection channel");

    var centerWs = websockets[json[Message.WSID]];
    centerWs.send(
        JSON.stringify({
            profile: Profile.TWOWAYCALL,
            action: json[Message.ACTION],
            channel: conn.channel
        }));
}

function containsObject(obj, list) {
    var i;
    for (i = 0; i < list.length; i++) {
        if (list[i] === obj) {
            return true;
        }
    }

    return false;
}

function noop() {}

const interval = setInterval(function ping() {
    wss.clients.forEach(function each(ws) {
      if (ws.isAlive === false) return ws.terminate();
  
      ws.isAlive = false;
      ws.ping(noop);
    });
  }, 120000);