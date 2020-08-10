import React from "react";
import { IonContent, IonThumbnail } from "@ionic/react";
import Header from "../components/Header";
import List, { ListItem } from "../components/List";
import ThumbnailImg from "../icons/thumbnail.svg";

export default function ProfilePage() {
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
      </IonContent>
    </>
  );
}
