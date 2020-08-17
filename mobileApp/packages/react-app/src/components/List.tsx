import { IonItem, IonListHeader } from "@ionic/react";
import styled, { css } from "styled-components";

export { IonList as default } from "@ionic/react";

export const ListItem = styled(IonItem)`
  ${({ theme }) => css`
    --padding-bottom: 0.5rem;
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

export const ListHeader = styled(IonListHeader)`
  ${({ theme }) => css`
    --color: ${theme.colors.font.fontPrimary};
  `}
`;
