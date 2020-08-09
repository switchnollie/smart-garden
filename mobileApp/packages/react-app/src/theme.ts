import { createGlobalStyle } from "styled-components";

const fontPrimary = "#fff";
const fontSecondary = "rgba(255,255,255,0.5)";

const gradientDark = "linear-gradient(255deg, #13171F 0%, #202635 100%);";

const theme = {
  colors: {
    accentBlue: "#0087FF",
    accentRed: "#FF0063",
    font: {
      fontPrimary,
      fontSecondary,
    },
    gradients: {
      dark: gradientDark,
      blueish: "linear-gradient(255deg, #2D3750 0%, #3D4760 100%);",
      blue: "linear-gradient(255deg, #2D74FF 0%, #3D96FF 100%);",
    },
  },
};

export const AppStyles = createGlobalStyle`
  html {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
  }
  body::before {
    content: "";
    position: absolute;
    top: 0%;
    left: 0%;
    width: 100%;
    height: 100%;
    background: ${gradientDark};
  }
  :root {
    --ion-background-color: transparent;
    --ion-font-family: -apple-system, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
    --ion-text-color: ${fontPrimary};
  }

  ion-header {
    padding: 1rem 1rem 0 1rem;
    box-sizing: border-box;
  }

  h1 {
    font-size: 2.625rem;
    color: ${fontPrimary};
    font-weight: bold;
    letter-spacing: 0.021em;
    margin: 1.143em 0;
  }
  h2 {
    font-size: 2.25rem;
    color: ${fontPrimary};
    font-weight: bold;
    letter-spacing: -0.0255em;
  }
`;

export default theme;
