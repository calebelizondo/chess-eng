import { useState } from "react";
import { CONNECTION } from "../P2PConnection/p2pConnection";

const Chat = () => {

    const [id, setID] = useState<string | null>(CONNECTION.get_self_id());
    const [peerID, setPeerID] = useState<string>("");

    const [msg, setMsg] = useState<string>("");


    const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
        e.preventDefault();
        CONNECTION.init_connection(peerID);
    };

    const handleChat = (e: React.FormEvent<HTMLFormElement>) => {
        e.preventDefault();
        CONNECTION.chat(msg);
    };

    return (
        <>
            <button onClick={() => setID(CONNECTION.get_self_id())}>refresh ID</button>
            <div>ID: {id}</div>
            <form onSubmit={handleSubmit}>
                <input name="peerID" type="text" onChange={(e) => setPeerID(e.target.value)}/>
                <button type="submit">Connect</button>
            </form>
            <form onSubmit={handleChat}>
                <input type="text" onChange={(e) => setMsg(e.target.value)}/>
                <button type="submit">Chat</button>
            </form>
        </>
    );
}

export default Chat