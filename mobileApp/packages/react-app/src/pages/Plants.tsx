import React from "react";
import { RouteComponentProps } from "react-router-dom";
import moment from "moment";
import useWateringGroups from "../hooks/useWateringGroups";
import { IonContent, IonLabel, IonThumbnail } from "@ionic/react";
import Header from "../components/Header";
import IconButton from "../components/IconButton";
import List, { ListItem } from "../components/List";
import { useSessionContext } from "../contexts/SessionContext";

interface PlantsPageProps extends RouteComponentProps {}

export default function PlantsPage({ match }: PlantsPageProps) {
  const { groups } = useWateringGroups();
  const [sessionState] = useSessionContext();

  return (
    <>
      <Header>Watering Groups</Header>
      <IonContent>
        <List>
          {groups
            ? groups.map(({ _id: groupId, displayName, lastPumped }) => (
                <ListItem
                  key={groupId}
                  lines="none"
                  routerLink={`${match.url}/${groupId}`}
                >
                  <IonLabel>
                    <h2>{displayName}</h2>
                    <h4>{`last watering: ${moment(lastPumped).fromNow()}`}</h4>
                  </IonLabel>
                  <IonThumbnail slot="end">
                    <IconButton
                      icon="water"
                      onClick={async (e) => {
                        e.stopPropagation();
                        e.preventDefault();
                        await fetch("/api/action/pump", {
                          method: "POST",
                          body: JSON.stringify({
                            userId: sessionState.user?._id,
                            groupId,
                          }),
                          headers: {
                            Authorization: sessionState.jwt || "",
                            "Content-Type": "application/json",
                          },
                        });
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
