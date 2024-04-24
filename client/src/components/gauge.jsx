import Plot from 'react-plotly.js';

import React from 'react'

const Gauge = ({value, title, width=400, height=250}) => {
    return (
        <Plot
            data={[
                {
                    domain: { x: [0, 1], y: [0, 1] },
                    value: value,
                    title: { text: title },
                    type: 'indicator',
                    mode: 'gauge+number',
                    marker: { color: 'red', font:{size:10} },
                },
                // {type: 'bar', x: [1, 2, 3], y: [2, 5, 3]},
            ]}
            config={{displayModeBar: false}}
            layout={{ width: width, height: height, margin: { t: 0, b: 0 }, paper_bgcolor: "rgba(0,0,0,0)"}}
        />
    )
}

export default Gauge