import { useEffect, useState } from 'react';
import './App.css';
import Board from './Board/Board';
import { initEngine } from './Engine/Engine';
import { GameStateProvider } from './GameState/GameState';
import Menu from './Menu/Menu';
import EvalBar from './EvalBar/EvalBar';

type ScreenDim = {width: number; height: number};

function App() {
  const [engineReady, setEngineReady] = useState(false);
  const [dim, setDim] = useState<ScreenDim>({width: window.innerWidth, height: window.innerHeight});

  useEffect(() => {
    initEngine().then(() => {
      setEngineReady(true);
    }).catch(err => {
      console.error("Failed to initialize engine:", err);
    });

    const handleResize = () => setDim({width: window.innerWidth, height: window.innerHeight});
    window.addEventListener("resize", handleResize);

    return () => window.removeEventListener("resize", handleResize);

  }, []);

  if (!engineReady) {
    return <div style={{ color: 'white', textAlign: 'center', marginTop: '2rem' }}>Loading engine...</div>;
  }

  const screenSize = (dim.width > 760) ? 'large' : 'small';

  return (

    <div>
      <GameStateProvider>
        { screenSize === 'large' && (
          <div
            style={{
              display: "flex", 
              flexDirection: "row",
              gap: 0,
              alignContent: "space-between",
              margin: "1em"
            }}
          >
              <div style={{
                height: (dim.height > dim.width) ? "90vw" : "90vh",
                width: (dim.height > dim.width) ? "90vw" : "90vh", 
                minHeight: (dim.height > dim.width) ? "90vw" : "90vh",
                minWidth: (dim.height > dim.width) ? "90vw" : "90vh",
              }}>
                <Board />
              </div>
              <Menu />
          </div>
        )}
        { screenSize === 'small' && (
          <div
            style={{
              display: "flex",
              flexDirection: "column",
            }}
          >

              <div>
                <EvalBar />
                <Board />
              </div>
              <Menu />
          </div>
        )}

      </GameStateProvider>
    </div>
  );
}

export default App;
