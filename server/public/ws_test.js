console.log("hello world");

const hostname = "localhost:8080";
const WS_URL = `ws://${hostname}/ws_connect`;

const socket = new WebSocket(WS_URL);


socket.addEventListener("open", (event) => {
    console.log("websocket connected!");
    socket.send("hello server!");
});

socket.addEventListener("message", (event) => {
    console.log("received message!", event.data);
});


socket.addEventListener("error", (event) => {
    console.log("WS error", event.data);
});
