// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/someendpoint',   //The server endpoint we are connecting to
        success: function (data) {
            /*  Do something with returned object
                Note that what we get is an object, not a string, 
                so we do not need to parse it on the server.
                JavaScript really does handle JSONs seamlessly
            */
            //We write the object to the console to show that the request was successful
            console.log(data); 

        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log("cunt" + error); 
        }
    });

    //set up database

    updateFileLog();

    $('#calViewSelect').change(function(e){

        fileName = {data : $('#calViewSelect').val()}

        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            data: fileName,
            url: '/getEvents',   //The server endpoint we are connecting to
            success: function (data) {

                //console.log("length:"+JSON.parse(data.data).length);

                if(data.data == null){
                    $('#calTablePanel').html(fileName.data + "does not exist.<br>");
                    $("#eventSelect").html("");
                    return;
                }

                if(JSON.parse(data.data).length <= 0){
                    $('#calTablePanel').html(fileName.data + " has no events<br>");
                    $("#eventSelect").html("");
                    return;
                }

                var table = "<table id = 'eventTable'>";

                table += '<tr><th>Event Number</th><th>Start Data</th><th>Start Time</th><th>Summary</th><th>Props</th><th>Alarms</th></tr>'

                $("#eventSelect").html("")

                for(let i = 0; i<JSON.parse(data.data).length; i++){
                    let option = document.createElement("option");
                    option.text = (i + 1);
                    //option.value = data.fileNames[i];
                    $("#eventSelect").append(option);

                    table += '<tr><td>' + (i + 1) + '</td>'
                    
                    let event = JSON.parse(data.data)[i];

                    let eventData = event.startDT.date;
                    let eventTime = event.startDT.time;

                    if(eventData.length != 8 || eventTime.length != 6){
                        table += '<td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '<td>';
                        table += '</tr>';
                        continue;
                    }

                    let dataStr = eventData.substring(0,4) + "/" + eventData.substring(4,6) + "/" + eventData.substring(6,8);
                    let timeStr = eventTime.substring(0,2) + ":" + eventTime.substring(2,4) + ":" + eventTime.substring(4,6);
                    
                    if(event.startDT.isUTC){
                        timeStr += " (UTC)"
                    }

                    if(event != null){
                        table += '<td>' + dataStr + '</td><td>' + timeStr + '</td><td>' + event.summary + '</td><td>' + event.numProps + '</td><td>' + event.numAlarms + '</td>';
                    }else{
                        table += '<td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '</td><td>' + "" + '<td>';
                    }

                    table += '</tr>';
                }
                table += '</table>'

                $("#calTablePanel").html(table);

                $('#statTermPanel').append("File Log Populated<br>");

                },
                fail: function(error) {
                    // Non-200 return, do something with error
                    console.log(error); 
                }
        });

    });

    // Event listener form replacement example, building a Single-Page-App, no redirects if possible
    $('#btnTermClear').click(function(e){

        $('#statTermPanel').html("");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        //$.ajax({});
    });

    $('#btnShowProps').click(function(e){

        if($('#eventSelect')[0].length <= 0){
            $('#statTermPanel').append("Not a valid file.<br>");
            return;
        }

        let sendData = {
            fileName: $('#calViewSelect').val(),
            eventNum: $('#eventSelect').val()
        }


        $('#statTermPanel').append("Show Props Selected<br>");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            data: sendData,
            url: '/getOptProps',   //The server endpoint we are connecting to
            success: function (data) {

                if(data.data == null){
                    $('#statTermPanel').append("Properties not found.<br>");
                    return;
                }
                let propList = JSON.parse(data.data);
                
                for(let c = 0; c < propList.length; c++){
                    $('#statTermPanel').append("Property Name:" + propList[c].propName + ", " + "Property Description:" + propList[c].propDescr + "<br>");
                }

            },
            fail: function(error){
                console.log(error);

                $('#statTermPanel').append(sendData.fileName + " not found.<br>");
            }
        });
    });

    $('#btnShowAlarms').click(function(e){
        
        if($('#eventSelect')[0].length <= 0){
            $('#statTermPanel').append("Not a valid file.<br>");
            return;
        }

        let sendData = {
            fileName: $('#calViewSelect').val(),
            eventNum: $('#eventSelect').val()
        }


        $('#statTermPanel').append("Show Alarms Selected<br>");
        e.preventDefault();
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            data: sendData,
            url: '/getOptAlarms',   //The server endpoint we are connecting to
            success: function (data) {

                if(data.data == null){
                    $('#statTermPanel').append("Alarms not found.<br>");
                    return;
                }
                let alarmList = JSON.parse(data.data);

                
                for(let c = 0; c < alarmList.length; c++){
                    $('#statTermPanel').append("Action:" + alarmList[c].action + ", " + "Trigger:" + alarmList[c].trigger + "<br>");
                }

            },
            fail: function(error){
                console.log(error);

                $('#statTermPanel').append(sendData.fileName + " not found.<br>");
            }
        });
    });

    $('#addCalForm').submit(function(e){
        e.preventDefault();

        formData = {
            fileName: $('#addCalForm input[name = "fileName"]').val(),
            prodID: $('#addCalForm input[name = "prodID"]').val(),
            version: $('#addCalForm input[name = "version"]').val(),
            eventUID: $('#addCalForm input[name = "eventUID"]').val(),
            eventDTSTAMP: $('#addCalForm input[name = "eventDTSTAMP"]').val(),
            eventDTSTART: $('#addCalForm input[name = "eventDTSTART"]').val(),
            eventSummary: $('#addCalForm input[name = "eventSummary"]').val()
        }

        if(formData.fileName.split('.').pop() != "ics"){
            formData.fileName += ".ics";
        }

        //console.log("filesname:" + formData.fileName)

        if(!validateDTInput(formData.eventDTSTART)){
            $('#statTermPanel').append(formData.eventDTSTART + ": Invalid DTSTART.<br>");
            //console.log("dtstart:" + formData.eventDTSTART);
            return;
        }else if(!validateDTInput(formData.eventDTSTAMP)){
            console.log("dtstamp:" + formData.eventDTSTAMP);
            //$('#statTermPanel').append(formData.eventDTSTAMP + ": Invalid DTSTAMP.<br>");
            return;
        }else if(isNaN(formData.version)){
            console.log("version:" + formData.version);
            //$('#statTermPanel').append(formData.version + ": Invalid Version.<br>");
            return;
        }


        $('#statTermPanel').append("Create Calendar Selected<br>");
        
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            data: formData,
            url: '/createCal',   //The server endpoint we are connecting to
            success: function (data) {

                if(data.result == 0){
                    $('#statTermPanel').append("File Created.<br>");
                    updateFileLog();
                }else if(data.result == -2){
                    $('#statTermPanel').append(formData.fileName + " already exists.<br>");
                }else{
                    $('#statTermPanel').append(formData.fileName + " not created.<br>");
                }
                

            },
            fail: function(error){
                console.log(error);

                $('#statTermPanel').append("Files not found.<br>");
            }
        });
    });

    $('#addEventForm').submit(function(e){
        e.preventDefault();
        formData = {
            fileName: $('#addEventSelect').val(),
            eventUID: $('#addEventForm input[name = "UID"]').val(),
            eventDTSTAMP: $('#addEventForm input[name = "DTSTAMP"]').val(),
            eventDTSTART: $('#addEventForm input[name = "DTSTART"]').val(),
            eventSummary: $('#addEventForm input[name = "Summary"]').val()
        }

        let numbersOnly = /^[0-9]+$/;
        //console.log(numbersOnly.test(formData.eventDTSTART));

        if(!validateDTInput(formData.eventDTSTART)){
            $('#statTermPanel').append(formData.eventDTSTART + ": Invalid DTSTART.<br>");
            //console.log("dtstart:" + formData.eventDTSTART);
            return;
        }else if(!validateDTInput(formData.eventDTSTAMP)){
            //console.log("dtstamp:" + formData.eventDTSTAMP);
            $('#statTermPanel').append(formData.eventDTSTAMP + ": Invalid DTSTAMP.<br>");
            return;
        }else{
            //$('#statTermPanel').append("Invalid DTSTART.<br>");
        }


        $('#statTermPanel').append("Create Event Selected<br>");
        
        //Pass data to the Ajax call, so it gets passed to the 
        $.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            data: formData,
            url: '/addEvent',   //The server endpoint we are connecting to
            success: function (data) {

                //console.log("back");
                $('#statTermPanel').append(formData.fileName + " Updated.<br>");
                updateFileLog();

            },
            fail: function(error){
                console.log(error);

                $('#statTermPanel').append(formData.fileName + " not found.<br>");
            }
        });
    });



});

