import { useGameState } from "../GameState/GameState";
import "./styles.css";
import Piece from "../Piece/Piece";
import type { Space } from "./types";

const determineSpace: (idx: number) => Space = (idx: number) => {
    const row = 8 - Math.floor(idx / 8);
    const fileIndex = idx % 8;
    const file = String.fromCharCode('a'.charCodeAt(0) + fileIndex) as unknown as File;
    if (row < 1 || row > 8) throw new Error("Invalid row");
    if ((file as unknown as String) < 'a' || (file as unknown as String) > 'h') throw new Error("Invalid file");

    return `${file}${row}` as Space;
};


const Board = () => {

  const state = useGameState(); 

  if (state === null || state?.state.positions === null) return <></>;

  const {side} = state.state;

  const determineSquareColor = (idx: number) => {
    return (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black");
  };

  const positions = (side === "WHITE") 
    ? state.state.positions.split("") 
    : state.state.positions.split("").reverse(); 

  const space_labels = (side === "WHITE") 
    ? Array.from({length: 64}).map((_, i) => determineSpace(i))
    : Array.from({length: 64}).map((_, i) => determineSpace(i)).reverse();

  const valid_moves = ((side === "BLACK")
    ? state.state.validMoves
    : state.state.validMoves?.split('').reverse().join('')) ?? null;

  return (
    <>
      <div className="board">
          {   positions.map((char: string, idx: number) => {
              return <div key={idx} className={"square " + determineSquareColor(idx)}>
                  <Piece char={char} index={idx} space={space_labels[idx]} validMove={(valid_moves !== null && valid_moves[idx] === 'x')}/>
                  { (valid_moves !== null && valid_moves[idx] === 'x') &&
                    <div className="validmove"/>
                  }
              </div>
          })
          }
      </div>
    </>
  )
}

export default Board;