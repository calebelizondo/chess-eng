import { useGameState } from '../GameState/GameState';

const EvalBar = () => {
    const context = useGameState();
    if (!context) return <></>;

    const { score } = context.state;

    // clamp score between -50 and 50
    const clampedScore = Math.max(-50, Math.min(50, score));
    // convert score to a percentage (0 to 100)
    const whiteBarHeight = ((clampedScore + 50) / 100) * 100;

    return (
        <div style={{
            width: '40px',
            height: '1200px',
            backgroundColor: '#000',
            position: 'relative',
            display: 'flex',
            flexDirection: 'column-reverse',
            border: '1px solid #ccc',
        }}>
            <div style={{
                backgroundColor: '#fff',
                width: '100%',
                height: `${whiteBarHeight}%`,
                transition: 'height 0.3s ease-in-out',
            }} />
        </div>
    );
};

export default EvalBar;
