import type { ReactNode } from "react";
import charToPiece from "../../Piece/utils/charToPiece";
import Piece from "../../Piece/Piece";

export default function renderPiece(char: string): ReactNode {

    const piece = charToPiece(char);
    if (piece) return <Piece type={piece.type} side={piece.side} />
    else return <></>
};
