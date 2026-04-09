import typescript from '@rollup/plugin-typescript';
import commonjs from '@rollup/plugin-commonjs';
import terser from '@rollup/plugin-terser';

export default [
  {
    input: 'src/ts-config/config.ts',
    output: {
      file: 'src/ts-build/config.js',
      format: 'cjs',
      compact: true,
    },
    plugins: [typescript(), commonjs(), terser()],
  },
];
