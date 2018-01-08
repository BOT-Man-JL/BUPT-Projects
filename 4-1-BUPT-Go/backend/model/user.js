'use strict';
const mongoose = require('mongoose');

const Schema = mongoose.Schema;
const ObjectId = Schema.ObjectId;

const schema = new Schema({
    name: {
        type: String,
        required: true,
        unique: true
    },
    pass: {
        type: String,
        required: true
    },
    avatar: {
        type: String,
        required: true
    }
});

exports.schema = schema;
exports.model = mongoose.model('User', schema);
