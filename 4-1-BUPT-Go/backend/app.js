'use strict';
const path = require('path');

const express = require('express');
const mongoose = require('mongoose');

const favicon = require('serve-favicon');
const logger = require('morgan');
const cookieParser = require('cookie-parser');
const bodyParser = require('body-parser');

if (!process.env.NODE_ENV) {
    process.env.NODE_ENV = 'development';
}

const app = express();

app.use(favicon(__dirname + '/public/favicon.ico'));
app.use(logger('dev'));
app.use(cookieParser('BUPT-Go'));

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));

app.use(express.static(path.join(__dirname, 'public')));
app.use('/article', require('./routes/article-api'));
app.use('/user', require('./routes/user-api'));

// catch 404 and forward to error handler
app.use(function (req, res, next) {
    const err = new Error('Not Found');
    err.status = 404;
    next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
    app.use(function (err, req, res, next) {
        res.status(err.status || 500);
        res.send({
            err: err.message,
            stack: err.stack
        });
    });
}

// production error handler
// no stacktraces leaked to user
app.use(function (err, req, res, next) {
    res.status(err.status || 500);
    res.send({
        err: err.message
    });
});

app.set('port', process.env.PORT || 8080);

const mongoUrl = 'mongodb://localhost:27017/mydb';
mongoose.connect(mongoUrl).then(function () {
    console.log('Connected to ' + mongoUrl);
    const server = app.listen(app.get('port'), function () {
        console.log('Listening on port ' + server.address().port);
    });
}).catch(function (err) {
    console.error(err);
});
