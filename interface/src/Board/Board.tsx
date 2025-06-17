import { useGameState } from "../GameState/GameState";
import "./styles.css";
import Piece from "../Piece/Piece";

const Board = () => {

  const state = useGameState(); 

  if (state === null || state?.state.positions === null) return <></>;

  return (
    <div className="board">
        {   state.state.positions.split("").reverse().map((char: string, idx: number) => {
            return <div key={idx} className={"square " + (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black")}>
                <Piece char={char} index={idx} />
            </div>
        })
        }
    </div>
  )
}

export default Board;