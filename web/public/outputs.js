const PAYLOAD_ADDR = "http://192.168.8.10:80";

// Helper function to convert ArrayBuffer to hex string
function bufferToHex(buffer) {
    return Array.from(new Uint8Array(buffer))
        .map(byte => byte.toString(16).padStart(2, '0'))
        .join(' ');
}

export function sendPositionUpdate(positionValue, forceValue, airOut, airIn) {
    // Signals to the actuator
    // long positionCommand; // (range: -1000 to 1000)
    // long forceCommand;  // (range: 0 to 1023)
    // bool activated; // Not used
    // bool airOut;  // Set high to open air-out valve
    // bool airIn;   // Set high to open air-in valve
    const buffer = new ArrayBuffer(4);
    const view = new DataView(buffer);
    view.setInt16(0, positionValue, true);
    view.setInt16(2, forceValue, true);

    let boolArr = 0;
    if (airOut) {
        boolArr += 1 << 0;
    }
    if (airIn) {
        boolArr += 1 << 1;
    }
    view.setUint8(3, boolArr);

    console.log('Hex Payload:', bufferToHex(buffer));

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
