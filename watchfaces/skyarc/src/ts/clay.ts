// Clay setup with PKTS - see README.md

// TODO: Better typing if this proves to work
interface Clay {}
interface ClayConstructor {
  new(config: object[]): Clay;
};

export const setupClay = () => {
  // @ts-ignore
  const Clay: ClayConstructor = require('@rebble/clay');
  // @ts-ignore
  const buildClayConfig: () => any[] = require("../ts-build/config");
  new Clay(buildClayConfig());
};
