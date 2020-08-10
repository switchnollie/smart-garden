import React from "react";
import { RouteComponentProps, Link } from "react-router-dom";
import { IonContent, IonLabel, IonThumbnail } from "@ionic/react";
import Header from "../components/Header";
import IconButton from "../components/IconButton";
import List, { ListItem } from "../components/List";

const wateringGroups = [
  {
    id: "5f2d2bfe7824f2b9fd33cb66",
    displayName: "Orchid",
    lastPumped: "yesterday, 5:12am",
  },
];

interface PlantsPageProps extends RouteComponentProps {}

export default function PlantsPage({ match }: PlantsPageProps) {
  return (
    <>
      <Header>Watering Groups</Header>
      <IonContent>
        <List>
          {wateringGroups.map(({ id, displayName, lastPumped }) => (
            <Link to={`${match.url}/${id}`} style={{ textDecoration: "none" }}>
              <ListItem lines="none" key={id}>
                <IonLabel>
                  <h2>{displayName}</h2>
                  <h4>{`last watering: ${lastPumped}`}</h4>
                </IonLabel>
                <IonThumbnail slot="end">
                  <IconButton icon="water" />
                </IonThumbnail>
              </ListItem>
            </Link>
          ))}
        </List>
      </IonContent>
    </>
  );
}
