/**
 * @description Utility file for API calls and Report generation.
 * @version: July 2nd, 2020
 */

import axios from 'axios' // axios libraries for HTTP calls

// emotions with separation of bad and good set
const emotions = {
    bad: ['anger', 'contempt', 'disgust', 'fear', 'sadness', 'None',],
    good: ['happiness', 'neutral', 'surprise']
}

// gestures with separation of bad and good set
const gestures = {
    bad: ['clasp_hands', 'cross_arms', 'hide_one_arm', 'hide_two_arms', 'hold', 'point', 'rotate_head', 'stand_improperly', 'touch_body'],
    good: ['call_me', 'list', 'move', 'open_one_arm', 'open_two_arms', 'roll', 'show_level', 'show_small_thing', 'stand_properly']
}

/**
 * Compute and generate the report data.
 * @param {Object} report_data report data from the api call with given username.
 * @return {Object} computed report data of the given username.
 */
const generateReport = report_data => {
    let duration = report_data[report_data.length - 1].timestamp - report_data[0].timestamp // duration of the current user
    let time = { minute: Math.floor(duration / 60), second: duration % 60 } // extract time to minutes:seconds unit

    let [total, bad_eye, bad_emotion, bad_gesture] = [report_data.length, 0, 0, 0] // initialize count of each type (eye contact, facial expression, and gesture)
    let eyes_count = { '0': 0, '1': 0 }
    let emotions_count = { 'anger': 0, 'contempt': 0, 'disgust': 0, 'fear': 0, 'sadness': 0, 'None': 0, 'happiness': 0, 'neutral': 0, 'surprise': 0 }
    let gestures_count = {
        'clasp_hands': 0, 'cross_arms': 0, 'hide_one_arm': 0, 'hide_two_arms': 0, 'hold': 0, 'point': 0, 'rotate_head': 0, 'stand_improperly': 0, 'touch_body': 0,
        'call_me': 0, 'list': 0, 'move': 0, 'open_one_arm': 0, 'open_two_arms': 0, 'roll': 0, 'show_level': 0, 'show_small_thing': 0, 'stand_properly': 0
    }
    let gestures_image = {
        'clasp_hands': [], 'cross_arms': [], 'hide_one_arm': [], 'hide_two_arms': [], 'hold': [], 'point': [], 'rotate_head': [], 'stand_improperly': [], 'touch_body': [],
        'call_me': [], 'list': [], 'move': [], 'open_one_arm': [], 'open_two_arms': [], 'roll': [], 'show_level': [], 'show_small_thing': [], 'stand_properly': []
    }
    let emotions_image = { 'anger': [], 'contempt': [], 'disgust': [], 'fear': [], 'sadness': [], 'None': [], 'happiness': [], 'neutral': [], 'surprise': [] }
    let eyes_images = { '0': [], '1': [] }

    // loop entire report data records and count the number of each type
    for (const rd of report_data) {
        if (rd.eye_contact < 0.7) {
            bad_eye += 1
            eyes_count['0'] += 1
            eyes_images['0'].push(rd.image_url)
        } else {
            eyes_count['1'] += 1
            eyes_images['1'].push(rd.image_url)
        }

        if (emotions.bad.includes(rd.emotion)) { bad_emotion += 1 }
        if (gestures.bad.includes(rd.gesture)) { bad_gesture += 1 }

        emotions_count[rd.emotion] += 1
        gestures_count[rd.gesture] += 1

        // add images to the corresponding facial expression and gesture
        emotions_image[rd.emotion].push(rd.image_url)
        gestures_image[rd.gesture].push(rd.image_url)
    }

    // set data of chart for each behavior type
    const eye_chart = { 'eye_contact': total - bad_eye, 'no_eye_contact': bad_eye }
    const emotion_chart = { 'good': total - bad_emotion, 'bad': bad_emotion }
    const gesture_chart = { 'good': total - bad_gesture, 'bad': bad_gesture }

    // set data for eye contact table
    let eye_table = []
    for (const [key, value] of Object.entries(eyes_count)) {
        eye_table.push({
            key: key,
            name: key === '0' ? 'No Eye Contact' : 'Eye Contact',
            frequency: value,
            type: 'eye'
        })
    }

    // set data for facial expression table
    let emotion_table = []
    for (const [key, value] of Object.entries(emotions_count)) {
        emotion_table.push({
            key: key,
            name: key.charAt(0).toUpperCase() + key.slice(1),
            frequency: value,
            type: 'emotion'
        })
    }

    // set data for gesture table
    let gesture_table = []
    const gestures_name = {
        'clasp_hands': 'Hand Clasping', 'cross_arms': 'Crossing Arms', 'hide_one_arm': 'Hide One Hand', 'hide_two_arms': 'Hide Two Hands', 'hold': 'Hold Something', 'point': 'Pointing', 'rotate_head': 'Head Rotation', 'stand_improperly': 'Improper Standing', 'touch_body': 'Be Fidgety',
        'call_me': 'Self Mention', 'list': 'Item Listing', 'move': 'Walking', 'open_one_arm': 'Open One Arm', 'open_two_arms': 'Open Two Arms', 'roll': 'Rolling Hands', 'show_level': 'Indicate Level/Degree', 'show_small_thing': 'Show Small Thing', 'stand_properly': 'Proper Standing'
    }

    for (const [key, value] of Object.entries(gestures_count)) {
        gesture_table.push({
            key: key,
            name: gestures_name[key],
            frequency: value,
            type: 'gesture'
        })
    }

    // initialize begin statement of each behavior type.
    let summary = { eye: `Let's start with your eye contact. `, emotion: 'Moreover, ', gesture: 'Finally, about your gestures, ' }

    // set summary and suggestion statements of eye contact.
    const eye_ratio = 1 - (bad_eye / total)
    if (eye_ratio < 0.5) {
        summary.eye += `According to the results, you have not made enough eye contact with your audience. Our suggestion is that you need to make more eye contact with your audience because eye contact is one of the body languages that not only shows your confidence in the presentation but also gives more trust to your audience.`
    } else if (eye_ratio < 0.9) {
        summary.eye += `Our system thinks that you are great at making eye contact with your audience in sufficient proportion to other attention points.`
    } else {
        summary.eye += `You are great at making eye contact with your audience. However, making too much eye contact may cause your audience to feel stared and get a bit uncomfortable. We suggest that you should not always put your eyes on your audience; instead, you may sometimes look at your presentation slides or look around.`
    }

    // set summary and suggestion statements of facial expression.
    const emotion_ratio = 1 - (bad_emotion / total)
    if (emotion_ratio < 0.5) {
        summary.emotion += `your facial expressions looked uncomfortable, nervous, or in a bad mood, which can affect your audience's feelings and engagement. Here, we recommend you to practice or record your presentation in front of a mirror and relax tension from your face while delivering your presentation.`
    } else if (emotion_ratio < 0.9) {
        summary.emotion += `you have made good facial expressions during your presentation. However, your facial expressions sometimes looked a bit uncomfortable, nervous, or in a bad mood, which can also affect your audience's feelings and engagement. Here, we recommend you to practice or record your presentation in front of a mirror and relax tension from your face while delivering your presentation.`
    } else {
        summary.emotion += `you smoothly made an excellent facial expression during your presentation, which can effectively enhance your point and draw the audience's attention.`
    }

    // set summary and suggestion statements of gesture
    const gesture_ratio = 1 - (bad_gesture / total)
    let [bad_top, good_top] = [[], []]

    // find top-3 bad gestures and top-3 good gestures
    for (const [key, value] of Object.entries(gestures_count)) {
        if (gestures.bad.includes(key) && value > 0) { bad_top.push({ name: key, value: value }) }
    }

    for (const [key, value] of Object.entries(gestures_count)) {
        if (['roll', 'move'].includes(key) && (value / total) > 0.12) { good_top.push({ name: key, value: value }) }
    }

    // descendingly sort the frequency of each gesture
    bad_top.sort((a, b) => b.value - a.value)
    good_top.sort((a, b) => b.value - a.value)

    if (gesture_ratio < 0.5) {
        if (bad_top.length >= 3) {
            summary.gesture += `our system noticed that your overall gestures looked uncomfortable or nervous and sometimes lacked confidence during the presentation. Additionally, you mostly did ${gestures_name[bad_top[0].name].toLowerCase()}, ${gestures_name[bad_top[1].name].toLowerCase()}, and ${gestures_name[bad_top[2].name].toLowerCase()}. As our advice, you should not make those gestures for your next presentation.`
        } else if (bad_top.length === 2) {
            summary.gesture += `our system noticed that your overall gestures looked uncomfortable or nervous and sometimes lacked confidence during the presentation. Additionally, you mostly did ${gestures_name[bad_top[0].name].toLowerCase()} and ${gestures_name[bad_top[1].name].toLowerCase()}. As our advice, you should not make those gestures for your next presentation.`
        } else {
            summary.gesture += `our system noticed that your overall gestures looked uncomfortable or nervous and sometimes lacked confidence during the presentation. Additionally, you mostly did ${gestures_name[bad_top[0].name].toLowerCase()}. As our advice, you should not make those gestures for your next presentation.`
        }
    } else if (gesture_ratio < 0.9) {
        if (bad_top.length >= 3) {
            summary.gesture += `you were doing great in making the overall gestures! Unfortunately, there were a few bad gestures, such as ${gestures_name[bad_top[0].name].toLowerCase()}, ${gestures_name[bad_top[1].name].toLowerCase()}, and ${gestures_name[bad_top[2].name].toLowerCase()}. These gestures can make your audience perceive the discomfort, nervousness, or diffidence from you. To get a better positive result, please consider not making those gestures in the next time.`
        } else if (bad_top.length === 2) {
            summary.gesture += `you were doing great in making the overall gestures! Unfortunately, there were a few bad gestures, such as ${gestures_name[bad_top[0].name].toLowerCase()} and ${gestures_name[bad_top[1].name].toLowerCase()}. These gestures can make your audience perceive the discomfort, nervousness, or diffidence from you. To get a better positive result, please consider not making those gestures in the next time.`
        } else {
            summary.gesture += `you were doing great in making the overall gestures! Unfortunately, there were a few bad gestures (e.g., ${gestures_name[bad_top[0].name].toLowerCase()}). These gestures can make your audience perceive the discomfort, nervousness, or diffidence from you. To get a better positive result, please consider not making those gestures in the next time.`
        }
    } else {
        // additional summary and suggestion for excessive use of good gestures
        summary.gesture += `the system observed your excellent work on making proper gestures.`
        if (good_top.length > 0) {
            if (good_top.length === 1) {
                summary.gesture += `You were making the ${gestures_name[good_top[0].name].toLowerCase()} too much. Even though ${gestures_name[good_top[0].name].toLowerCase()} while presenting is good, excessively making of this action can show your nervousness and probably distract your audience from the presentation.`
            } else {
                summary.gesture += `You were making the ${gestures_name[good_top[0].name].toLowerCase()} and ${gestures_name[good_top[1].name].toLowerCase()} too much. Even though ${gestures_name[good_top[0].name].toLowerCase()} and ${gestures_name[good_top[1].name].toLowerCase()} while presenting are good things to do, excessively making of these actions can show your nervousness and probably distract your audience from the presentation.`
            }
        } else if (good_top.length === 0 && bad_top.length === 0) {
            summary.gesture += ` Please keep going in this direction!`
        }
    }

    return { time, eye_chart, emotion_chart, gesture_chart, eye_table, emotion_table, gesture_table, eyes_images, emotions_image, gestures_image, summary }
}


/**
 * Update user's current status.
 * @param {Object} data user's data (username, status, and privaycy options)
 * @return {Object} update result data
 */
const updateCurrentStatus = async data => {
    const result = await axios.post('https://us-central1-jet-presentation.cloudfunctions.net/services/updateStatus', data)
    if (result.status === 200) {
        return result.data
    } else {
        return null
    }
}

/**
 * Get user's raw report data.
 * @param {Object} data user's data (username, status, and privaycy options)
 * @return {Object} user's raw report data.
 */
const getReportData = async data => {
    const result = await axios.post('https://us-central1-jet-presentation.cloudfunctions.net/services/getReport', data)
    if (result.status === 200) {
        return result.data
    } else {
        return null
    }
}

export default { generateReport, updateCurrentStatus, getReportData }