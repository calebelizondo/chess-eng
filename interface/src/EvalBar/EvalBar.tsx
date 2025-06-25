import { useGameState } from '../GameState/GameState';

const EvalBar = () => {

    const context = useGameState();

    if (!context) return <></>;

    return (
        <div style={{
            width: "40px",
            height: "100%",

        }}>
            {context.state.score}
        </div>
    );
}

export default EvalBar;