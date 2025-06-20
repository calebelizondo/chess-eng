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

  constructor(module: any) {
    const print = module.cwrap('printCurrentBoardState', null, []);
    const resetGame = module.cwrap('resetGame', 'number', []);
    const getBoardStatePtr = module.cwrap('getCurrentBoardState', 'number', []);
    const getMovePtr = module.cwrap('movePiece', 'number', ['string', 'string', 'boolean', 'boolean', 'boolean', 'string']);
    const getMoves = module.cwrap('getValidPieceMoves', 'number', ['string']);

    this.printBoard = () => {
      print();
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
  }
}

const module = await loadEngineModule();
export const ENGINE = new Engine(module);