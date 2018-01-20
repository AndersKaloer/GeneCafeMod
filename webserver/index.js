const express = require('express')
const fs = require('fs');
const mustacheExpress = require('mustache-express');
const bodyParser = require('body-parser');
const csvParser = require('csv-parse/lib/sync');


const app = express()
var router = express.Router();

// Register '.mustache' extension with The Mustache Express
app.engine('mustache', mustacheExpress());
app.set('view engine', 'mustache');
app.set('views', __dirname + '/views');

app.use(express.static('public'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));


app.get('/', function (req, res) {
    var roasts = load_roasts();
    res.render('index', {
        roasts: roasts
    });
})

app.get('/roast/:roastid', function (req, res) {
    var querystring = require('querystring');
    var roastid = req.params['roastid'];
    var roastid_urlescaped = querystring.escape(roastid);
    var roast_params = load_roast(roastid);
    res.render('roast', {
        roast: roast_params,
        roastid: roastid,
        roastid_urlescaped: roastid_urlescaped,
    });
})

app.get('/roast/data/:roastid.json', function (req, res) {
    var roastid = req.params['roastid'];
    var roast_data = load_roast_data(roastid);
    res.send(roast_data);
})

app.post('/roast/:roastid', function (req, res) {
    var roastid = req.params['roastid'];
    save_roast_metadata(roastid, req.body);
    res.sendStatus(200);
})

app.delete('/roast/:roastid', function (req, res) {
    var roastid = req.params['roastid'];
    delete_roast(roastid);
    res.sendStatus(200);
})

app.listen(3000, () => console.log('Roaster web server listening on port 3000!'))


function load_roasts() {
    var path = require('path');
    var querystring = require('querystring');
    var dateFormat = require('dateformat');

    var log_dir = path.join(__dirname, '..', 'roast_log');

    var roasts = [];
        
    var items = fs.readdirSync(log_dir);
    for(var i=0; i<items.length; i++) {
        try {
            roast = load_roast(items[i]);
            roast['id'] = items[i];
            roast['id_urlescaped'] = querystring.escape(roast['id']);
            roasts.push(roast);
        } catch(e) {
            // Skip
        }
    }

    // Sort by date
    roasts.sort(function(a, b) {
        return b['date']-a['date'];
    });

    // Add index
    for(var i=0; i < roasts.length; i++) {
        roasts[i]['index'] = i+1;
    }
    
    return roasts;
}

function load_roast(roastid) {
    var path = require('path');
    var log_dir = path.join(__dirname, '..', 'roast_log', roastid);
    if(!fs.existsSync(log_dir)) {
        throw 'Invalid roastid';
    }
    var meta_file = path.join(log_dir, 'roast.json');
    if(!fs.existsSync(meta_file)) {
        throw 'Meta file does not exist';
    }
    var content = fs.readFileSync(meta_file);
    var metadata = JSON.parse(content);
    metadata['date'] = new Date(metadata['date']*1000); // Convert to ms
    // format date
    var dateFormat = require('dateformat');
    metadata['date_fmt'] = dateFormat(metadata['date'], 'default');
    
    return metadata;
}

function load_roast_data(roastid) {
    var path = require('path');
    var log_dir = path.join(__dirname, '..', 'roast_log', roastid);
    if(!fs.existsSync(log_dir)) {
        throw 'Invalid roastid';
    }
    var log_file = path.join(log_dir, 'roast.csv');
    if(!fs.existsSync(log_file)) {
        throw 'Log file does not exist';
    }


    var content = fs.readFileSync(log_file);
    var parser = csvParser()
    var records = csvParser(content, {columns: true});

    var data = {
        'time': new Array(records.length),
        'data': {
            'heat': new Array(records.length),
            'temp': new Array(records.length),
            'crack': new Array(records.length),
        }
    };
    for(var i=0; i < records.length; i++) {
        data['time'][i] = new Date(records[i]['time']*1000);
        data['data']['heat'][i] = records[i]['heat'];
        data['data']['temp'][i] = records[i]['temp'];
        data['data']['crack'][i] = (records[i]['crack']==='1' ? records[i]['temp'] : NaN);
    }
    delete records['heat'];
    delete records['temp'];
    delete records['crack'];
    
    return data;
}

function save_roast_metadata(roastid, data) {
    var path = require('path');
    var log_dir = path.join(__dirname, '..', 'roast_log', roastid);
    if(!fs.existsSync(log_dir)) {
        throw 'Invalid roastid';
    }
    var meta_file = path.join(log_dir, 'roast.json');
    if(!fs.existsSync(meta_file)) {
        throw 'Meta file does not exist';
    }
    var content = fs.readFileSync(meta_file);
    var metadata = JSON.parse(content);
    // Update
    for(var key in data) {
        metadata[key] = data[key];
    }
    // Save
    fs.writeFileSync(meta_file, JSON.stringify(metadata));
}

function delete_roast(roastid) {
    var path = require('path');
    var log_dir = path.join(__dirname, '..', 'roast_log', roastid);
    if(!fs.existsSync(log_dir)) {
        throw 'Invalid roastid';
    }
    // Delete
    const rimraf = require('rimraf');
    rimraf.sync(log_dir);
}
