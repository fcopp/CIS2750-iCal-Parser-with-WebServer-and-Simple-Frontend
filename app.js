'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');
const mysql = require('mysql');

app.use(fileUpload());
//app.use(express.bodyParser());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  //console.log("test");
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});



//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 



var libcal = ffi.Library('./libcal',{
  'fileCaltoJSON': ['string',['string']],
  'getEventAlarms': ['string',['string','int']],
  'getEventList': ['string',['string']],
  'getEventProps':['string',['string','int']],
  'addEventtoCal':['int',['string','string','string','string','string']],
  'createCalfromUI':['int',['string','string','float','string','string','string','string']]
});

//Sample endpoint
app.get('/someendpoint', function(req , res){
  
    console.log("shit");
  
    const connection = mysql.createConnection({  
        host     : 'dursley.socs.uoguelph.ca',  
        user     : 'fcopp',  
        password : '0928913',  
        database : 'databaseNameGoesHere' 
    });
    connection.connect();
    connection.query("create table student (id int not null auto_increment,  last_name char(15),  first_name char(15), mark char(2), primary key(id) )", function (err, rows, fields) {
      if (err){
        console.log("Something went wrong. "+err);
      }else{
        console.log("got here");
      }
    });

    connection.end();



  res.send({
    foo: "shit"
  });
});

app.get('/getOptProps', function(req, res){

  let str = libcal.getEventProps(path.join(__dirname+'/uploads/'+req.query.fileName),parseInt(req.query.eventNum));

  res.send({
    data:str
  })

});

app.get('/getOptAlarms', function(req, res){

  let str = libcal.getEventAlarms(path.join(__dirname+'/uploads/'+req.query.fileName),parseInt(req.query.eventNum));

  res.send({
    data:str
  })

});

app.get('/addEvent', function(req, res){

  //console.log(req.query);
  let data = req.query;




  let status = libcal.addEventtoCal(data.eventUID, data.eventDTSTAMP, data.eventDTSTART, data.eventSummary, path.join(__dirname+'/uploads/'+req.query.fileName));
  //console.log(status);
  res.send({
    data:status
  });

});

app.get('/getEvents', function(req, res){

  //console.log(req.query.data);

  let str = libcal.getEventList(path.join(__dirname+'/uploads/'+req.query.data));

  res.send({
    data:str
  });
});

app.get('/createCal', function(req, res){

  //console.log("" + JSON.stringify(req.query));

  var files = fs.readdirSync(path.join(__dirname+'/uploads/'));

  if(files.indexOf(req.query.fileName) != -1){
    res.send({
      result:-2
    });

  }else{
    let data = req.query;
    let status = libcal.createCalfromUI(path.join(__dirname+'/uploads/'+data.fileName),data.prodID,parseFloat(data.version),data.eventUID,data.eventDTSTAMP,data.eventDTSTART,data.eventSummary);

    //console.log("status:" + status);

    res.send({
      result:status
    });
  }



  
});

app.get('/getAllCals', function(req , res){

  var fullCals = "[";
  var files = fs.readdirSync(path.join(__dirname+'/uploads/'));
  //console.log(files);



  for (var i = 0; i < files.length; i++) {

    if(files[i].split('.').pop() != "ics"){
      files.splice(i,1);
      i--;
      continue;
    }

    let str = libcal.fileCaltoJSON(path.join(__dirname+'/uploads/'+files[i]));

    if(!str){
      files.splice(i,1);
      i--;
      continue;
    }

    if(i == 0){
      fullCals += str;
    }else{
      fullCals += "," + str;
    }


  }

  fullCals += "]";

  //console.log("stuff:" + fullCals);

  res.send({
    fileNames:files,
    cals: fullCals
  });

});


app.listen(portNum);
console.log('Running app at localhost: ' + portNum);