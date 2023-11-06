/**
 * @description Button component.
 * @version: July 2nd, 2020
 */

//  Import related libraries and modules
import React from 'react';
import { Button } from 'antd';

/**
 * Render connection button.
 * @param {Object} props (status: current status, onChangeStatus: update function
 * onSaveReport: save report function, isLoading: loading status)
 * @return {Button} Button or button group that is conditionally rendered.
 */
export default function ConnectButton({ status, onChangeStatus, onSaveReport, isLoading }) {
    if (status === 0) {
        return <Button loading={isLoading} type="primary" block onClick={onChangeStatus}>Connect to Jetbot</Button>
    } else if (status === 1) {
        return <Button loading={isLoading} type="primary" danger block onClick={onChangeStatus}>Start Recording</Button>
    } else if (status === 2) {
        return <Button loading={isLoading} block onClick={onChangeStatus}>End Recording</Button>
    } else if (status === 3) {
        return (
            <>
                <Button loading={isLoading} danger block onClick={onChangeStatus}>Disconnect</Button>
                <p></p>
                <Button loading={isLoading} type="primary" block onClick={onSaveReport}>Save Report</Button>
            </>
        )
    }
}