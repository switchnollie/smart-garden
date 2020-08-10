import { IonItem } from "@ionic/react";
import styled, { css } from "styled-components";

export { IonList as default } from "@ionic/react";

export const ListItem = styled(IonItem)`
  ${({ theme }) => css`
    h2 {
      font-size: 1.5rem;
      color: ${theme.fontPrimary};
      font-weight: bold;
      letter-spacing: 0.01916em;
    }
    h4 {
      font-size: 0.875rem;
      color: ${theme.colors.font.fontSecondary};
      font-weight: bold;
    }
  `}
`;
