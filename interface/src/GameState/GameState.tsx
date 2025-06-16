import React, { createContext, useContext, useEffect, useState } from "react";
import { ENGINE } from "../Engine/loadEngine";


export interface GameState {
    positions: String | null,
    turn: "WHITE" | "BLACK",
    state: "ACTIVE" | "CHECK" | "CHECKMATE"
}

const GameStateContext = createContext<GameState | null>(null);

export const GameStateProvider: React.FC<{children: React.ReactNode}> = ({children}) => {

    const [positions, setPositions] = useState<String | null>(null);
    const [turn, setTurn] = useState<"WHITE" | "BLACK">("WHITE");
    const [state, setState] = useState<"ACTIVE" | "CHECK" | "CHECKMATE">("ACTIVE");

    useEffect(() => {
        ENGINE.getBoardState().then((result: String) => setPositions(result)).catch(() => 
            {throw new Error("Could not get game state from Engine!")});
    }, []);
    

    return (
        <GameStateContext.Provider value={{positions, turn, state}}>{children}</GameStateContext.Provider>
    )
}

export const useGameState = () => {
    const context = useContext(GameStateContext);
    if (!context) {
        return null;
    }

    return context;

};
