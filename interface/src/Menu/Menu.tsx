import { useGameState } from '../GameState/GameState'

const Menu = () => {

    const gameState = useGameState();
    if (gameState === null) return <></>;

    // const {setState} = gameState;

    return (
        <div>
            {/* <button onClick={() => {
                ENGINE.resetGame().then((result: String) => {
                    console.log("new state", result);
                    setState({activePiece: null, positions: result})
                })}}>
                Reset Board
            </button>
            <button onClick={() => ENGINE.printBoard()}>Print board</button> */}
        </div>
    )
}

export default Menu