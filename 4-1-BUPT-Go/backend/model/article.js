'use strict';
const mongoose = require('mongoose');
const options = require('../../common/article-common.json');

const Schema = mongoose.Schema;
const ObjectId = Schema.ObjectId;

const schema = new Schema({
    author: {
        type: String,
        required: true
    },
    timestamp: {
        type: Date,
        required: true
    },
    img: {
        type: String,
        required: true
    },
    title: {
        type: String,
        trim: true,
        required: true
    },
    text: {
        type: String,
        required: true
    },
    category: {
        type: String,
        required: true,
        enum: options.categoryOptions
    },
    area: {
        type: String,
        required: true,
        enum: options.areaOptions
    },
    location: String,
    contact: String,
    cost: String
});

exports.schema = schema;
exports.model = mongoose.model('Article', schema);

exports.richTemplate = {
    _id: 1, author: 1, timestamp: 1,
    title: 1, img: 1,
    category: 1, area: 1
};
exports.thinTemplate = {
    _id: 1, title: 1, timestamp: 1
};
