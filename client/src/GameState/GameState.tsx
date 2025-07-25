import React, { createContext, useContext, useEffect, useState } from "react";
import { ENGINE } from "../Engine/Engine";
import type { Space } from "../Board/types";
import { CONNECTION } from "../P2PConnection/p2pConnection";

export type GameState = 
    {
        status: "NOT_STARTED" | "OVER",
        opponent: null,
        side: "WHITE",
        turn: null,
        move_n: number,
        score: number,
        positions: String,
        activePiece: null,
        validMoves: null    
    } 
        |
    {   
        status: "ACTIVE",
        opponent: "ENGINE" | "PEER",
        side: "WHITE" | "BLACK",
        turn: "WHITE" | "BLACK", 
        move_n: number,
        score: number,
        positions: String, 
        activePiece: Space | null, 
        validMoves: String | null
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
        status: "NOT_STARTED",
        opponent: null,
        side: "WHITE",
        turn: null,
        move_n: 0,
        score: 0,
        positions: ENGINE.getBoardState(),
        activePiece: null,
        validMoves: null,
    });

    useEffect(() => {
        CONNECTION.startPeerInitiatedGame = () => {
            ENGINE.resetGame(); 
            setGameState({
                status: "ACTIVE", 
                side: "BLACK", 
                opponent: "PEER", 
                turn: "WHITE", 
                move_n: 0, 
                score: 0, 
                positions: ENGINE.getBoardState(),
                activePiece: null,
                validMoves: null
            });
        }
        
        CONNECTION.applyPeerMove = (move) => {
            // @ts-ignore
            ENGINE.move(move.from, move.to);
            setGameState((state) => {return {
                status: "ACTIVE", 
                side: state.side, 
                opponent: "PEER", 
                turn: state.side === "WHITE" ? "WHITE" : "BLACK", 
                move_n: state.move_n + 1, 
                score: 0, 
                positions: ENGINE.getBoardState(),
                activePiece: null,
                validMoves: null
            }});
        };
    }, []);

    useEffect(() => {
        console.log(gameState);
        console.log("iswhtsturn", ENGINE.isWhitesTurn());
    }); 

    const opponent_side = (gameState.side === "WHITE") ? "BLACK" : "WHITE";

    const moveActivePieceTo = (to: Space) => {
        if (gameState.status !== "ACTIVE") return;
        if (gameState === null || gameState.activePiece === null || gameState.side === null) throw new Error("attempting to move null piece!");

        if (gameState.opponent === "ENGINE") {

            //apply the player move
            let newBoardPositions = ENGINE.move(gameState.activePiece, to);
            let newScore = ENGINE.getScore();
            setGameState({...gameState, activePiece: null, validMoves: null, move_n: gameState.move_n + 1, score: newScore, positions: newBoardPositions});

            //apply the enemy move
            newBoardPositions = ENGINE.engineMove();
            newScore = ENGINE.getScore();
            setGameState({...gameState, activePiece: null, validMoves: null, move_n: gameState.move_n + 1, score: newScore, positions: newBoardPositions});

        } else if (gameState.opponent === "PEER") {
            //wait for ack
            CONNECTION.send_move({from: gameState.activePiece, to}).then(() => {
                // @ts-ignore
                let newBoardPositions = ENGINE.move(gameState.activePiece, to);
                let newScore = ENGINE.getScore();
                setGameState({...gameState, activePiece: null, validMoves: null, move_n: gameState.move_n + 1, score: newScore, positions: newBoardPositions, turn: opponent_side});
            }).catch((e) => {
                alert(e);
            });
        }
    };

    const setActivePiece = async (ap: Space | null) => {
        if (gameState.status !== "ACTIVE" || ap === null) return;
        const validMoves = await ENGINE.getValidMoves(ap);
        setGameState({...gameState, activePiece: ap, validMoves});
    };

    return (
        <GameStateContext.Provider value={
            {
                state: gameState,
                setState: setGameState,
                setActivePiece,
                moveActivePieceTo
            }
            }>
            {children}
        </GameStateContext.Provider>
    )
}

export const useGameState = () => {
    const context = useContext(GameStateContext);
    if (!context) {
        return null;
    }

    return context;

};
