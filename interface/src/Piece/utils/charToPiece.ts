import type { Piece } from "../types";

export default function charToPiece(char: string): Piece | null {
    switch (char) {
        case ('.'): return null;
        case ('R'): return {type: 'ROOK', side: 'BLACK'};
        case ('B'): return {type: 'BISHOP', side: 'BLACK'};
        case ('N'): return {type: 'KNIGHT', side: 'BLACK'};
        case ('K'): return {type: 'KING', side: 'BLACK'};
        case ('Q'): return {type: 'QUEEN', side: 'BLACK'};
        case ('P'): return {type: 'PAWN', side: 'BLACK'};
        case ('r'): return {type: 'ROOK', side: 'WHITE'};
        case ('b'): return {type: 'BISHOP', side: 'WHITE'};
        case ('n'): return {type: 'KNIGHT', side: 'WHITE'};
        case ('k'): return {type: 'KING', side: 'WHITE'};
        case ('q'): return {type: 'QUEEN', side: 'WHITE'};
        case ('p'): return {type: 'PAWN', side: 'WHITE'};

        default: 
            throw new Error('unrecognized piece character');
        
    }
}