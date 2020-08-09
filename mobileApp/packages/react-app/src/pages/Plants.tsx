import React from "react";
import { IonContent } from "@ionic/react";
import Header from "../components/Header";
import Card from "../components/Card";

export default function PlantsPage() {
  return (
    <>
      <Header>Watering Groups</Header>
      <IonContent>
        <Card />
      </IonContent>
    </>
  );
}
