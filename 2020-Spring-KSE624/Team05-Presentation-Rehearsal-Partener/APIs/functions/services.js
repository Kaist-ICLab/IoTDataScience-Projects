/**
 * @description Service file for REST API calls.
 * @version: July 2nd, 2020
 */

const express = require('express')
const admin = require('./firebase')
const router = express.Router()

/**
 * Update current status of the given username.
 * @method POST
 * @request username and current status 
 * @respond current status and result message
 */
router.post('/updateStatus', async (req, res) => {
    const { username, status } = req.body
    const writeResult = await admin.firestore().collection('presentation')
        .doc(username)
        .update({ username: username, status: status });

    res.status(200).json({ currentStatus: status, result: `Message with ID: ${writeResult.id} added.` })
})

/**
 * Get current status of the given username.
 * @method POST
 * @request username 
 * @respond current status
 */
router.post('/getStatus', async (req, res) => {
    const { username } = req.body
    const doc = await admin.firestore().collection('presentation').doc(username).get();

    if (doc.exists) {
        res.status(200).json({ data: doc.data() })
    } else {
        res.status(404).json({ data: null })
    }
})

/**
 * Update/Save raw report data from Jetson Nano with the given username.
 * @method POST
 * @request username, image_url, timestamp, eye_contact, emotion, and gesture.
 * @respond Update status message.
 */
router.post('/updateReportData', async (req, res) => {
    const { username, image_url, timestamp, eye_contact, emotion, gesture } = req.body
    const updateResult = await admin.firestore().collection('presentation').doc(username).update(
        { 'report_data': admin.firestore.FieldValue.arrayUnion({ timestamp, image_url, eye_contact, emotion, gesture }) }
    );

    res.status(200).json({ result: `Updated: ${updateResult.writeTime}.` })
})

/**
 * Get raw user's collected report data.
 * @method POST
 * @request username
 * @respond raw user's report data object.
 */
router.post('/getReport', async (req, res) => {
    const { username } = req.body
    const doc = await admin.firestore().collection('presentation').doc(username).get();
    
    if (doc.exists) {
        res.status(200).json({ data: doc.data() })
    } else {
        res.status(404).json({ data: null })
    }
})

module.exports = router