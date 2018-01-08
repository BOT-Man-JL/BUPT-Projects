'use strict';
const express = require('express');
const router = express.Router();

const User = require('../model/user').model;

const errInvalidUserNamePass = '无效的用户名或密码';
const errInvalidAvatar = '无效的头像';
const errBadLoginAttempt = '错误的用户名或密码';
const errTakenUserName = '用户名已被占用';
const errBadModelSave = '无法保存用户账户';
const errAlreadyLogin = '请先注销';
const errNoLogin = '请先登录';

const msgLogin = '登陆成功！';
const msgSignup = '注册成功！';
const msgLogout = '注销成功！';

router.post('/login', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (userName) {
        return res.status(400).send({ err: errAlreadyLogin });
    }

    const name = req.body.name;
    const pass = req.body.pass;
    if (!name || !pass) {
        return res.status(400).send({ err: errInvalidUserNamePass });
    }

    const query = { name, pass };
    User.find(query).then(function (docs) {
        if (docs.length != 1) {
            return res.status(400).send({ err: errBadLoginAttempt });
        }
        const avatar = docs[0].avatar;

        res.cookie('userName', name);
        res.cookie('userNameSigned', name, { signed: true });
        res.cookie('userAvatar', avatar);

        res.send({ msg: msgLogin });
        console.log('login', name, pass, avatar);

    }).catch(function (err) {
        res.status(500).send({ err });
        console.error(err);
    });
});

router.post('/signup', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (userName) {
        return res.status(400).send({ err: errAlreadyLogin });
    }

    const name = req.body.name;
    const pass = req.body.pass;
    if (!name || !pass) {
        return res.status(400).send({ err: errInvalidUserNamePass });
    }

    if (!req.file || req.file.mimetype.indexOf('image/') != 0) {
        return res.status(400).send({ err: errInvalidAvatar });
    }

    const avatar = '/upload/' + req.file.filename;
    new User({ name, pass, avatar }).save().then(function (doc) {
        if (!doc) {
            return res.status(400).send({ err: errBadModelSave });
        }

        res.cookie('userName', name);
        res.cookie('userNameSigned', name, { signed: true });
        res.cookie('userAvatar', avatar);

        res.send({ msg: msgSignup });
        console.log('signup', name, pass, avatar);

    }).catch(function (err) {
        if (err && err.code == 11000) {
            return res.status(400).send({ err: errTakenUserName });
        }

        res.status(500).send({ err });
        console.error(err);
    });
});

router.post('/logout', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (!userName) {
        return res.status(400).send({ err: errNoLogin });
    }

    res.clearCookie('userName');
    res.clearCookie('userNameSigned');
    res.clearCookie('userAvatar');

    res.send({ msg: msgLogout });
    console.log('logout', userName);
});

// dev helper

if (process.env.NODE_ENV == 'development') {
    router.post('/clear', function (req, res) {
        User.remove().then(function (ret) {
            res.send(ret);
            console.log('clear users');
        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
    });
}

module.exports = router;
