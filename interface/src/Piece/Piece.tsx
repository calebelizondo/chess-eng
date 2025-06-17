import type React from "react"
import getSVG from "./utils/getPieceSVG";
import charToPiece from "./utils/charToPiece";
import { useGameState } from "../GameState/GameState";

const Piece: React.FC<{char: string, index: number}> = ({char, index}) => {
  
  const piece = charToPiece(char);
  const gameState = useGameState();

  if (gameState === null) return <></>;

  const {state, setActivePiece, moveActivePieceTo} = gameState;
  const {activePiece} = state;

  if (piece === null) {
    return (
    <div
      style={{width: "100%", height: "100%"}}
      onClick={() => {
        if (activePiece) {
          moveActivePieceTo(index);
        }
      }}
    />);
  }

  return (
    <img 
      style={{
        width: "100%", 
        height: "100%",
        filter: activePiece === index ? "drop-shadow(0 0 10px rgba(0, 0, 0, 0.5))" : "none"
      }}
      src={getSVG(piece.type, piece.side)} alt={`${piece.side} ${piece.type}`} 
      onClick={() => {
        if (activePiece && piece.side === "BLACK") moveActivePieceTo(index);
        else if (piece.side === "WHITE") setActivePiece(index);
      }}/>
  )
}

export default Piece;