import styled, { css } from "styled-components";
import { IonCard, IonCardSubtitle } from "@ionic/react";

export {
  IonCardHeader as CardHeader,
  IonCardTitle as CardTitle,
  IonCardContent as CardContent,
} from "@ionic/react";

const Card = styled(IonCard)`
  ${({ theme }) => css`
    --background: ${theme.colors.gradients.blueish};
    --color: ${theme.colors.font.fontPrimary};
  `}
`;

export const CardSubtitle = styled(IonCardSubtitle)`
  ${({ theme }) => css`
    --color: ${theme.colors.font.fontSecondary};
  `}
`;

export default Card;
