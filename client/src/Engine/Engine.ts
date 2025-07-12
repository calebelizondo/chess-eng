import type { Space } from '../Board/types';

async function loadEngineModule(): Promise<any> {

    const { default: url } = await import('/build/main.js?url');

    const module = await new Promise<any>((resolve, reject) => {
      const script = document.createElement('script');
      script.src = url;

      script.onload = () => {
        (window as any).EngineModule()
          .then(resolve)
          .catch(reject);
      };

      script.onerror = reject;
      document.body.appendChild(script);
    });

  return module;
}


class Engine {
  public printBoard: () => void;
  public resetGame: () => String;
  public getBoardState: () => String;
  public move: (from: Space, to: Space) => String; 
  public getValidMoves: (space: Space) => String;
  public isPlayerInCheck: () => boolean;
  public isEnemyInCheck: () => boolean;
  public getScore: () => number;
  public isWhitesTurn: () => boolean;

  constructor(module: any) {
    const print = module.cwrap('printCurrentBoardState', null, []);
    const resetGame = module.cwrap('resetGame', 'number', []);
    const getBoardStatePtr = module.cwrap('getCurrentBoardState', 'number', []);
    const getMovePtr = module.cwrap('movePiece', 'number', ['string', 'string', 'boolean', 'boolean', 'boolean', 'string']);
    const getMoves = module.cwrap('getValidPieceMoves', 'number', ['string']);
    const getPlayerInCheck = module.cwrap('isPlayerInCheck', 'boolean', []);
    const getEnemyInCheck = module.cwrap('isEnemyInCheck', 'boolean', []);
    const getPlayerScore = module.cwrap('getScore', 'number', []);
    const isWhitesTurn = module.cwrap('isWhitesTurn', 'boolean', []);

    this.printBoard = () => {
      print();
    }

    this.isPlayerInCheck = () => {
      // console.log("player in check?", getPlayerInCheck());
      return getPlayerInCheck();
    }

    this.isEnemyInCheck = () => {
      // console.log("enemy in check?", getEnemyInCheck());
      return getEnemyInCheck();
    }

    this.resetGame = () => {
      const ptr = resetGame();
      const boardStr = module.UTF8ToString(ptr);
      return boardStr;
    }

    this.getBoardState = () => {
      const ptr = getBoardStatePtr();
      const boardStr = module.UTF8ToString(ptr);
      return boardStr;
    }

    this.move = (from: Space, to: Space) => {

      //args are: 
      //1: piece to move
      //2: where to move
      //3: castle?
      //4: enpassant?
      //5: pawn promotion?
      //6: promote to what?

      const ptr = getMovePtr(from, to, false, false, false, '');
      const boardStr = module.UTF8ToString(ptr);
      return boardStr;
    }

    this.getValidMoves = (space: Space) => {
      const ptr = getMoves(space);
      const moveStr = module.UTF8ToString(ptr);
      return moveStr;
    }

    this.getScore = () => {
      return getPlayerScore();
    }

    this.isWhitesTurn = () => {
      return isWhitesTurn();
    }
  }
}

let ENGINE: Engine;

export async function initEngine(): Promise<void> {
  const module = await loadEngineModule();
  ENGINE = new Engine(module);
}

export { ENGINE };