import { useGameState } from "../GameState/GameState";
import "./styles.css";
import Piece from "../Piece/Piece";
import type { Space } from "./types";

const Board = () => {

  const state = useGameState(); 

  if (state === null || state?.state.positions === null) return <></>;

  const determineSquareColor = (idx: number) => {
    //if (state.state.validMoves !== null && state.state.validMoves.split("").reverse()[idx] === 'x') return "blue";
    return (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black");
  };

  const determineSpace: (idx: number) => Space = (idx: number) => {
      const row = 8 - Math.floor(idx / 8);
      const fileIndex = idx % 8;
      const file = String.fromCharCode('a'.charCodeAt(0) + fileIndex) as unknown as File;
      if (row < 1 || row > 8) throw new Error("Invalid row");
      if ((file as unknown as String) < 'a' || (file as unknown as String) > 'h') throw new Error("Invalid file");

      return `${file}${row}` as Space;
  };

  return (
    <>
      <div className="board">
          {   state.state.positions.split("").map((char: string, idx: number) => {
              return <div key={idx} className={"square " + determineSquareColor(idx)}>
                  <Piece char={char} index={idx} space={determineSpace(idx)} validMove={(state.state.validMoves !== null && state.state.validMoves.split("").reverse()[idx] === 'x')}/>
                  { (state.state.validMoves !== null && state.state.validMoves.split("").reverse()[idx] === 'x') &&
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