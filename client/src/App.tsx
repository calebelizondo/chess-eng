import { useEffect, useState } from 'react';
import './App.css';
import Board from './Board/Board';
import { initEngine } from './Engine/Engine';
import { GameStateProvider } from './GameState/GameState';
import Menu from './Menu/Menu';
import EvalBar from './EvalBar/EvalBar';
import Chat from './Chat/Chat';

function App() {
  const [engineReady, setEngineReady] = useState(false);

  useEffect(() => {
    initEngine().then(() => {
      setEngineReady(true);
    }).catch(err => {
      console.error("Failed to initialize engine:", err);
    });
  }, []);

  if (!engineReady) {
    return <div style={{ color: 'white', textAlign: 'center', marginTop: '2rem' }}>Loading engine...</div>;
  }

  return (
    <GameStateProvider>
      <div
        style={{
          display: "flex",
          flexDirection: "row",
        }}
      >
          <EvalBar />
          <Board />
        <Menu />
      </div>
      <Chat />
    </GameStateProvider>
  );
}

export default App;
