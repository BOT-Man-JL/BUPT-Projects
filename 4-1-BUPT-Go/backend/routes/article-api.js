'use strict';
const express = require('express');
const router = express.Router();

const ObjectId = require('mongoose').Types.ObjectId;
const Article = require('../model/article').model;
const User = require('../model/user').model;

const richTemplate = require('../model/article').richTemplate;
const thinTemplate = require('../model/article').thinTemplate;

const errBadModelSave = '无法保存该文章';
const errInvalidArticleId = '无效的文章ID';
const errInvalidArticleContent = '无效的文章标题/正文';
const errInvalidArticleMeta = '无效的类别/地区信息';
const errInvalidArticleImage = '无效的文章图片';
const errNoArticle = '查询不到此文章';
const errNoLogin = '未登录！';

const msgCreate = '创建成功！';
const msgUpdate = '更新成功！';
const msgDelete = '删除成功！';

// List Query

router.get('/recent', function (req, res) {
    Article.find()
        .select(richTemplate)
        .sort({ timestamp: -1 })
        .then(function (docs) {
            res.send(docs);

        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
});

router.get('/search', function (req, res) {
    const query = {};
    if (req.query.category)
        query.category = req.query.category;
    if (req.query.area)
        query.area = req.query.area;

    Article.find(query)
        .select(richTemplate)
        .sort({ timestamp: -1 })
        .then(function (docs) {
            res.send(docs);

        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
});

router.get('/user', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (!userName) {
        return res.status(401).send({ err: errNoLogin });
    }

    const query = { author: userName };
    Article.find(query)
        .select(thinTemplate)
        .sort({ timestamp: -1 })
        .then(function (docs) {
            res.send(docs);

        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
});

// Item Query/Post/Delete

router.get('/', function (req, res) {
    const id = req.query.id;
    if (!ObjectId.isValid(id)) {
        return res.status(400).send({ err: errInvalidArticleId });
    }

    Article.findById(id).then(function (doc) {
        if (!doc) {
            return res.status(400).send({ err: errNoArticle });
        }

        const query = { name: doc.author };
        User.find(query).then(function (docs) {
            if (docs.length == 1) {
                return res.send({
                    author: doc.author,
                    timestamp: doc.timestamp,
                    img: doc.img,
                    title: doc.title,
                    text: doc.text,
                    category: doc.category,
                    area: doc.area,
                    location: doc.location,
                    contact: doc.contact,
                    cost: doc.cost,
                    authorAvatar: docs[0].avatar
                });
            }
            res.send(doc);

        }).catch(function (err) {
            res.send(doc);

            // ignore this error
            console.error(err);
        });
    }).catch(function (err) {
        res.status(500).send({ err });
        console.error(err);
    });
});

router.post('/submit', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (!userName) {
        return res.status(401).send({ err: errNoLogin });
    }

    const id = req.body.id;
    if (id && !ObjectId.isValid(id)) {
        return res.status(400).send({ err: errInvalidArticleId });
    }

    if (req.file && req.file.mimetype.indexOf('image/') != 0) {
        return res.status(400).send({ err: errInvalidArticleImage });
    }

    const article = {
        author: userName,
        timestamp: new Date(),
        img: req.file ? req.file.filename : null,
        title: req.body.title,
        text: req.body.text,
        category: req.body.category,
        area: req.body.area,
        location: req.body.location || '',
        contact: req.body.contact || '',
        cost: req.body.cost || ''
    };

    if (!article.title || !article.text) {
        return res.status(400).send({ err: errInvalidArticleContent });
    }

    if (!article.category || !article.area) {
        return res.status(400).send({ err: errInvalidArticleMeta });
    }

    if (id) {
        if (!article.img) {
            delete article.img;
        }
        else {
            article.img = '/upload/' + article.img;
        }

        const query = { _id: id, author: userName };
        const setter = { $set: article };
        Article.findOneAndUpdate(query, setter).then(function (doc) {
            if (!doc) {
                return res.status(400).send({ err: errNoArticle });
            }

            res.send({ msg: msgUpdate });
            console.log('update', id, 'to', article);

        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
    }
    else {
        if (!article.img) {
            return res.status(400).send({ err: errInvalidArticleImage });
        }
        article.img = '/upload/' + article.img;

        new Article(article).save().then(function (doc) {
            if (!doc) {
                return res.status(400).send({ err: errBadModelSave });
            }

            res.send({ msg: msgCreate });
            console.log('create', article);

        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
    }
});

router.post('/delete', function (req, res) {
    const userName = req.signedCookies['userNameSigned'];
    if (!userName) {
        return res.status(401).send({ err: errNoLogin });
    }

    const id = req.body.id;
    if (!ObjectId.isValid(id)) {
        return res.status(400).send({ err: errInvalidArticleId });
    }

    const query = { _id: id, author: userName };
    Article.findOneAndRemove(query).then(function (doc) {
        if (!doc) {
            return res.status(400).send({ err: errNoArticle });
        }

        res.send({ msg: msgDelete });
        console.log('delete', id);

    }).catch(function (err) {
        res.status(500).send({ err });
        console.error(err);
    });
});

// dev helper

if (process.env.NODE_ENV == 'development') {
    router.post('/clear', function (req, res) {
        Article.remove().then(function (ret) {
            res.send(ret);
            console.log('clear articles');
        }).catch(function (err) {
            res.status(500).send({ err });
            console.error(err);
        });
    });
}

module.exports = router;
