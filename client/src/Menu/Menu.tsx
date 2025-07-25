import { useEffect, useState } from "react";
import "./styles.css";
import { useGameState } from "../GameState/GameState";
import { CONNECTION } from "../P2PConnection/p2pConnection";
import { ENGINE } from "../Engine/Engine";

const Menu = () => {

    const gameState = useGameState();

    useEffect(() => {
        let interval: NodeJS.Timeout;
        if (id === null) {
            interval = setInterval(() => setID(CONNECTION.get_self_id()), 200);
        }

        return () => clearInterval(interval);
    }, []);

    const [id, setID] = useState<string | null>(CONNECTION.get_self_id());
    const [peerID, setPeerID] = useState<string>("");

    if (gameState === null) return <></>;

    const handleSubmit = (e: React.FormEvent<HTMLFormElement>) => {
        e.preventDefault();

        try {
            CONNECTION.init_connection(peerID);      
            gameState.setState({
                status: "ACTIVE", 
                side: "WHITE", 
                opponent: "PEER", 
                turn: "WHITE", 
                move_n: 0, 
                score: 0, 
                positions: ENGINE.getBoardState(),
                activePiece: null,
                validMoves: null
            });
        } catch {
            alert("failed to connect :(");
        } 
    };

    const { setState, state } = (gameState);

    return (
        <div style={{
            width: "100%",
            display: "grid",
            gridTemplateRows: "1fr 7fr 7fr 1fr", 
            gap: "2em", 
            alignContent: "center"
        }}>

            <div></div>
            <div style={{
                height: "30vh",
                width: "100%"
            }}>
                <div style={{
                padding: "2em",
                height: "50%", 
                alignItems: "center",
        }}>
            <form onSubmit={handleSubmit}>
                <div>
                    <div 
                        className="input__container" 
                        // @ts-ignore
                        style={{"--code_label": `"YOUR CODE IS ${id}"`}}
                    >
                    <div className="shadow__input"></div>
                    <input
                        type="text"
                        name="username"
                        className="input__search"
                        placeholder="ENTER A FRIEND'S CODE"
                        onChange={(e) => setPeerID(e.target.value)}
                    />

                    <button className="input__button__shadow">
                        
                        <h3>CONNECT</h3>
                    </button>
                    </div>
                </div>
            </form>
        </div>
            </div>
            <div style={{
                padding: "2em",
                height: "50%", 
                alignItems: "center", 
            }}>
                <div>
                    <div className="input__container" style={{
                        display: "flex",
                        alignContent: "space-between"
                        }}>
                    <div className="shadow__input"></div>
                    <h3 style={{color: "black"}}>...OR PLAY THE ENGINE</h3>

                    <button className="input__button__shadow" onClick={() => {
                                ENGINE.resetGame();
                                setState({...state, status: "ACTIVE", opponent: "ENGINE", side: "WHITE", turn: "WHITE"});
                            }}>   
                        <h3>START</h3>
                    </button>
                    </div>
                </div>
            </div>
        </div>
    );
}

export default Menu