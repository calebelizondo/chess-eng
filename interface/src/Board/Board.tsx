import { useGameState } from "../GameState/GameState";
import "./styles.css";
import Piece from "../Piece/Piece";

const Board = () => {

  const state = useGameState(); 

  if (state === null || state?.state.positions === null) return <></>;

  const determineSquareColor = (idx: number) => {
    if (state.state.validMoves !== null && state.state.validMoves.split("").reverse()[idx] === 'x') return "blue";
    return (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black");
  };

  return (
    <div className="board">
        {   state.state.positions.split("").reverse().map((char: string, idx: number) => {
            return <div key={idx} className={"square " + determineSquareColor(idx)}>
                <Piece char={char} index={idx} />
            </div>
        })
        }
    </div>
  )
}

export default Board;