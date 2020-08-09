import React from "react";
import styled, { css } from "styled-components";
import {
  IonCard,
  IonCardHeader,
  IonCardSubtitle,
  IonCardTitle,
  IonCardContent,
} from "@ionic/react";

const StyledCard = styled(IonCard)`
  ${({ theme }) => css`
    --background: ${theme.colors.gradients.blueish};
    --color: ${theme.colors.font.fontPrimary};
  `}
`;

export default function Card() {
  return (
    <StyledCard>
      <IonCardHeader>
        <IonCardSubtitle>Card Subtitle</IonCardSubtitle>
        <IonCardTitle>Card Title</IonCardTitle>
      </IonCardHeader>

      <IonCardContent>
        Keep close to Nature's heart... and break clear away, once in awhile,
        and climb a mountain or spend a week in the woods. Wash your spirit
        clean.
      </IonCardContent>
    </StyledCard>
  );
}
