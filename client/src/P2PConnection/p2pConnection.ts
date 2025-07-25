const hostname = "localhost:8080";
const WS_URL = `ws://${hostname}/ws`;

class P2PConnection { 

    private client_id: string | null;
    private peer_id: string | null;
    private peerConnection: RTCPeerConnection;
    private moveChannel: RTCDataChannel;

    private offer: RTCSessionDescriptionInit | null;
    private answer: RTCSessionDescriptionInit | null;
    private socket: WebSocket;

    private socketListener: (event: any) => void; 
    public applyPeerMove: ((move: {to: string, from: string}) => void) | null;
    public startPeerInitiatedGame: (() => void);


    constructor() {

        this.client_id = null;
        this.peer_id = null;
        this.peerConnection = this.peerConnection = this.create_peer_connection();
        this.moveChannel = this.peerConnection.createDataChannel("move");

        this.applyPeerMove = null;
        this.startPeerInitiatedGame = () => {};

        this.moveChannel.onmessage = this.incomingMoveHandler.bind(this);
        this.moveChannel.onopen = () => {console.log("channel opened"); this.startPeerInitiatedGame()};

        this.offer = null;
        this.answer = null;

        this.socket = new WebSocket(WS_URL);
        this.socketListener = this.socket_listener.bind(this);
        this.socket.addEventListener('message', this.socketListener);
    }

    private incomingMoveHandler = (ev: MessageEvent<any>) => {
        try {
            if (this.applyPeerMove) {
                const payload: {type: "move", to: string, from: string} = JSON.parse(ev.data);
                this.applyPeerMove({...payload});
                this.moveChannel.send(JSON.stringify({type: "ack"}));
            } else {
                throw Error("no handler provided to process move!");
            }
        } catch (e) { 
            this.moveChannel.send(JSON.stringify({type: "error", error: e}));
        }
    };

    public async send_move(move: {from: string, to: string}): Promise<void> {
        if (this.moveChannel === null) throw new Error("No peer connection");

        return new Promise<void>((resolve, reject) => {
            this.moveChannel.send(JSON.stringify({...move, type: "move"}));
            this.moveChannel.onmessage = (ev: MessageEvent<any>) => {
                const payload = JSON.parse(ev.data);
                if (payload.type === "error") { 
                    this.moveChannel.onmessage = this.incomingMoveHandler;
                    reject(new Error(payload.error));
                } else if (payload.type === "ack") {
                    console.log("got ack!");
                    this.moveChannel.onmessage = this.incomingMoveHandler;
                    resolve();
                }
            }
        });
    }

    public async init_connection(peer_id: string): Promise<void> {
        this.peer_id = peer_id;

        this.offer = await this.peerConnection.createOffer();
        await this.peerConnection.setLocalDescription(this.offer);

        const offer = {
            type: "offer", 
            from: this.client_id,
            to: this.peer_id, 
            sdp: this.offer.sdp 
        };

        this.socket.send(JSON.stringify(offer));

        return new Promise((resolve, reject) => {

            const timeout = setTimeout(() => 
                reject(new Error("Timed out, try again")), 1500);

            if (!this.moveChannel) {
                return reject(new Error("Channel failed to open"));
            }

            this.moveChannel.onopen = () => {
                clearTimeout(timeout);
                console.log("channel opened")
                resolve();
            };

            this.moveChannel.onerror = (e) => {
                clearTimeout(timeout);
                reject(new Error("Data channel error: " + e))
            }
        });
    }

    private create_peer_connection() {
        const peerConnection = new RTCPeerConnection({
            iceServers: [{ urls: 'stun:stun.l.google.com:19302' }]
        });
        peerConnection.onicecandidate = (event: any) => {
            if (event.candidate) {
                this.socket.send(JSON.stringify({
                    type: "ice",
                    from: this.client_id,
                    to: this.peer_id,
                    candidate: event.candidate
                }));
            }
        };

        return peerConnection;
    }

    private socket_listener = async (event: any) => {
        const payload = JSON.parse(event.data);

        switch (payload.type) {
            case "init":
                this.client_id = payload.value;
                console.log("client code: ", this.client_id);
                break;

            case "offer":

                this.peer_id = payload.from;
                this.peerConnection = this.create_peer_connection();

                const remoteDesc = new RTCSessionDescription({
                    type: "offer",
                    sdp: payload.sdp
                });

                this.peerConnection.setRemoteDescription(remoteDesc);

                this.answer = await this.peerConnection.createAnswer();
                await this.peerConnection.setLocalDescription(this.answer);

                this.socket.send(JSON.stringify({
                    type: "answer",
                    from: this.client_id,
                    to: this.peer_id,
                    sdp: this.answer.sdp
                }));

                this.peerConnection.ondatachannel = (event: RTCDataChannelEvent) => {
                    this.moveChannel = event.channel;
                    this.moveChannel.onmessage = this.incomingMoveHandler.bind(this);
                    this.moveChannel.onopen = () => {
                        this.startPeerInitiatedGame();
                    };
                };
                break;

            case "answer":
                this.peer_id = payload.from;
                const nRemoteDesc = new RTCSessionDescription({
                    type: "answer",
                    sdp: payload.sdp
                });

                await this.peerConnection?.setRemoteDescription(nRemoteDesc);

                break;

            case "ice":
                if (this.peerConnection) {
                    await this.peerConnection.addIceCandidate(new RTCIceCandidate(payload.candidate));
                }
                break;

            case "error":
                throw new Error("Error recieved from signaling server: " + payload.value);
        }
    }

    public get_self_id() {
        return this.client_id;
    }
}


export const CONNECTION = new P2PConnection();