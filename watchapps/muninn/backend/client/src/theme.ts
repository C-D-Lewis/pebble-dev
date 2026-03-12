/* eslint-disable jsdoc/require-jsdoc */

/** App theme */
const Theme = {
  palette: {
    text: 'white',
    grey: (v: number) => `#${String(v).repeat(3)}`,
    primary: '#7d2725',
    secondary: '#aa2724',
  },
};

export default Theme;
