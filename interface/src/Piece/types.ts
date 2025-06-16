

export type PieceType = "QUEEN" | "ROOK" | "KING" | "PAWN" | "BISHOP" | "KNIGHT";
export type Side = "WHITE" | "BLACK";

export type Piece = {type: PieceType, side: Side}; 