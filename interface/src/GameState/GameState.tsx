import React, { createContext, useContext, useState } from "react";


export interface GameState {
    positions: string[],
    turn: "WHITE" | "BLACK",
    state: "ACTIVE" | "CHECK" | "CHECKMATE"
}

const GameStateContext = createContext<GameState | null>(null);

export const GameStateProvider: React.FC<{children: React.ReactNode}> = ({children}) => {

    const [positions, setPositions] = useState<string[]>([]);
    const [turn, setTurn] = useState<"WHITE" | "BLACK">("WHITE");
    const [state, setState] = useState<"ACTIVE" | "CHECK" | "CHECKMATE">("ACTIVE");
    

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