function validateDTInput(dt){
    let numbersOnly = /^[0-9]+$/;

    //console.log(dt);

    if(!numbersOnly.test(dt.substring(0,8))){
        //console.log(dt.substring(0,8));
        return false;
    }
    if(!numbersOnly.test(dt.substring(9,15))){
        //console.log(dt.substring(9,15));
        return false;
    }
    if(dt.substring(8,9) != "T" && dt.substring(8,9) != "t"){
        //console.log(dt.substring(8,9));
        return false;
    }
    if(dt.length != 16 && dt.length != 15){
        //console.log(dt.substring(15,16));
        return false;
    }
    if(dt.length == 16){
        if(dt.substring(15,16) != "z" && dt.substring(15,16) != "Z"){
            //console.log("dd" + dt.substring(15,16));
            return false;
        }
    }
    return true;
}

function updateFileLog(){
    $.ajax({
        type: 'get',            //Request type
        dataType: 'json',       //Data type - we will use JSON for almost everything 
        url: '/getAllCals',   //The server endpoint we are connecting to
        success: function (data) {

            //console.log("updatingfiles");

            if(data.fileNames.length <= 0){
                return;
            }

            var table = "<table id = 'fileTable'>";

            table += '<tr><th>File Name<br>(click to download)</th><th>Version</th><th>Product ID</th><th>Number of<br>Events</th><th>Number of<br>Properties</th></tr>'

            $("#calViewSelect").html("");
            $("#addEventSelect").html("");

            for(let i = 0; i<data.fileNames.length; i++){
                cal = JSON.parse(data.cals)[i];

                let option = document.createElement("option");
                option.text = data.fileNames[i];
                $("#calViewSelect").append(option);

                if(cal.version != null){
                    let option2 = document.createElement("option");
                    option2.text = data.fileNames[i];
                    $("#addEventSelect").append(option2);
                }
                

                table += '<tr><td>' + '<a href="/uploads/' + data.fileNames[i] +'">' + data.fileNames[i] + '</a>' + '</td>'
                
                


                if(cal.version != null){
                    table += '<td>' + cal.version + '</td><td>' + cal.prodID + '</td><td>' + cal.numEvents + '</td><td>' + cal.numProps + '</td>';
                }else{
                    table += '<td>' + "" + '</td><td>' + cal.prodID + '</td><td>' + "" + '</td><td>' + "" + '</td>'
                }

                table += '</tr>';
            }
            table += '</table>'
            $("#fileTablePanel").html(table);

            $('#statTermPanel').append("File Log Populated<br>");


        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error);

            $('#statTermPanel').append("Files not found.<br>");

        }
    });
}