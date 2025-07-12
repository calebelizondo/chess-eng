import type { Piece } from "../types";

export default function charToPiece(char: string): Piece | null {
    switch (char) {
        case ('.'): return null;
        case ('R'): return {type: 'ROOK', side: 'WHITE'};
        case ('B'): return {type: 'BISHOP', side: 'WHITE'};
        case ('N'): return {type: 'KNIGHT', side: 'WHITE'};
        case ('K'): return {type: 'KING', side: 'WHITE'};
        case ('Q'): return {type: 'QUEEN', side: 'WHITE'};
        case ('P'): return {type: 'PAWN', side: 'WHITE'};
        case ('r'): return {type: 'ROOK', side: 'BLACK'};
        case ('b'): return {type: 'BISHOP', side: 'BLACK'};
        case ('n'): return {type: 'KNIGHT', side: 'BLACK'};
        case ('k'): return {type: 'KING', side: 'BLACK'};
        case ('q'): return {type: 'QUEEN', side: 'BLACK'};
        case ('p'): return {type: 'PAWN', side: 'BLACK'};

        default: 
            throw new Error('unrecognized piece character');
        
    }
}