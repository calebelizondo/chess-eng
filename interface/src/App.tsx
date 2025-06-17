import './App.css'
import Board from './Board/Board'
import { GameStateProvider } from './GameState/GameState'
import Menu from './Menu/Menu'

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
            <Menu />
          </div>
        </GameStateProvider>
    </>
  )
}

export default App;
