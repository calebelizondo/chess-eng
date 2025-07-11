import React, { createContext, useContext, useEffect, useState } from "react";
import { ENGINE } from "../Engine/Engine";
import type { Space } from "../Board/types";

// export interface GameState {
//     positions: String, 
//     activePiece: number | null,
//     validMoves: String | null
// }

export type GameState = 
    {
        score: number,
        positions: String,
        activePiece: null,
        validMoves: null    
    } 
        |
    {
        score: number,
        positions: String, 
        activePiece: Space, 
        validMoves: String
    };


export interface GameStateContextType {
    state: GameState,
    setState: (gs: GameState) => void,
    setActivePiece: (ap: Space | null) => void,
    moveActivePieceTo: (to: Space) => void,
}



const GameStateContext = createContext<GameStateContextType | null>(null);

export const GameStateProvider: React.FC<{children: React.ReactNode}> = ({children}) => {

    const [gameState, setGameState] = useState<GameState>({
        score: 0,
        positions: "",
        activePiece: null,
        validMoves: null,
    });


    const moveActivePieceTo = (to: Space) => {
        // console.log(`move active from ${gameState.activePiece} piece to ${to}`);
        if (gameState === null || gameState.activePiece === null) throw new Error("attempting to move null piece!");
        const newBoardPositions = ENGINE.move(gameState.activePiece, to);
        const newScore = ENGINE.getScore();
        setGameState({positions: newBoardPositions, score: newScore, activePiece: null, validMoves: null});
    };

    const setActivePiece = async (ap: Space | null) => {
        if (gameState === null || ap === null) return;
        const validMoves = await ENGINE.getValidMoves(ap);
        setGameState({...gameState, activePiece: ap, validMoves});
    };

    useEffect(() => {
        setGameState({positions: ENGINE.getBoardState(), score: 0, activePiece: null, validMoves: null});
    }, []);
    

    console.log("current score:", ENGINE.getScore());

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
