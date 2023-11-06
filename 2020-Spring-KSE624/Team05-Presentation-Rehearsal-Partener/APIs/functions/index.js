/**
 * @description Index file for routing services.
 * @version: July 2nd, 2020
 */

const functions = require('firebase-functions');
const express = require('express')
const cors = require('cors')
const services = express()

// Create and Deploy Your First Cloud Functions
// https://firebase.google.com/docs/functions/write-firebase-functions

services.use(cors({ origin: true })) // allow cross-origin resource sharing
services.use('/', require('./services')) // set middleware services

exports.services = functions.https.onRequest(services) // export http functions of the services
