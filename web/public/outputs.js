import {MAX_AMPLITUDE, MAX_FREQUENCY, PAYLOAD_ADDR} from "./constants.js";

// Helper function to convert ArrayBuffer to hex string
function bufferToHex(buffer) {
    return Array.from(new Uint8Array(buffer))
        .map(byte => byte.toString(16).padStart(2, '0'))
        .join(' ');
}

export function sendPositionUpdate(speedValue, amplitudeValue, forceValue, currentTimeS, airOut, airIn, isPaused) {
    // Normalize speedValue to get frequency in Hz
    const frequency = (speedValue / 1023) * MAX_FREQUENCY;

    // Normalize amplitudeValue to get amplitude in range (-1000 to 1000)
    const amplitude = (amplitudeValue / 1023) * MAX_AMPLITUDE;

    // Calculate positionCommand using sine wave
    const positionCommand = Math.round(amplitude * Math.sin(2 * Math.PI * frequency * currentTimeS));
    if (isPaused) {
        forceValue = 0;
    }

    const buffer = new ArrayBuffer(9);
    const view = new DataView(buffer);
    view.setInt32(0, positionCommand, true);
    view.setInt32(4, forceValue, true);

    let boolArr = 0;
    if (airOut) {
        boolArr += 1 << 0;
    }
    if (airIn) {
        boolArr += 1 << 1;
    }
    view.setUint8(8, boolArr);
    console.log("Sending out these values", positionCommand, forceValue, airOut, airIn);

    console.log('Hex Payload:', bufferToHex(buffer));

    //Send it via POST
    fetch(PAYLOAD_ADDR, {
        mode: 'no-cors',
        method: 'POST',
        headers: {
            'Content-Type': 'application/octet-stream'
        },
        body: buffer
    })
        .then(response => response.text())
        .then(data => console.log('Response:', data))
        .catch(error => console.error('Error:', error));
}
