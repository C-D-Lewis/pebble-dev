/* eslint-disable jsdoc/require-jsdoc */

/** App theme */
const Theme = {
  palette: {
    text: 'white',
    grey: (v: number) => `#${String(v).repeat(3)}`,
    primary: '#7d2725',
    secondary: '#460f0f',
  },
};

export default Theme;
