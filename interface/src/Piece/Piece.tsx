import type React from "react"
import getSVG from "./utils/getPieceSVG";
import charToPiece from "./utils/charToPiece";
import { useGameState } from "../GameState/GameState";
import type { Space } from "../Board/types";
import "./styles.css";
import { ENGINE } from "../Engine/Engine";

const Piece: React.FC<{char: string, index: number, space: Space, validMove: boolean}> = ({char, index, space, validMove}) => {
  
  const piece = charToPiece(char);
  const gameState = useGameState();

  if (gameState === null) return <></>;

  const {state, setActivePiece, moveActivePieceTo} = gameState;
  const {activePiece} = state;


  const isChecked = () => {
    if (piece?.type === "KING") {
      if (piece.side === "WHITE") return ENGINE.isPlayerInCheck();
      else return ENGINE.isEnemyInCheck();
    }

    return false;
  }

  if (piece === null) {
    return (
    <div
      style={{width: "100%", height: "100%"}}
      onClick={() => {
        if (activePiece && validMove) {
          moveActivePieceTo(space);
        } else {
          setActivePiece(null);
        }
      }}
    />);
  }

  return (
    <>
      <img 
        style={{
          width: "100%", 
          height: "100%",
          filter: isChecked() ? "drop-shadow(0 0 15px rgba(255, 0, 0, 0.5))" :
            (activePiece === space ? "drop-shadow(0 0 10px rgba(0, 0, 0, 0.5))" : "none")
        }}
        src={getSVG(piece.type, piece.side)} alt={`${piece.side} ${piece.type}`} 
        onClick={() => {
          if (validMove) moveActivePieceTo(space);
          else if (piece.side === "WHITE") setActivePiece(space);
          else setActivePiece(null);
        }}/>
    </>
  )
}

export default Piece;