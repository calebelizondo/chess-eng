import { useEffect } from 'react'
import './App.css'
import Board from './Board/Board'
import { GameStateProvider } from './GameState/GameState'
import Engine, { loadEngine } from './EngineAPI/Engine.js';

function App() {


  useEffect(() => {
    loadEngine();
  }, []);
  return (
    <>
      <Engine />
      <GameStateProvider >
        <Board />
      </GameStateProvider>
    </>
  )
}

export default App;
