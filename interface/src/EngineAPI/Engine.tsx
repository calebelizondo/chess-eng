// src/wasm/engine.ts

type EngineModule = {
  //_some_exported_function: (arg: number) => number;
};

export async function loadEngine(): Promise<EngineModule> {
  const module: any = await import(/* @vite-ignore */ '/build/main.js'); // skip bundler transform
  const engine: EngineModule = await module.default(); // often Emscripten exports as default()
  return engine;
}
