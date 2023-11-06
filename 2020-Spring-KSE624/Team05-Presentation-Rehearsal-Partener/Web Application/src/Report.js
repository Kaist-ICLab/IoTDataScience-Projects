/**
 * @description Report component for computing and rendering user's report.
 * @version: July 2nd, 2020
 */

//  Import related libraries and modules
import React from 'react';
import { Divider, Table, Button } from 'antd'
import Chart from "react-google-charts"

// Pie Chart options
const chartOptions = {
    legend: 'none',
    chartArea: { left: 8, top: 8, width: "100%", height: "100%" },
    slices: [{ color: "#2BB673" }, { color: "#d91e48" }, { color: "#007fad" }, { color: "#e9a227" }]
}

/**
 * Render user's report.
 * @param {Object} props the object that contains report data.
 * @return {HTML} the HTML DOM of the user's report containing charts, tables, and text summary.
 */
export default function Report(props) {
    // define columns and key of data for each table (eye contact, facial expression, and gesture)
    const columns = [
        {
            title: 'Name',
            dataIndex: 'name',
            key: 'name',
        },
        {
            title: '#',
            dataIndex: 'frequency',
            key: 'frequency',
            defaultSortOrder: 'descend',
            // descendingly sort the rows by number of occurrence.
            sorter: (a, b) => a.frequency - b.frequency,
        },
        {
            title: 'Image',
            key: 'image',
            render: (text, record) => (
                // set View button for example image of each record.
                <Button onClick={() => onShowImage(record)} disabled={record.frequency < 1} size="small">View</Button>
            ),
        },
    ]

    // deconstruct report data from the api data
    const { onShowImage, time, eye_chart, emotion_chart, gesture_chart, eye_table, emotion_table, gesture_table, summary } = props
    return (
        <>
            {/* Chart Section */}
            <h4>Overall Report ({time.minute.toString().padStart(2, '0')}:{time.second.toString().padStart(2, '0')} minutes)</h4>
            <Divider>Eye Contact</Divider>
            <Chart width={'100%'} height={'100%'} chartType="PieChart" loader={<div>Loading Chart</div>}
                data={[
                    ['Type', 'Frequency'],
                    ['Eye Contact', eye_chart.eye_contact],
                    ['No Eye Contact', eye_chart.no_eye_contact],
                ]}

                options={chartOptions}
            />
            <Divider>Facial Expression</Divider>
            <Chart width={'100%'} height={'100%'} chartType="PieChart" loader={<div>Loading Chart</div>}
                data={[
                    ['Type', 'Frequency'],
                    ['Good Facial Expression', emotion_chart.good],
                    ['Bad Facial Expression', emotion_chart.bad],
                ]}

                options={chartOptions}
            />
            <Divider>Gestures</Divider>
            <Chart width={'100%'} height={'100%'} chartType="PieChart" loader={<div>Loading Chart</div>}
                data={[
                    ['Type', 'Frequency'],
                    ['Good Gesture', gesture_chart.good],
                    ['Bad Gesture', gesture_chart.bad],
                ]}

                options={chartOptions}
            />
            <Divider />

            {/* Table Section */}
            <h4>Detailed Report</h4>
            <Divider>Eye Contact</Divider>
            <Table columns={columns} dataSource={eye_table} pagination={false} size='small' />
            <Divider>Facial Expression</Divider>
            <Table columns={columns} dataSource={emotion_table} pagination={false} size='small' footer={() => <small><strong>None:</strong> means your face could not be detected.</small>} />
            <Divider>Gestures</Divider>
            <Table columns={columns} dataSource={gesture_table} pagination={false} size='small' />
            <Divider />

            {/* Summary Section */}
            <h4>Summary &amp; Suggestion</h4>
            <p style={{ fontSize: 12, textAlign: 'justify', textIndent: '1rem' }}>{summary.eye}</p>
            <p style={{ fontSize: 12, textAlign: 'justify', textIndent: '1rem' }}>{summary.emotion}</p>
            <p style={{ fontSize: 12, textAlign: 'justify', textIndent: '1rem' }}>{summary.gesture}</p>
            <Divider />
        </>
    )
}