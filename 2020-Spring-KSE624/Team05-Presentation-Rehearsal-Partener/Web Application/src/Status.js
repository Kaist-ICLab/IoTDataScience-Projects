/**
 * @description Status component.
 * @version: July 2nd, 2020
 */
import React from 'react';

export default function Status({ status }) {
    if (status === 0) {
        return <span style={{ color: 'red' }}>Not Connected</span>
    } else if (status === 1) {
        return <span style={{ color: 'green' }}>Connected</span>
    } else if (status === 2) {
        return <span style={{ color: 'black' }}>Recording</span>
    } else if (status === 3) {
        return <span style={{ color: 'blue' }}>Presentation Ended</span>
    }
}