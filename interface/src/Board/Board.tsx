import Piece from "../Piece/Piece";
import "./styles.css";

const Board = () => {
  return (
    <div className="board">
        {   Array.from({length: 64}).map((_, idx: number) => {
            return <div key={idx} className={"square " + (((idx + Math.floor(idx / 8)) % 2) == 0 ? "white" : "black")}>
                <Piece type="BISHOP" side="WHITE"></Piece>
            </div>
        })
        }
    </div>
  )
}

export default Board;