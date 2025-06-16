import { useGameState } from "../GameState/GameState";
import "./styles.css";
import renderPiece from "./utils/renderPiece";

const Board = () => {

  const state = useGameState(); 

  if (state === null || state?.positions === null) return <></>;

  return (
    <div className="board">
        {   state.positions.split("").map((char: string, idx: number) => {
            return <div key={idx} className={"square " + (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black")}>
                {renderPiece(char)}
            </div>
        })
        }
    </div>
  )
}

export default Board;