import { createGlobalStyle } from "styled-components";

const accentBlue = "#0087FF";
const accentRed = "#FF0063";

const fontPrimary = "#fff";
const fontSecondary = "rgba(255,255,255,0.5)";

const gradientDark = "linear-gradient(255deg, #13171F 0%, #202635 100%);";
const backgroundDark = "#10141C";

const theme = {
  colors: {
    accentBlue,
    accentRed,
    backgroundDark,
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
    --ion-color-primary: ${accentBlue};
    --ion-color-primary-rgb: 0,135,255;
    --ion-color-primary-shade: #0077e0;
    --ion-color-primary-tint: #1a93ff;

    --ion-color-danger: ${accentRed};
    --ion-color-danger-rgb: 255,0,99;
    --ion-color-danger-shade: #e00057;
    --ion-color-danger-tint: #ff1a73;
  }

  ion-header {
    padding: 1rem 1rem 0 1rem;
    box-sizing: border-box;
  }

  ion-tab-bar {
    --background: ${backgroundDark};
    --border: none;
    margin: 1rem;
    border-radius: 2rem;
    padding: 0.3rem;
    box-shadow: 0px 1px 4px 0px rgba(0,0,0,0.08), 0px 3px 16px 0px rgba(0,0,0,0.06);
  }
  ion-label {
    --color: ${fontSecondary};
    font-size: 0.75rem;
  }

  ion-grid {
    --ion-grid-padding: 0px;
    margin: 0 -5px;
  }
  .alert-radio-label.sc-ion-alert-ios {
    color: #000;
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
  h3 {
    font-size: 1.5rem;
    color: ${fontPrimary};
    font-weight: bold;
    letter-spacing: 0.01916em;
  }
  h4 {
    font-size: 0.875rem;
    color: ${theme.colors.font.fontSecondary};
    font-weight: bold;
  }
`;

export default theme;
