const hostname = "localhost:8080";
const WS_URL = `ws://${hostname}/ws_connect`;
const socket = new WebSocket(WS_URL, 'ws-protocol');


const input_element = document.getElementById("code_field");
const button = document.getElementById("submit_button");
const msg_btn = document.getElementById("send_msg");
const msg_fld = document.getElementById("msg_field");


let client_id;
let peer_id;
let peerConnection;
let dataChannel;
let offer;

socket.addEventListener("open", async () => {
    console.log("WebSocket connected!");


    peerConnection = createPeerConnection();


    dataChannel = peerConnection.createDataChannel("chat");
    setupDataChannel(dataChannel);


    offer = await peerConnection.createOffer();
    await peerConnection.setLocalDescription(offer);

    socket.send("hello server!");
});

socket.addEventListener("message", async (event) => {
    const payload = JSON.parse(event.data);
    console.log("Received:", payload);

    let remoteDesc;

    switch (payload.type) {
        case "init":
            client_id = payload.id;
            break;

        case "offer":
            peer_id = payload.from;
            peerConnection = createPeerConnection();

            peerConnection.ondatachannel = (event) => {
                dataChannel = event.channel;
                setupDataChannel(dataChannel);
            };

            remoteDesc = new RTCSessionDescription({
                type: "offer",
                sdp: payload.sdp
            });

            await peerConnection.setRemoteDescription(remoteDesc);

            const answer = await peerConnection.createAnswer();
            await peerConnection.setLocalDescription(answer);

            socket.send(JSON.stringify({
                type: "answer",
                from: client_id,
                to: peer_id,
                sdp: answer.sdp
            }));
            break;

        case "answer":
            peer_id = payload.from;
            remoteDesc = new RTCSessionDescription({
                type: "answer",
                sdp: payload.sdp
            });
            await peerConnection.setRemoteDescription(remoteDesc);
            break;

        case "ice":
            if (peerConnection) {
                await peerConnection.addIceCandidate(new RTCIceCandidate(payload.candidate));
            }
            break;

        case "error":
            console.error("Server error:", payload);
            break;
    }
});

socket.addEventListener("error", (event) => {
    console.error("WebSocket error:", event);
});


button.addEventListener("click", () => {
    sendOffer(client_id, input_element.value);
});


msg_btn.addEventListener("click", () => {
    if (dataChannel && dataChannel.readyState === "open") {
        const msg = msg_fld.value;
        dataChannel.send(msg);
        console.log("Sent message:", msg);
    } else {
        console.warn("Data channel not open.");
    }
});


function createPeerConnection() {
    const pc = new RTCPeerConnection({
        iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
    });

    pc.onicecandidate = (event) => {
        if (event.candidate) {
            socket.send(JSON.stringify({
                type: "ice",
                from: client_id,
                to: peer_id,
                candidate: event.candidate
            }));
        }
    };

    return pc;
}


function setupDataChannel(channel) {
    channel.onopen = () => {
        console.log("Data channel open!");
    };

    channel.onmessage = (e) => {
        console.log("Received message:", e.data);
    };
}

function sendOffer(selfId, targetId) {
    peer_id = targetId;

    socket.send(JSON.stringify({
        type: "offer",
        from: selfId,
        to: targetId,
        sdp: offer.sdp
    }));
}