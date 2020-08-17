import React from "react";
import { IonContent, IonThumbnail, IonButton } from "@ionic/react";
import Header from "../components/Header";
import List, { ListItem } from "../components/List";
import ThumbnailImg from "../icons/thumbnail.svg";
import { useSessionContext } from "../contexts/SessionContext";

export default function ProfilePage() {
  /* eslint-disable @typescript-eslint/no-unused-vars */
  const [_, __, removeSession] = useSessionContext();

  return (
    <>
      <Header>Profile</Header>
      <IonContent>
        <List>
          <ListItem lines="none">
            <IonThumbnail slot="start">
              <img src={ThumbnailImg} alt="thumbnail" />
            </IonThumbnail>
            <h3>Tim Weise</h3>
          </ListItem>
        </List>
        <div className="ion-padding">
          <IonButton
            expand="block"
            type="submit"
            className="ion-no-margin"
            onClick={(_) => removeSession()}
          >
            Logout
          </IonButton>
        </div>
      </IonContent>
    </>
  );
}
