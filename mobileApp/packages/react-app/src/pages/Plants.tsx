import React from "react";
import { RouteComponentProps } from "react-router-dom";
import useWateringGroups from "../hooks/useWateringGroups";
import { IonContent, IonLabel, IonThumbnail } from "@ionic/react";
import Header from "../components/Header";
import IconButton from "../components/IconButton";
import List, { ListItem } from "../components/List";

interface PlantsPageProps extends RouteComponentProps {}

export default function PlantsPage({ match }: PlantsPageProps) {
  const { groups } = useWateringGroups();
  return (
    <>
      <Header>Watering Groups</Header>
      <IonContent>
        <List>
          {groups
            ? groups.map(({ _id, displayName, lastPumped }) => (
                <ListItem
                  key={_id}
                  lines="none"
                  routerLink={`${match.url}/${_id}`}
                >
                  <IonLabel>
                    <h2>{displayName}</h2>
                    <h4>{`last watering: ${lastPumped}`}</h4>
                  </IonLabel>
                  <IonThumbnail slot="end">
                    <IconButton
                      icon="water"
                      onClick={(e) => {
                        e.stopPropagation();
                        e.preventDefault();
                      }}
                    />
                  </IonThumbnail>
                </ListItem>
              ))
            : "Loading"}
        </List>
      </IonContent>
    </>
  );
}
