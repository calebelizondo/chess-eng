import { useEffect, useState } from 'react';
import './App.css';
import Board from './Board/Board';
import { initEngine } from './Engine/Engine';
import { GameStateProvider } from './GameState/GameState';
import Menu from './Menu/Menu';

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
          display: "grid",
          gridTemplateRows: "1fr 1fr",
          gridTemplateColumns: "1fr"
        }}
      >
        <Board />
        <Menu />
      </div>
    </GameStateProvider>
  );
}

export default App;
