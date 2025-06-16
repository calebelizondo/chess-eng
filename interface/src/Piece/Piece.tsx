import type React from "react"
import type { PieceType, Side } from "./types";
import getSVG from "./utils/getPieceSVG";

const Piece: React.FC<{type: PieceType, side: Side}> = ({type, side}) => {
  
  
  
    return (
    <img style={{width: "100%", height: "100%"}}src={getSVG(type, side)} alt={`${side} ${type}`} />
  )
}

export default Piece;