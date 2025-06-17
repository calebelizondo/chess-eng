import React, { createContext, useContext, useEffect, useState } from "react";
import { ENGINE } from "../Engine/Engine";

export interface GameState {
    positions: String, 
    activePiece: number | null
}

export interface GameStateContextType {
    state: GameState,
    setState: (gs: GameState) => void,
    setActivePiece: (ap: number | null) => void,
    moveActivePieceTo: (to: number) => void,
}



const GameStateContext = createContext<GameStateContextType | null>(null);

export const GameStateProvider: React.FC<{children: React.ReactNode}> = ({children}) => {

    const [gameState, setGameState] = useState<GameState>({
        positions: "",
        activePiece: null
    });


    useEffect(() => {console.log("active piece", gameState.activePiece)}, [gameState])


    const moveActivePieceTo = (to: number) => {
        console.log(`move active from ${gameState.activePiece} piece to ${to}`);
        if (gameState === null || gameState.activePiece === null) throw new Error("attempting to move null piece!");
        ENGINE.move(gameState.activePiece, to).then((result: String) => setGameState({positions: result, activePiece: null})).catch(() =>
            {throw new Error("Error triggered when attempting to move piece")});
    };

    const setActivePiece = (ap: number | null) => {
        if (gameState === null) return;
        setGameState({...gameState, activePiece: ap});
    };

    useEffect(() => {
        ENGINE.getBoardState().then((result: String) => setGameState({positions: result, activePiece: null})).catch(() => 
            {throw new Error("Could not get game state from Engine!")});
    }, []);
    

    return (
        <GameStateContext.Provider value={
            {
                state: gameState,
                setState: setGameState,
                setActivePiece,
                moveActivePieceTo
            }
        }>{children}</GameStateContext.Provider>
    )
}

export const useGameState = () => {
    const context = useContext(GameStateContext);
    if (!context) {
        return null;
    }

    return context;

};
