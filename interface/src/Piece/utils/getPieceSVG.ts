import type { PieceType, Side } from "../types";

function getSVG(piece: PieceType, side: Side) {


    let filename = "Chess_";
    if (piece == "QUEEN") {
        filename += "q";
    } else if (piece == "ROOK") {
        filename += "r";
    } else if (piece == "KING") {
        filename += "k";
    } else if (piece == "PAWN") {
        filename += "p";
    } else if (piece == "BISHOP") {
        filename += "b";
    } else if (piece == "KNIGHT") {
        filename += "n";
    } else {
        throw new Error("failed to find piece type");
    }

    if (side == "WHITE") {
        filename += "l";
    } else {
        filename += "d";
    }

    filename += "t45.svg";

    return filename;
}

export default getSVG;