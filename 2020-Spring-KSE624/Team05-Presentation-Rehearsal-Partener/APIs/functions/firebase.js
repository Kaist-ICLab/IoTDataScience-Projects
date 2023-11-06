/**
 * @description Firebase set up and service module file.
 * @version: July 2nd, 2020
 */

const functions = require('firebase-functions');
const admin = require('firebase-admin');

admin.initializeApp(functions.config().firebase);

module.exports = admin