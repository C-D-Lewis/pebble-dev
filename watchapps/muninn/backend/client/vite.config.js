import { viteStaticCopy } from 'vite-plugin-static-copy';

// Set by Vite during 'build'
const isDev = process.env.NODE_ENV !== 'production';

export default {
  // For terraform-modules/s3-cloudfront-website serving of dist/index.html
  base: isDev ? '/' : '/dist',
  plugins: [
    viteStaticCopy({
      targets: [
        {
          src: 'assets',
          dest: '.',
        },
      ],
    }),
  ],
  server: {
    port: 8080,
  },
};
