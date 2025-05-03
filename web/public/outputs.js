const PAYLOAD_ADDR = "http://0.0.0.0:7070";

export function sendPositionUpdate(positionValue, forceValue, airOut, airIn) {
    // Signals to the actuator
    // long positionCommand; // (range: -1000 to 1000)
    // long forceCommand;  // (range: 0 to 1023)
    // bool activated; // Not used
    // bool airOut;  // Set high to open air-out valve
    // bool airIn;   // Set high to open air-in valve
    const buffer = new ArrayBuffer(3);
    const view = new DataView(buffer);
    view.setInt16(0, positionValue);
    view.setInt16(1, forceValue);

    let boolArr = 0;
    if (airOut) {
        boolArr += 1 << 0;
    }
    if (airIn) {
        boolArr += 1 << 1;
    }
    view.setUint8(2, boolArr);

    // Send it via POST
    fetch(PAYLOAD_ADDR, {
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
