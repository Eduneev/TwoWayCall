'use strict';
exports.__esModule = true;
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
var Studio = /** @class */ (function () {
    function Studio(StudioID, StudioName, wsID, conn) {
        this.StudioID = StudioID;
        this.StudioName = StudioName;
        this.wsID = wsID;
        this.conn = conn;
    }
    return Studio;
}());
var Classroom = /** @class */ (function () {
    function Classroom(ClassroomID, ClassroomName, wsID, conn) {
        this.ClassroomID = ClassroomID;
        this.ClassroomName = ClassroomName;
        this.wsID = wsID;
        this.conn = conn;
    }
    return Classroom;
}());
var Connection = /** @class */ (function () {
    function Connection(channel, wsID) {
        this.channel = channel;
        this.wsID = wsID;
        this.classroomWsID = [];
    }
    return Connection;
}());
var Events;
(function (Events) {
    Events["CONNECTION"] = "Connection";
    Events["MESSAGE"] = "Message";
    Events["DISCONNECTION"] = "Disconnect";
})(Events || (Events = {}));
var Profile;
(function (Profile) {
    Profile["TWOWAYCALL"] = "2WayCall";
})(Profile || (Profile = {}));
var Action;
(function (Action) {
    Action["JOIN"] = "join";
    Action["LEAVE"] = "leave";
})(Action || (Action = {}));
var Message;
(function (Message) {
    Message["PROFILE"] = "profile";
    Message["TYPE"] = "type";
    Message["WSID"] = "wsID";
    Message["CHANNEL"] = "channel";
    Message["STUDIONAME"] = "StudioName";
    Message["STUDIOID"] = "StudioID";
    Message["CLASSROOMNAME"] = "ClassroomName";
    Message["CLASSROOMID"] = "ClassroomID";
    Message["ACTION"] = "action";
})(Message || (Message = {}));
var WebSocket = require("ws");
var connections = [];
var websockets = {}; // websocket mapping
var studios = {};
var classrooms = {};
var port = 2000;
var idCounter = 0;
var wss = new WebSocket.Server({ port: port });
wss.on('connection', function connection(ws) {
    ws.on('close', function () {
        CloseConnection(ws);
    });
    ws.on('message', function (message) {
        try {
            var json = JSON.parse(message);
            HandleMessage(ws, json);
        }
        catch (e) {
            console.error("Exception: " + e);
        }
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
function CloseConnection(ws) {
    // If studio disconnection, remove connection, as well as corresponding classrooms
    if (ws.id in studios) {
        console.log("Removing Studio Connection");
        var studio = studios[ws.id];
        var conn = studio.conn;
        // Delete corresponding websocket and classroom
        for (var _i = 0, _a = conn.classroomWsID; _i < _a.length; _i++) {
            var wsID = _a[_i];
            delete websockets[wsID];
            delete classrooms[wsID];
        }
        var index = -1;
        for (var i = 0; i < connections.length; i++) {
            if (connections[i] === conn) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            console.error("Connection not found in connections array");
        }
        else {
            connections.splice(index, 1);
        }
        // Delete studio
        delete studios[ws.id];
    }
    else if (ws.id in classrooms) {
        console.log("Removing Classroom connection");
        var classroom = classrooms[ws.id];
        var conn = classroom.conn;
        var index = -1;
        for (var i_1 = 0; i_1 < conn.classroomWsID.length; i_1++) {
            if (ws.id === conn.classroomWsID[i_1]) {
                index = i_1;
                break;
            }
        }
        if (index === -1) {
            console.error("Classroom not in Connection Classroom list");
        }
        else {
            conn.classroomWsID.splice(index, 1);
        }
        delete classrooms[ws.id];
        delete websockets[ws.id];
    }
}
function HandleMessage(ws, json) {
    assert(Message.PROFILE in json, "Profile non existent in message");
    if (json[Message.PROFILE] === Profile.TWOWAYCALL) {
        if (json[Message.TYPE] == Events.CONNECTION) {
            ws.id = idCounter++;
            if (Message.STUDIONAME in json) {
                AddStudio(ws, json);
            }
            else if (Message.CLASSROOMNAME in json) {
                AddClassroom(ws, json);
            }
        }
        else if (json[Message.TYPE] == Events.MESSAGE) {
            assert(ws.id in studios, "Illegal Entity sending message. Do not send!");
            SendMessage(ws.id, json);
        }
    }
}
function AddStudio(ws, json) {
    console.log("Adding new studio and connection " + ws.id);
    var conn = new Connection(json[Message.CHANNEL], ws.id);
    var numConnections = connections.push(conn); // Add to connections array
    console.log("Number of Connections: " + numConnections);
    var studio = new Studio(json[Message.STUDIOID], json[Message.STUDIONAME], ws.id, conn);
    studios[ws.id] = studio; // Add to studios dict
    websockets[ws.id] = ws; // Add to websockets dict
}
function AddClassroom(ws, json) {
    var centerWsID = ws.id;
    console.log("Adding new classroom " + centerWsID);
    try {
        var channel = json[Message.CHANNEL];
        var connection = undefined;
        for (var _i = 0, connections_1 = connections; _i < connections_1.length; _i++) {
            var conn = connections_1[_i];
            if (conn.channel == channel) {
                connection = conn;
                conn.classroomWsID.push(centerWsID); // Add to channel's classroom list
            }
        }
        assert(connection !== undefined, "Could not add Classroom. Connection channel non existent!");
        var classroom = new Classroom(json[Message.CLASSROOMID], json[Message.CLASSROOMNAME], ws.id, connection);
        classrooms[centerWsID] = classroom; // Add to classrooms dict
        websockets[centerWsID] = ws; // Add to websockets dict
        // Send Message to Studio to Add Classroom
        var studiows = websockets[connection.wsID];
        studiows.send(JSON.stringify({
            profile: Profile.TWOWAYCALL,
            type: Events.CONNECTION,
            ClassroomName: json[Message.CLASSROOMNAME],
            wsID: centerWsID
        }));
    }
    catch (e) {
        console.error(e);
    }
}
function SendMessage(studioWsID, json) {
    var studio = studios[studioWsID];
    var conn = studio.conn;
    if (json[Message.WSID] in conn.classroomWsID) {
        var centerWs = websockets[json[Message.WSID]];
        centerWs.send(JSON.stringify({
            profile: Profile.TWOWAYCALL,
            action: json[Message.ACTION],
            channel: conn.channel
        }));
    }
    else {
        console.error("Classroom not in connection channel");
    }
}