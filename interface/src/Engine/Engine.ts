class Engine {
  private module: any = null;

  private async loadEngineModule(): Promise<void> {
    if (this.module != null) return;

    const { default: url } = await import('/build/main.js?url');

    this.module = await new Promise<any>((resolve, reject) => {
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
  }

  public async printBoard(): Promise<void> {
    await this.loadEngineModule();

    const print = this.module.cwrap('printCurrentBoardState', null, []);
    print();
  }

  public async resetGame(): Promise<String> {
    await this.loadEngineModule();

    const reset = this.module.cwrap('resetGame', 'number', []);
    const ptr = reset();
    const boardStr = this.module.UTF8ToString(ptr);
    return boardStr;
  }

  public async getBoardState(): Promise<String> {
    await this.loadEngineModule();

    const getBoardStatePtr = this.module.cwrap('getCurrentBoardState', 'number', []);
    const ptr = getBoardStatePtr();
    const boardStr = this.module.UTF8ToString(ptr);
    return boardStr;
  }

  public async move(from: number, to: number): Promise<String> {
    await this.loadEngineModule();
    const m = this.module.cwrap('movePiece', 'number', ['number', 'number']);
    const ptr = m(from, to);
    const boardStr = this.module.UTF8ToString(ptr);
    return boardStr;
  }

  public async getValidMoves(idx: number): Promise<String> {
    await this.loadEngineModule();
    const getMoves = this.module.cwrap('getValidPieceMoves', 'number', ['number']);
    const ptr = getMoves(idx);
    const moveStr = this.module.UTF8ToString(ptr);
    return moveStr;
  }
}

export const ENGINE = new Engine();
