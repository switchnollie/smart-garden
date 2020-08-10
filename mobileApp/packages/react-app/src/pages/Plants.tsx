import React from "react";
import styled, { css } from "styled-components";
import {
  IonContent,
  IonList,
  IonItem,
  IonLabel,
  IonThumbnail,
} from "@ionic/react";
import Header from "../components/Header";
import IconButton from "../components/IconButton";

const ListItem = styled(IonItem)`
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

export default function PlantsPage() {
  return (
    <>
      <Header>Watering Groups</Header>
      <IonContent>
        <IonList>
          <ListItem lines="none">
            <IonLabel>
              <h2>Orchid</h2>
              <h4>last watering: yesterday, 5:12am</h4>
            </IonLabel>
            <IonThumbnail slot="end">
              <IconButton icon="water" />
            </IonThumbnail>
          </ListItem>
        </IonList>
      </IonContent>
    </>
  );
}
