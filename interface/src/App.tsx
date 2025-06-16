import { useEffect } from 'react'
import './App.css'
import Board from './Board/Board'
import { GameStateProvider } from './GameState/GameState'
import { ENGINE } from './Engine/loadEngine'

function App() {

  return (
    <>
        <GameStateProvider >

          <div style={{
            display: "grid",
            gridTemplateRows: "1fr 1fr",
            gridTemplateColumns: "1fr"
          }}>
            <Board />
            <div>
              <button>Reset Board</button>
              <button>Print board</button>
            </div>
          </div>
        </GameStateProvider>
    </>
  )
}

export default App;
