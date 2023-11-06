/**
 * @description Main application file for page rendering.
 * @version: July 2nd, 2020
 */

//  Import related libraries and modules
import React from 'react'
import { Layout, Input, Divider, Checkbox, Modal } from 'antd'
import html2canvas from 'html2canvas'

import logo from './logo.png'
import utils from './utils'
import Status from './Status'
import ConnectButton from './ConnectButton'
import Report from './Report'

// Import custom CSS file
import './App.css'

const { Header, Content, Footer } = Layout

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

// privacy options that will be use later for deleting user's data (if needed)
const privacyOptions = [
    { label: 'Allow Recording', value: 1 },
    { label: 'Allow Saving Image on Cloud', value: 2 },
    { label: 'Allow Saving Evaluation on Cloud', value: 3 },
]

/** App class that renders main application. */
class App extends React.Component {

    /** @private @const {Object} state*/
    state = {
        isLoading: false, // loading status for buttons
        username: 'jetbot', // username for database reference
        privacy: [1], // privacy options' values
        status: 0, // user's current status
        isShowResult: false, // report showing status 
        report: null // user's report data
    }

    /**
     * Update checked privacy options.
     * @param {Array<number>} checkedValues checked values from privacy options form.
     * @return {null}.
     */
    onPrivacyChange = checkedValues => {
        this.setState({ privacy: checkedValues })
    }

    /**
     * Render report by calling the API with current username and get the computed
     * report data from utility function.
     * @return {null}
     */
    onRenderReport = async () => {
        const { data } = await utils.getReportData({ username: this.state.username }) // call get report data api for current username
        const report = utils.generateReport(data.report_data) // call generate report utility function

        this.setState({ isShowResult: true, report }) // show report result and update computed report data
    }


    /**
     * Open example image modal when user click on 'View' buttons.
     * @param {Object} record data of each instance record that contains
     * type, key, name, and frequence.
     * @return {null}.
     */
    onShowImage = record => {
        const { type, key, name } = record // get data from each record
        const { eyes_images, emotions_image, gestures_image } = this.state.report // get all images from user's report data
        if (type === 'eye') {
            let url = eyes_images[key][Math.floor(Math.random() * eyes_images[key].length)] // random one image from this group
            if (key === '0') {
                Modal.error({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with error icon (bad behavior)
            } else {
                Modal.success({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with success icon (good behavior)
            }
        } else if (type === 'emotion') {
            let url = emotions_image[key][Math.floor(Math.random() * emotions_image[key].length)] // random one image from this group
            if (emotions.bad.includes(key)) {
                Modal.error({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with error icon (bad behavior)
            } else {
                Modal.success({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with success icon (good behavior)
            }
        } else {
            let url = gestures_image[key][Math.floor(Math.random() * gestures_image[key].length)] // random one image from this group
            if (gestures.bad.includes(key)) {
                Modal.error({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with error icon (bad behavior)
            } else {
                Modal.success({ title: `Example Image (${name})`, content: <img width='100%' src={url} />, }) // show image with success icon (good behavior)
            }
        }
    }

    /**
     * Convert entire HTML page to canvas and save it as a png file.
     * @return {null}
     */
    onSaveReport = async () => {
        const canvas = await html2canvas(document.body) // select a DOM for canvas conversion
        const uri = canvas.toDataURL() // change canvas to uri data
        const filename = `${this.state.username}-report.png` // set file name

        let link = document.createElement('a') // create tag <a> element for reference
        if (typeof link.download === 'string') {
            link.href = uri; // set uri data
            link.download = filename; // set file name
            document.body.appendChild(link); // Firefox requires the link to be in the body
            link.click(); // simulate click action
            document.body.removeChild(link); // remove the link when done
        } else {
            window.open(uri); // simply open uri data for download
        }
    }

    /**
     * Change user's current status when user clicks the main button.
     * @return {null}
     */
    onChangeStatus = async () => {
        let { status } = this.state
        this.setState({ isLoading: true }) // set loading status
        /** status: 0 is not connected
         * status: 1 is connected
         * status: 2 is recording
         * status: 3 is present
         */
        if (status === 0) {
            status = 1
        } else if (status === 1) {
            status = 2
        } else if (status === 2) {
            status = 3
        } else {
            status = 0
        }

        // update user's current status to the database
        let data = await utils.updateCurrentStatus({ username: this.state.username, status })

        if (data.currentStatus === 3) { this.onRenderReport() } // call render report function

        // set loading status, current status, and report visibility
        this.setState({ isLoading: false, status: data.currentStatus, isShowResult: status === 3 })
    }

    /**
     * Render the page.
     * @return {HTML} the entire main application HTML DOM.
     */
    render() {

        return (
            <Layout className="layout" >
                <Header style={{ backgroundColor: 'black', boxShadow: '0 1px 7px' }}>
                    <div>
                        <img width='36' src={logo} alt="jet-presentation logo" />
                        <strong style={{ textAlign: 'center', marginLeft: 16, color: 'white', fontSize: '1.5rem' }}>Jet Presentation</strong>
                    </div>
                </Header>
                <Content style={{ padding: '0 24px' }}>
                    <br />
                    <div className="site-layout-content">
                        {/* Status Section */}
                        <p><strong>Status 🤖: </strong><Status status={this.state.status} /></p>
                        {this.state.isShowResult ?
                            // Report Section
                            <> {this.state.report ? <Report {...this.state.report} onShowImage={this.onShowImage} /> : null} </>
                            :
                            <>
                                {/* Input Form Section */}
                                <Input placeholder="Please enter your name here." value={this.state.username} disabled />
                                <Divider orientation='left'>Privacy Options</Divider>
                                <Checkbox.Group options={privacyOptions} defaultValue={this.state.privacy} onChange={this.onPrivacyChange} />
                                <br />
                                <br />
                            </>
                        }
                        <ConnectButton status={this.state.status} onChangeStatus={this.onChangeStatus} onSaveReport={this.onSaveReport} isLoading={this.state.isLoading} />
                    </div>
                </Content>
                <Footer style={{ textAlign: 'center', fontSize: 10 }}>Jet Presentation © 2020 Created by Ant Design</Footer>
            </Layout>
        );
    }
}

export default App;
